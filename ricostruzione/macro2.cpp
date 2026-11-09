// g++ -O2 -std=c++17 -Wall -Wextra macro2.cpp -o macro.exe $(root-config --cflags --libs)
#include <array>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "pmt.h"
#include "TBranch.h"
#include "TChain.h"
#include "TFile.h"
#include "TParameter.h"
#include "TTree.h"
using namespace std;
using Clock = chrono::steady_clock;
const string default_input = "/storage/gpfs_data/juno/junofs/users/lucapelicci/posReco/junosw_final/J2561/junosw/reco/root";

vector<Pmt> load_pmts(const string& path) {
    ifstream in(path);
    if (!in) throw runtime_error("Impossibile aprire geometria: " + path);
    vector<Pmt> pmts;
    int id; double x,y,z; string type;
    while (in >> id >> x >> y >> z >> type) {
        // Conserva il formato originale; l'indicizzazione rapida richiede ID consecutivi.
        if (id == numeric_limits<int>::min() || abs(id) != int(pmts.size()) ||
            !isfinite(x) || !isfinite(y) || !isfinite(z) || x*x+y*y+z*z <= 0)
            throw runtime_error("Geometria non valida: richiesti ID consecutivi da zero e coordinate finite");
        pmts.emplace_back(abs(id),x,y,z,type);
    }
    if (!in.eof() || pmts.empty()) throw runtime_error("Geometria vuota o malformata");
    return pmts;
}

