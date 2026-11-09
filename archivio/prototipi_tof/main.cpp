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

    TFile* inputfile = new TFile(input, "READ");
    TTree *tree = (TTree*)inputfile->Get("CdEvents");

    double n_s= 1.54;
    double n_w= 1.33;

    Float_t JRecoX, JRecoY, JRecoZ, en;
    
    tree->SetBranchAddress("JRecoX", &JRecoX);
    tree->SetBranchAddress("JRecoY", &JRecoY);
    tree->SetBranchAddress("JRecoZ", &JRecoZ);
    tree->SetBranchAddress("NPE", &en);

    TH1F* energy= new TH1F("energy", "energy distribution", 500, 0, 3000);

    int n_entries = tree->GetEntries();

    vector<int> indici;

    vector<Pmt> pmts;
    int i;
    double x,y,z;
    string type;

    ifstream pos_pmt(positions);
    
    while(pos_pmt>>i >> x >> y >> z >> type){
        
        pmts.push_back(Pmt(i, x, y, z, type));
        
    }

    
    //trovo tutti gli eventi che sono nei pressi della sorgente e per ognuno di questi eventi trovo tutti i PMT colpiti e i tempi di arrivo dei fotoni
    
    for(int id = 0; id < n_entries; id++){
        tree->GetEntry(id);
        
        TVector3 v(JRecoX, JRecoY, JRecoZ - 16600);
        TVector3 v2(JRecoX, JRecoY, JRecoZ);
        
        // Se l'evento è nei pressi della sorgente
        if(v.Mag() < 1000 && v2.Mag()<17700){
            
            energy->Fill(en);
            
            indici.push_back(id);
            
        }
    }
    
    vector<double> mean(pmts.size());
    double en_peak= energy->GetBinCenter(energy->GetMaximumBin());
    int counts =0;
    
    double zmean=0;
    for(auto i:indici){
        tree->GetEntry(i);
        if(en< en_peak+200 && en> en_peak-200){
        
            for(int j=0; j<pmts.size(); j++){

                pmts[j].initialize(JRecoX, JRecoY, JRecoZ, n_s, n_w);

                mean[j]+= pmts[j].get_time();


            }
            zmean+=JRecoZ;
            counts++;
        }

    }
    cout << "Media " << zmean/counts << "\n";
    ofstream outputfile(output);

    for(auto i:mean){
        outputfile << i/static_cast<double>(counts) << "\n";
    }

    cout << "Trovati " << counts << " eventi nei pressi della sorgente.\n";

    inputfile->Close();
    outputfile.close();

}

int main(int argc, char**argv){

    //primo passo: importo i dati dal file root
    // l'obiettivo qui è prendere i dati del file, calcolarmi i tempi di volo, scriverli su un file e plottarli su un grafico

    if(argc!=4){
        cout << "Usage: " << argv[0] << " <file root> <posizioni> <outputfile>\n";

        return 1;
    }

    analisi(argv[1], argv[2], argv[3]);

    return 0;

}