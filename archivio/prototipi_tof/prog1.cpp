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

void analisi(char* input, char* positions, char* output){

    ifstream file1(input);

    string nome_run;
    double pos_z;
    int n_run= 0;
	double mean_tof_ns;
	double err_tof_ns;
	int n_events;
	int pmt_id; 
    bool pmt_type;

    double n_s= 1.54;
    double n_w= 1.33;
    vector<int> *PMTID = nullptr;
    
    
	TFile* outputfile= new TFile(output, "RECREATE");
	TTree* outtree = new TTree("tof_cal", "tof_cal"); 
    
	outtree->Branch("run", &n_run);
	outtree->Branch("posZ", &pos_z);
	outtree->Branch("mean_tof_ns", &mean_tof_ns);
	outtree->Branch("err_tof_ns", &err_tof_ns);
    outtree->Branch("pmt_id", &pmt_id);
    outtree->Branch("n_events", &n_events);
    outtree->Branch("pmt_type", &pmt_type);
    
    while(file1>> nome_run >> pos_z){
        TFile* inputfile = new TFile(nome_run.c_str(), "READ");
        TTree *tree = (TTree*)inputfile->Get("CdEvents");

        
        Float_t JRecoX, JRecoY, JRecoZ, en;
        
        tree->SetBranchAddress("PMTID", &PMTID);
        tree->SetBranchAddress("JRecoX", &JRecoX);
        tree->SetBranchAddress("JRecoY", &JRecoY);
        tree->SetBranchAddress("JRecoZ", &JRecoZ);
        tree->SetBranchAddress("NPE", &en);
        
        TH1F* energy= new TH1F("energy", "energy distribution", 500, 0, 3000);
        energy->SetDirectory(nullptr);

        Long64_t n_entries = tree->GetEntries();
        
        vector<int> indici;
        vector<Pmt> pmts;

        int i;
        double x,y,z;
        string type;

        ifstream pos_pmt(positions);
        
        while(pos_pmt>>i >> x >> y >> z >> type){
            
            pmts.push_back(Pmt(i, x, y, z, type));

        }

        
        //trovo tutti gli eventi che sono nei pressi della sorgente e per ognuno di questi eventi trovo tutti i PMT colpiti
        
        for(Long64_t id = 0; id < n_entries; id++){
            tree->GetEntry(id);
            
            TVector3 v(JRecoX, JRecoY, JRecoZ - pos_z);
            TVector3 v2(JRecoX, JRecoY, JRecoZ);
            
            // Se l'evento è nei pressi della sorgente
            if(v.Mag() < 1000 && v2.Mag()<17700){
                
                energy->Fill(en);
                
                indici.push_back(id);
                
            }
        }
        
        vector<int> indici2;
        vector<int> indici_pmt;
        vector<int> n_hit(pmts.size());
        double en_peak= energy->GetBinCenter(energy->GetMaximumBin());

        for(auto i:indici){
            tree->GetEntry(i);
            if(en< en_peak+200 && en> en_peak-200){

                indici2.push_back(i);
                for(int hit_id : *PMTID){
                    if(pmts[j].get_index() == hit_id){
                    n_hit[j]++;
                    break;
                    }
                }

            }

        }
        
        for(size_t j=0; j<pmts.size(); j++){
            if(n_hit[j]>10) indici_pmt.push_back(j);
        }
        
        vector<double> mean(indici_pmt.size());
        vector<double> mean2(indici_pmt.size());

        for(auto i:indici2){
            tree->GetEntry(i);

            
            for(size_t j=0; j<indici_pmt.size(); j++){

                pmts[indici_pmt[j]].initialize(JRecoX, JRecoY, JRecoZ, n_s, n_w);

                double t = pmts[indici_pmt[j]].get_time();
                mean[j]+= t;
                mean2[j]+=t*t;
            }
        }

        int counts= indici2.size();

        for(size_t j = 0; j < indici_pmt.size(); j++){
            
            pmt_id =pmts[indici_pmt[j]].get_index();
            mean_tof_ns = mean[j]/static_cast<double>(counts);
            err_tof_ns = sqrt((mean2[j]/counts- mean_tof_ns*mean_tof_ns)/(counts-1));
            n_events= counts;
            //pos_z è già sistemata
            //n_run è già sistemata
            pmt_type= pmts.get_type();

            outtree->Fill();
        }

        n_run++;
        inputfile->Close();
        delete inputfile;
        delete energy;
    }
    outputfile->Close();
}

int main(int argc, char**argv){

    //primo passo: importo i dati dal file root
    // l'obiettivo qui è prendere i dati del file, calcolarmi i tempi di volo, scriverli su un file e plottarli su un grafico

    if(argc!=4){
        cout << "Usage: " << argv[0] << " <file lista> <file posizioni pmt> <outputfile>\n";

        return 1;
    }

    analisi(argv[1], argv[2], argv[3]);

    return 0;

}