void analisi(double z_s, int run, const string& positions, const string& outputfile,
             double peak, const string& input_dir, int nfiles) {
    constexpr double n_s=1.60;
    auto pmts=load_pmts(positions);
    for (auto& p:pmts) p.initialize(z_s,n_s);
    TChain chain("tofTree");
    for (int i=1;i<=nfiles;++i) {
        string path=input_dir+"/"+to_string(run)+"/tof-"+to_string(run)+"_nLS1.60_nW1.33_"+to_string(i)+".root";
        // Alcune run hanno meno di nfiles file: salta quelli assenti.
        // Un file presente ma illeggibile/non valido resta invece un errore.
        if (!filesystem::exists(path)) {
            cerr << "Run " << run << ": file assente, salto " << path << '\n';
            continue;
        }
        // nentries=0 forza la verifica del file/tree, invece dell'Add lazy originale.
        if (chain.Add(path.c_str(),0)==0) throw runtime_error("File/tree non leggibile: "+path);
    }
    const Long64_t n_events=chain.GetEntries();
    if (n_events<=0 || chain.LoadTree(0)<0) throw runtime_error("Nessun evento leggibile");
    double energy=0,x=0,y=0,z=0,t0=0;
    // Storage di proprietà locale: ROOT riutilizza i vettori senza allocare puntatori non gestiti.
    vector<int> ids_storage; vector<int>* ids=&ids_storage;
    vector<double> times_storage,charges_storage;
    vector<double>* times=&times_storage; vector<double>* charges=&charges_storage;
    array<const char*,8> names={"energy","xReco","yReco","zReco","t0Reco","pmtid","fht","charge"};
    array<TBranch*,8> branches{};
    chain.SetBranchStatus("*",false);
    for (const auto* name:names) chain.SetBranchStatus(name,true);
    auto bind=[&](int i,auto* address) {
        if (chain.SetBranchAddress(names[i],address,&branches[i])<0)
            throw runtime_error(string("Branch mancante o tipo errato: ")+names[i]);
    };
    bind(0,&energy); bind(1,&x); bind(2,&y); bind(3,&z); bind(4,&t0);
    bind(5,&ids); bind(6,&times); bind(7,&charges);
    // Cache dei basket compressi: favorisce letture grandi su GPFS.
    // I vettori vengono comunque deserializzati solo dopo i tagli.
    chain.SetCacheSize(64LL*1024*1024);
    for (const auto* name:names) chain.AddBranchToCache(name,true);
    chain.StopCacheLearningPhase();
    const auto started=Clock::now();
    Long64_t accepted=0,saved=0,bad_id=0,bad_hit=0,bad_event=0;
    int tree_number=-1;
    for (Long64_t event=0;event<n_events;++event) {
        // GetEntry sui TBranch usa l'indice LOCALE al file, non quello globale della TChain.
        const auto local=chain.LoadTree(event);
        if (local<0) throw runtime_error("LoadTree fallito all'evento "+to_string(event));
        if (chain.GetTreeNumber()!=tree_number) {
            tree_number=chain.GetTreeNumber();
            // Controlla schema/tipi ad ogni cambio file; aggiorna i TBranch tramite TChain.
            for (const auto* name:names)
                if (!chain.GetTree()->GetBranch(name)) throw runtime_error(string("Branch assente nel nuovo file: ")+name);
            bind(0,&energy); bind(1,&x); bind(2,&y); bind(3,&z); bind(4,&t0);
            bind(5,&ids); bind(6,&times); bind(7,&charges);
        }
        auto read=[&](int i) {
            if (!branches[i] || branches[i]->GetEntry(local)<=0)
                throw runtime_error(string("Errore lettura ")+names[i]+" evento "+to_string(event));
        };
        read(0); read(1); read(2); read(3);
        if ((event+1)%10000==0 || event+1==n_events)
            cout<<"Run "<<run<<": "<<event+1<<"/"<<n_events<<" eventi\r"<<flush;
        if (!isfinite(energy)||!isfinite(x)||!isfinite(y)||!isfinite(z)) {++bad_event; continue;}
        if (energy<peak-200 || energy>peak+200 || sqrt(x*x+y*y+(z-z_s)*(z-z_s))>1000) continue;
        read(4);
        if (!isfinite(t0)) {++bad_event; continue;}
        read(5); read(6); read(7);
        if (!ids||!times||!charges || ids->size()!=times->size() || ids->size()!=charges->size())
            throw runtime_error("Vettori nulli o non allineati all'evento "+to_string(event));
        ++accepted;
        for (size_t h=0;h<ids->size();++h) {
            const int id=(*ids)[h];
            // JVertex esporta -100 per PMT senza hit. Non stampare una riga per sentinella!
            if (id<0 || size_t(id)>=pmts.size()) {++bad_id; continue;}
            auto& pmt=pmts[id];
            if (!pmt.get_type()) continue;
            const double time=(*times)[h],charge=(*charges)[h];
            if (!isfinite(time)||!isfinite(charge)||charge<=0) {++bad_hit; continue;}
            pmt.add_hit(time,t0,charge,double(event));
            ++saved;
        }
    }
    chain.ResetBranchAddresses();
    const auto read_done=Clock::now();
    TFile out(outputfile.c_str(),"RECREATE");
    if (out.IsZombie()) throw runtime_error("Impossibile creare "+outputfile);
    // Ambito separato: distrugge il TTree prima della chiusura del file.
    {
        TTree tree("pmtRes","First-hit-time residuals grouped by Hamamatsu PMT");
        vector<double> times_out,zeros_out,charges_out,events_out;
        double cosine=0; int id=0;
        tree.Branch("pmt_id",&id); tree.Branch("cos_theta",&cosine);
        tree.Branch("residual_times",&times_out); tree.Branch("t0",&zeros_out);
        tree.Branch("charge",&charges_out); tree.Branch("evtID",&events_out);
        for (auto& pmt:pmts) {
            if (!pmt.get_type()||pmt.get_residual_times_ns().empty()) continue;
            cosine=pmt.get_cos_theta(); id=pmt.get_index();
            pmt.take_hits(times_out,zeros_out,charges_out,events_out);
            if (tree.Fill()<0) throw runtime_error("Errore Fill output");
        }
        out.cd();
        if (tree.Write()<=0) throw runtime_error("Errore scrittura tree");
    }
    TParameter<double>("source_z_mm",z_s).Write();
    TParameter<double>("refractive_index",n_s).Write();
    out.Close();
    const auto done=Clock::now();
    cout<<"\nRun "<<run<<": "<<accepted<<" eventi accettati, "<<saved<<" hit salvati; "
        <<bad_id<<" ID scartati (incluse sentinelle), "<<bad_hit<<" hit non validi, "<<bad_event<<" eventi non finiti\n"
        <<"Lettura/elaborazione: "<<chrono::duration<double>(read_done-started).count()<<" s; scrittura: "
        <<chrono::duration<double>(done-read_done).count()<<" s\n";
}
int main(int argc,char** argv) {
    if (argc<3||argc>6) {
        cerr<<"Uso: "<<argv[0]<<" <posizioni_pmt> <input_run> [directory_input] [directory_output] [numero_file=13]\n";
        return 1;
    }
    try {
        const string input_dir=argc>3?argv[3]:default_input;
        const string output_dir=argc>4?argv[4]:"output";
        const int nfiles=argc>5?stoi(argv[5]):13;
        if (nfiles<1) throw runtime_error("Numero file non valido");
        ifstream runs(argv[2]);
        if (!runs) throw runtime_error("Impossibile aprire lista run");
        filesystem::create_directories(output_dir);
        int run; double z,peak;
        while (runs>>run>>z>>peak) {
            if (!isfinite(z)||!isfinite(peak)) throw runtime_error("Posizione/energia non valida");
            analisi(z,run,argv[1],output_dir+"/output_"+to_string(run)+".root",peak,input_dir,nfiles);
        }
        if (!runs.eof()) throw runtime_error("Lista run malformata");
    } catch (const exception& e) {cerr<<e.what()<<'\n';return 1;}
    return 0;
}
