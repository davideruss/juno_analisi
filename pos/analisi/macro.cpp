#include <iostream>
#include <string>
#include <cmath>
#include <vector>

#include "TFile.h"
#include "TTree.h"
#include "TVector3.h"



using namespace std;

void analisi(char* input, char* output){

    TFile* inputfile = new TFile(input, "READ");
    TTree *tree = (TTree*)inputfile->Get("CdEvents");

    std::vector<int>   *PMTID = nullptr;
    std::vector<float> *Charge = nullptr;
    std::vector<float> *Time = nullptr;
    Float_t JRecoX, JRecoY, JRecoZ;

    tree->SetBranchAddress("PMTID", &PMTID);
    tree->SetBranchAddress("Charge", &Charge);
    tree->SetBranchAddress("Time", &Time);

    tree->SetBranchAddress("JRecoX", &JRecoX);
    tree->SetBranchAddress("JRecoY", &JRecoY);
    tree->SetBranchAddress("JRecoZ", &JRecoZ);

    int id=-1;

    bool a=true;

    //trovo un evento nei pressi della sorgente 

    while(a && id<tree->GetEntries()){
        id++;
        tree->GetEntry(id);

        TVector3 v(JRecoX, JRecoY, JRecoZ-17000);

        if(v.Mag() < 1000){
            cout << "Evento " << id << "\n";
            
            a=false;
        }

    }

    tree->GetEntry(id);

    TFile* outputfile = new TFile(output, "RECREATE");

    TTree* outTree = new TTree("FilteredTree", "Tree con un singolo evento filtrato");
    
    outTree->Branch("PMTID", &PMTID);
    outTree->Branch("Time", &Time);

    outTree->Fill();

    outTree->Write();

    inputfile->Close();
    outputfile->Close();

}

int main(int argc, char**argv){

    //primo passo: importo i dati dal file root
    // l'obiettivo qui è prendere i dati del file, calcolarmi i tempi di volo, scriverli su un file, plottarli su un grafico

    if(argc!=3){
        cout << "Usage: " << argv[0] << " <file root> <outputfile>\n";

        return 1;
    }

    analisi(argv[1], argv[2]);

    return 0;

}