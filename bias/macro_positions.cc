#include <fstream>
#include <string>
#include <iostream>
#include <cmath>

#include <TVector3.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TGraph.h>
#include <TTree.h>
#include <TH1.h>
#include <TF1.h>
#include <TROOT.h>


using namespace std;


int RealWork(const string&, const string &);

int RealWork(const string&input, const string &output){
   
	//apro il file con la lista di run
	
   ifstream inputfile(input);
   if(!inputfile.is_open()){
      cout << "Error in inputfile\n\n";
      return 1;
   }
   
	// creo i grafici per il bias e la risoluzione e le variabili che verranno utilizzate dopo
	
   TGraphErrors* bias = new TGraphErrors();
   TGraphErrors* resolution = new TGraphErrors();
   
   string a;
   Float_t z_true;
   Float_t JRecoZ, npe, JRecoY, JRecoX;
   int k=0;
	
	TH1F *z = new TH1F("z", "JRecoZ Distribution", 2000, -19000, 19000);

   TH1F *energy = new TH1F("energy", "energy distribution", 1000, 0, 3000);
 	
	// cicolo while che va avanti fino a quando non finiscono le righe
	// Per ora ho usato solo il dato sulle z perché x e y sono sempre uguali a zero
	
   while(inputfile >> a >> z_true){
      
      Int_t nentries;
      
      string name = "/storage/gpfs_data/juno/junofs/users/caccianijuno/pipeline2_0/ReProd26B/calib/RUN"+ a + "_26B.root";
      
		//Apro il file .root in cui è presente la k-esima run
		
      TFile *f_IN = new TFile(name.c_str(),"READ");
      
      if(!f_IN || f_IN->IsZombie()) {
         
         cerr << "\n Error! Input rootfile not open\n\n";
         return 1;
      }
     
      TTree *JTree = (TTree*) f_IN->Get("CdEvents");
      if(!JTree){
         cerr << "Error: no CdEvents in " << name << endl;
         return 1;
      }

      nentries = JTree->GetEntries();

		JTree->SetBranchStatus("*", 0);     
		JTree->SetBranchStatus("JRecoZ", 1); 
		JTree->SetBranchStatus("JRecoX", 1);
		JTree->SetBranchStatus("JRecoY", 1);
		JTree->SetBranchStatus("NPE", 1);
   
		//imposto le variabili che dovrò usare (x, y, z, NPE)
      JTree->SetBranchAddress("JRecoZ",&JRecoZ);
		JTree->SetBranchAddress("NPE",&npe);
		JTree->SetBranchAddress("JRecoX", &JRecoX);
		JTree->SetBranchAddress("JRecoY", &JRecoY);
			
		//analisi preliminare
		//Lo scopo di questa analisi preliminare è quella di trovare l'intervallo di energia della sorgente (che non so a priori perché gli NPE dipendono dalla posizione) e di trovare un primo valore di z su cui poi basiamo la seconda analisi
		      	
      for(int i=0;i<nentries; i++){
     		
			JTree->GetEntry(i);
			
			TVector3 v(JRecoX, JRecoY, JRecoZ-z_true); 
	 		
			if(v.Mag()<1000){

				energy->Fill(npe);
	
			}		
      
      }

		double peak= energy->GetBinCenter(energy->GetMaximumBin());
      
      //seconda analisi

		for(int i=0;i<nentries; i++){
			
         JTree->GetEntry(i);

         TVector3 v(JRecoX, JRecoY, JRecoZ-z_true); 
			
         if(npe<peak+100 && npe>peak-100 && v.Mag()<1000){

            z->Fill(JRecoZ);

         }

      }
		
		int entries =  z->GetEntries();
		cout << "number of events= " << entries << "\n";
      
      double mean0 = z->GetMean();
      double stddev0 = z->GetStdDev();


      TF1 *gausFit = new TF1("gausFit", "gaus", mean0 - 3*stddev0, mean0 + 3*stddev0);
      gausFit->SetParameters(z->GetMaximum(), mean0, stddev0);

      TFitResultPtr fitResult = z->Fit(gausFit, "RQS");

      double mean    = gausFit->GetParameter(1);
		double meanErr = gausFit->GetParError(1);
      double sigma   = gausFit->GetParameter(2);
		double sigmaErr= gausFit->GetParError(2);

      double chi2 = gausFit->GetChisquare();
      int ndf = gausFit->GetNDF();
      cout << "chi2/ndf = " << chi2 << " / " << ndf << " = " << chi2/ndf << "\n";

		cout << "bias = " << mean -z_true << "\n";
      bias->SetPoint(k, z_true, mean -z_true);
		bias->SetPointError(k, 0, meanErr);

      resolution->SetPoint(k, z_true, sigma);
      resolution->SetPointError(k, z_true, sigmaErr);

      z->Reset();
		energy->Reset();
      f_IN->Close();
      delete f_IN;
      
      delete gausFit;
      
      cout << "file " << k << "\n\n";
      k++;
   }
   
	delete z;
	delete energy;

   const char* out_name = output.c_str();
   TFile *f_OUT = new TFile(out_name, "RECREATE");
   
   // Save all histos and close the output rootfile
   bias->Write("bias");
   
   resolution->Write("resolution");

   f_OUT->Close();
   delete f_OUT;
   delete bias;
   delete resolution;

   cout << "Rootfile " << out_name << " closed.\n\n";

   return 0;
}
//#####################################################
int main(int argc, char** argv) {
   if(argc!=3) {
      cout << "\n USAGE: " << argv[0] << " input_root output_root\n" << endl;
      return 1;
   }
   string input  = argv[1];
   string output = argv[2];
   RealWork(input, output);
   return 0;

}
