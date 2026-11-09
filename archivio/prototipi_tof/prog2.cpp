//per compilare:
// g++ -Wall prog2.cpp pmt.h -o prog2.exe `root-config --cflags --libs`

#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <numeric>

#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"
#include "TH1F.h"
#include "pmt.h"

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
            
        pmts.push_back(Pmt(i, x, y, z, type));

    }

    return true;

}

int analisi(const char* run_list,const char* positions, char* output){
    
    double n_s= 1.54;
    double n_w= 1.33;

    vector<Pmt> pmts;

    if(!load_pmts(positions, pmts)){
        return 1;
    }

    ifstream file1(run_list);

    if (!file1) {
        cerr << "Errore nella lista\n" ;
        return 1;
    }

    int n_run= 0;
	int event_id;
	int pmt_id; 
    
    double pos_z;
    Float_t reco_x;
    Float_t reco_y;
    Float_t reco_z;

	double tof_source_ns;
    double tof_reco_ns;
    double delta_ns;
    
    bool pmt_type;
    
    
    
	TFile* outputfile= new TFile(output, "RECREATE");
	TTree* outtree = new TTree("tof_cal", "tof_cal"); 
    
	outtree->Branch("run", &n_run);
    outtree->Branch("event_id", &event_id);
    outtree->Branch("pmt_id", &pmt_id);
	
    outtree->Branch("posZ", &pos_z);
    outtree->Branch("reco_x", &reco_x);
    outtree->Branch("reco_y", &reco_y);
    outtree->Branch("reco_z", &reco_z);
    
	outtree->Branch("tof_source_ns", &tof_source_ns);
	outtree->Branch("tof_reco_ns", &tof_reco_ns);
    outtree->Branch("delta_ns", &delta_ns);
    
    outtree->Branch("pmt_type", &pmt_type);
    
    string nome_run;
    
    while(file1>> nome_run >> pos_z){
        
        TFile* inputfile = new TFile(nome_run.c_str(), "READ");
        
        if(!inputfile){
            cerr<< "errore nel file: " << nome_run << "\n";
            return 1;
        }
        
        TTree *tree = (TTree*)inputfile->Get("CdEvents");
        
        Float_t JRecoX, JRecoY, JRecoZ, en;
        vector<int> *PMTID = nullptr;
        
        tree->SetBranchAddress("PMTID", &PMTID);
        tree->SetBranchAddress("JRecoX", &JRecoX);
        tree->SetBranchAddress("JRecoY", &JRecoY);
        tree->SetBranchAddress("JRecoZ", &JRecoZ);
        tree->SetBranchAddress("NPE", &en);

        tree->SetBranchStatus("*", 0); 
        tree->SetBranchStatus("JRecoX", 1);
        tree->SetBranchStatus("JRecoY", 1);
        tree->SetBranchStatus("JRecoZ", 1);
        tree->SetBranchStatus("NPE", 1);
        
        TH1F* energy= new TH1F("energy", "energy distribution", 500, 0, 3000);
        energy->SetDirectory(nullptr);

        Long64_t n_entries = tree->GetEntries();
        
        vector<Long64_t> indici; //indici degli eventi che rispettano la selezione spaziale
        
        for(Long64_t id = 0; id < n_entries; id++){
            tree->GetEntry(id);
            
            TVector3 v(JRecoX, JRecoY, JRecoZ - pos_z);
            TVector3 v2(JRecoX, JRecoY, JRecoZ);
            
            if(v.Mag() < 1000 && v2.Mag()<17700){
                
                energy->Fill(en);
                
                indici.push_back(id);
                
            }
        }
        
        int selected_events = 0;
        double en_peak= energy->GetBinCenter(energy->GetMaximumBin());
        
        cout << "  Picco energetico stimato= "<< en_peak << " NPE\n";

        vector<double> source_tof_per_pmt(pmts.size());
        
        for(size_t i =0; i<pmts.size(); i++){
            pmts[i].initialize(0., 0., pos_z, n_s, n_w);
            source_tof_per_pmt[i]= pmts[i].get_time();
        }

        tree->SetBranchStatus("*", 1);

        for(auto entry:indici){
            tree->GetEntry(entry);

            bool energy_cut = en< en_peak+200 && en> en_peak-200;
            if(!energy_cut){
                continue;
            }
            
            selected_events++;
            event_id = entry;
            reco_x = JRecoX;
            reco_y = JRecoY;
            reco_z = JRecoZ;

            for(int hit_id : *PMTID){

                tof_source_ns = source_tof_per_pmt[hit_id];
                pmts[hit_id].initialize(reco_x, reco_y, reco_z, n_s, n_w);
                tof_reco_ns = pmts[hit_id].get_time();

                delta_ns = tof_source_ns - tof_reco_ns;

                //n_run è già fissata
                //pos_z è già fissata
                pmt_id = hit_id;
                pmt_type= pmts[hit_id].get_type();
                
                outtree->Fill();
                
            }

            

        }

        cout << selected_events << " eventi selezionati\n";

        n_run++;
        inputfile->Close();
        delete PMTID;
        delete inputfile;
        delete energy;
    }
    outputfile->cd();
    outtree->Write();
    outputfile->Close();

    return 0;
}

int main(int argc, char**argv){

    //primo passo: importo i dati dal file root
    // l'obiettivo qui è prendere i dati del file, calcolarmi i tempi di volo, scriverli su un file e plottarli su un grafico

    if(argc!=4){
        cout << "Usage: " << argv[0] << " <file lista> <file posizioni pmt> <outputfile>\n";

        return 1;
    }

    return analisi(argv[1], argv[2], argv[3]);

}
