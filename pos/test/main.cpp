//per compilare:
// g++ -Wall main.cpp pmt.h -o main `root-config --cflags --libs`

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

	int pmt_id; 
    
    double pos_z;
    double cos_theta;

	double tof_approx;
    double tof_true;

    double delta_ns;
    
    bool pmt_type;
    
	TFile* outputfile= new TFile(output, "RECREATE");
	TTree* outtree = new TTree("tof_cal", "tof_cal"); 
    
    outtree->Branch("pmt_id", &pmt_id);
	
    outtree->Branch("posZ", &pos_z);
    
	outtree->Branch("tof_approx", &tof_approx);
	outtree->Branch("tof_true", &tof_true);
    outtree->Branch("delta_ns", &delta_ns);
    outtree->Branch("cos_theta", &cos_theta);
    
    outtree->Branch("pmt_type", &pmt_type);

    while(file1 >> pos_z){
        
        for(size_t i =0; i<pmts.size(); i++){
            
            pmts[i].initialize(0., 0., pos_z, n_s, n_w, 0.000001);

            tof_approx = pmts[i].get_time();
            tof_true = pmts[i].get_time_true();
   
            delta_ns = tof_true - tof_approx;

            //pos_z è già fissata
            pmt_id = abs(pmts[i].get_index());
            pmt_type= pmts[i].get_type();
            cos_theta = pmts[i].get_cos_theta();

            outtree->Fill();
        }

        cout << "Finita la posizione: " << pos_z <<"\n";

    }
    outputfile->cd();
    outtree->Write();
    outputfile->Close();

    return 0;
}

int main(int argc, char**argv){

    if(argc!=4){
        cout << "Usage: " << argv[0] << " <file lista> <file posizioni pmt> <outputfile>\n";

        return 1;
    }

    return analisi(argv[1], argv[2], argv[3]);

}
