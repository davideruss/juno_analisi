//Per compilare: g++ -Wall macro.cpp -o macro.exe `root-config --cflags --libs`

#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <fstream>

#include "pmt.h"

#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"
#include "TChain.h"
#include "TParameter.h"

using namespace std;

bool load_pmts(const char* positions, vector<Pmt>& pmts){
    ifstream pos_pmt(positions);

    if (!pos_pmt) {
        cerr << "Errore file delle posizioni\n";
        return false;
    }

    int i;
    double x,y,z;
    string type;

        
    while(pos_pmt>>i >> x >> y >> z >> type){
            
        pmts.push_back(Pmt(abs(i), x, y, z, type));

    }

    return true;

}

void load_root_file(int num_run, TChain &chain){
    for(int i=1; i<14; i++){
        
        string nome = "tof_" + to_string(num_run) +"_nLS1.597_nW1.33_" + to_string(i) + ".root";

        chain.Add(nome.c_str());
    }

    return;
}

int analisi(double z_s, int num_run, const char*positions, const char* outputfile, double peak_en){
    
    double n_s= 1.59;

    vector<Pmt> pmts;

    if(!load_pmts(positions, pmts)){
        return 1;
    }


    for(Pmt& pmt: pmts){
        pmt.initialize(z_s, n_s);
    }
    cout << "tempi calcolati\n";

    TChain input_tree("tofTree");

    load_root_file( num_run, input_tree);

    const Long64_t n_events = input_tree.GetEntries();
    
    if (n_events <= 0 || input_tree.LoadTree(0) < 0) {
        std::cerr << "Errore: la TChain non contiene eventi leggibili.\n";
        return 1;
    }

    input_tree.SetBranchStatus("*", false);
    input_tree.SetBranchStatus("NPE", true);
    input_tree.SetBranchStatus("pmtid", true);
    input_tree.SetBranchStatus("first_hit_time", true);

    Double_t npe;
    vector<int>   *pmt_ids = nullptr;
    vector<double> *first_hit_times_ns = nullptr;

    input_tree.SetBranchAddress("pmtid", &pmt_ids);
    input_tree.SetBranchAddress("first_hit_time", &first_hit_times_ns);
    input_tree.SetBranchAddress("NPE", &npe);

    Double_t min_energy = peak_en-100;
    Double_t max_energy = peak_en+100;

    int n_saved_residuals=0;
    
    for (Long64_t event = 0; event < n_events; ++event) {

        input_tree.GetEntry(event);

        if ((event + 1) % 10000 == 0 || event + 1 == n_events) {
            cout << "Elaborati " << event + 1 << " / " << n_events
                      << " eventi\r" << flush;
        }

        // Intervallo aperto: 1300 < NPE < 1900.
        if (!(npe > min_energy && npe < max_energy)) {
            continue;
        }

        if (!pmt_ids || !first_hit_times_ns) {
            cerr << "Errore: vettore nullo all'evento " << event << ".\n";
            return 1;
        }

        if (pmt_ids->size() != first_hit_times_ns->size()) {
            cout << "Casino.\n";
            continue;
        }


        for (size_t hit = 0; hit < pmt_ids->size(); ++hit) {
            const int pmt_id = pmt_ids->at(hit);

            if (pmt_id < 0 ||
                static_cast<std::size_t>(pmt_id) >= pmts.size()) {
                cout << "mi sa che devi mettere quel valore assoluto\n";
                continue;
            }

            Pmt& pmt = pmts[static_cast<std::size_t>(pmt_id)];
            if (!pmt.get_type()) {
                continue;
            }

            const double measured_time_ns = first_hit_times_ns->at(hit);
            if (!std::isfinite(measured_time_ns)) {
                continue;
            }

            pmt.add_t_res(measured_time_ns);
            ++n_saved_residuals;
        }
    }

    cout << "\nTempi salvati: " << n_saved_residuals << "\n";
    
    TFile output(outputfile , "RECREATE");
    if (output.IsZombie()) {
        cerr << "Errore: impossibile creare " << outputfile << ".\n";
        return 1;
    }

    TTree output_tree("pmtRes", "First-hit-time residuals grouped by Hamamatsu PMT");

    vector<double> output_tres;
    double output_cos_theta;
    int output_pmt_id;

    output_tree.Branch("pmt_id", &output_pmt_id);
    output_tree.Branch("cos_theta", &output_cos_theta);
    output_tree.Branch("residual_times", &output_tres);

    for (const Pmt& pmt : pmts) {
        if (!pmt.get_type() || pmt.get_residual_times_ns().empty()) {
            continue;
        }
        
        output_cos_theta = pmt.get_cos_theta();
        output_pmt_id = pmt.get_index();
        output_tres = pmt.get_residual_times_ns();
        output_tree.Fill();
    }

    output.cd();
    output_tree.Write();

    TParameter<double>("source_z_mm", z_s).Write();

    return 0;

}

int main(int argc, char ** argv){
    if(argc!=3){
        cout << "Uso: " << argv[0] << " <posizioni_pmt> <input_file>\n";
        return 1;
    }

    double peak_en;
    double z_s;
    int num_run;
    
    ifstream input(argv[2]);
    if(!input){
        cout << "Errore nell'apertura del file di input\n";
        return 1;
    }
    while(input >> num_run >> z_s >> peak_en){

        string nome_output = "output_" + to_string(num_run) + ".root";

        
        if(analisi(z_s, num_run , argv[1], nome_output.c_str(), peak_en)!=0){
            cerr << "Problema nel file " << num_run << "\n";
            return 1;
        }
        cout << "finita run " << num_run << "\n";
    }
    
    return 0;
}
