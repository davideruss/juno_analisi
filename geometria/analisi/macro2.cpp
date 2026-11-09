#include <iostream>
#include <string>
#include <cmath>
#include <vector>
#include <map>
#include <numeric>

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

    std::map<int, std::vector<float>> pmt_times;

    int n_entries = tree->GetEntries();
    int eventi_sorgente = 0;

    //trovo tutti gli eventi che sono nei pressi della sorgente e per ognuno di questi eventi trovo tutti i PMT colpiti e i tempi di arrivo dei fotoni

    for(int id = 0; id < n_entries; id++){
        tree->GetEntry(id);

        TVector3 v(JRecoX, JRecoY, JRecoZ - 17000);

        // Se l'evento è nei pressi della sorgente
        if(v.Mag() < 1000){
            eventi_sorgente++;
            
            // Scorro tutti i PMT colpiti in questo specifico evento
            for(size_t i = 0; i < PMTID->size(); i++){
                int current_pmt = (*PMTID)[i];
                float current_time = (*Time)[i];
                
                // Aggiungo il tempo alla lista dei tempi di questo specifico PMT
                pmt_times[current_pmt].push_back(current_time);
            }
        }
    }

    cout << "Trovati " << eventi_sorgente << " eventi nei pressi della sorgente.\n";

    TFile* outputfile = new TFile(output, "RECREATE");
    TTree* outTree = new TTree("PmtStats", "Statistiche dei PMT (Media Tempi)");
    
    int out_pmtid;
    int out_nhits;
    float out_meantime;
    float out_rmstime;s

    outTree->Branch("PMTID", &out_pmtid);
    outTree->Branch("NHits", &out_nhits);
    outTree->Branch("MeanTime", &out_meantime);
    outTree->Branch("RMSTime", &out_rmstime);

    int pmt_salvati = 0;

    // Calcolo la media per i PMT colpiti più di 5 volte e riempio il TTree
    for(auto const& pair : pmt_times){
        int pmt_id = pair.first;
        const std::vector<float>& times = pair.second;

        if(times.size() > 20){ // Condizione: più di 10 hit
            out_pmtid = pmt_id;
            out_nhits = times.size();
            
            // Calcolo la media
            float sum = std::accumulate(times.begin(), times.end(), 0.0f);
            out_meantime = sum / out_nhits;

            //calcolo la deviazione standard
            float sq_sum = 0.0f;
            for(float t : times){
                sq_sum += (t - out_meantime) * (t - out_meantime);
            }
            out_rmstime = std::sqrt(sq_sum / out_nhits);

            outTree->Fill(); // Salvo un'entry per ogni PMT che rispetta la condizione
            pmt_salvati++;
        }
    }

    cout << "Trovati " << pmt_salvati << " PMT con più di 10 hit.\n";

    outTree->Write();
    inputfile->Close();
    outputfile->Close();

}

int main(int argc, char**argv){

    //primo passo: importo i dati dal file root
    // l'obiettivo qui è prendere i dati del file, calcolarmi i tempi di volo, scriverli su un file e plottarli su un grafico

    if(argc!=3){
        cout << "Usage: " << argv[0] << " <file root> <outputfile>\n";

        return 1;
    }

    analisi(argv[1], argv[2]);

    return 0;

}