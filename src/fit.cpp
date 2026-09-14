//per compilare g++ -Wall fit.cpp -o fit.exe `root-config --cflags --libs`

#include <algorithm>
#include <string>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "TFile.h"
#include "TF1.h"
#include "TFitResult.h"
#include "TH1D.h"
#include "TTree.h"
#include "Math/MinimizerOptions.h"

using namespace std;
/*
Allora passaggi logici:
Prendo il file con i dati,
Creo un vector di istogrammi dimensione: numero di bin in cui divido cos_theta 
Riempio gli istogrammi
una volta che ho gli istogrammi devo prendere un buon intervallo tipo [0, 0.2) e faccio il primo fit e trovo il picco
prendo tutti gli intervalli e trovo i picchi
come output posso avere anche solo un file .txt con i valori delta t calcolati in questo modo
*/

constexpr int n_theta = 20, n_time = 150, reference_bin_min = 8, reference_bin_max=11;
constexpr double cos_min = -1., time_min = 200., time_max = 500.;
const double missing = numeric_limits<double>::quiet_NaN();
struct Result {
    double peak = missing, chi2_ndf = missing;
    double chi2 = missing, edm = missing;
    double fit_min = missing, fit_max = missing;
    int ndf = 0, status = -1, cov_status = -1, attempt = -1;
    bool accepted = false, root_valid = false;
    // DoubleExpGauss: C1, mu, sigma, tau1, C2, tau2, background.
    // Per la gaussiana si usano soltanto i primi quattro elementi.
    array<double, 7> params = {missing, missing, missing, missing,
                               missing, missing, missing};
};

double ExpGauss(double t, double C, double mu, double sigma, double tau) {
    const double a = (mu + sigma*sigma/tau - t) / (sqrt(2.)*sigma);
    const double exponent = (mu-t)/tau + sigma*sigma/(2.*tau*tau);
    // Per a grande exp(exponent)*erfc(a) produce inf*0. Questa forma
    // asintotica equivalente evita overflow nella stessa funzione ExpGauss.
    if (a > 20.) {
        const double u = 1./(2.*a*a);
        const double scaled_erfc = (1.-u+3.*u*u-15.*u*u*u+105.*pow(u,4))/ (sqrt(acos(-1.))*a);
        return C/(2.*tau) * exp(-0.5*pow((t-mu)/sigma,2)) * scaled_erfc;
    }
    return C/(2.*tau) * exp(exponent) * erfc(a);
}


double DoubleExpGauss(double* x, double* par){
    const double signal1 = ExpGauss(x[0], par[0], par[1], par[2], par[3]);
    const double signal2 = ExpGauss(x[0], par[4], par[1], par[2], par[5]);
    return signal1 + signal2 + par[6];
}

Result fit_peak(TH1D& h, bool double_exp){
    // se double_exp ==0 allora si fa la doppia gauss_Exp
    Result best;

    int best_status = -1 ; 

    if (h.GetEntries() < 30 || h.GetMaximum() <= 0.) return best;
    
    const double peak = h.GetBinCenter(h.GetMaximumBin());
    const double window = double_exp ? 50. : 12.;
    const double low = double_exp ? peak-window*0.4 : peak -window;
    const double high = double_exp ? peak+window*4: peak + window;
    
    TF1 f;
    
    if (double_exp) f = TF1("double_exp", DoubleExpGauss, low, high, 7);
    else f = TF1("gauss_local", "[0]*exp(-0.5*((x-[1])/[2])^2)+[3]", low, high);
    
    const double area = h.Integral(h.FindBin(low),h.FindBin(high),"width");
    const double background = max(0.,h.GetBinContent(h.FindBin(high)));
    
    if (double_exp) {
        f.SetParLimits(0,0.,20.*area);
        f.SetParLimits(1,low,min(high,peak+15.));
        f.SetParLimits(2,0.2,30.);
        f.SetParLimits(3,0.2,30.);
        f.SetParLimits(4,0.,20.*area);
        f.SetParLimits(5,3.,200.);
        f.SetParLimits(6,0.,h.GetMaximum());
    } else {
        f.SetParLimits(0,0.,10.*h.GetMaximum());
        f.SetParLimits(1,low,high);
        f.SetParLimits(2,0.2,20.);
        f.SetParLimits(3,0.,h.GetMaximum());
    }

    for (int attempt=0; attempt<4; ++attempt) {
        if (double_exp)
            f.SetParameters(0.75*area,peak-2.,3.+attempt,6.+2.*attempt,
                            0.25*area,25.+10.*attempt,background);
        else f.SetParameters(h.GetMaximum(),peak,3.+2.*attempt,background);
        auto result = h.Fit(&f,"RQSN");
        // Status 1 di Minuit2 riguarda la covarianza, non il valore del picco.
        // Lo accettiamo solo con distanza stimata dal minimo (EDM) piccola.
        if (!result.Get() || (int(result)!=0 && int(result)!=1) ||
            !isfinite(result->Edm()) || result->Edm()>1.e-4 || f.GetNDF()<=0)
            continue;

        const double mode = double_exp ? f.GetMaximumX(low,high) : f.GetParameter(1);
        const double chi = f.GetChisquare()/f.GetNDF();
        
        if (isfinite(mode) && isfinite(chi) && mode>low+h.GetBinWidth(1)
            && mode<high-h.GetBinWidth(1) && (!isfinite(best.chi2_ndf) || chi<best.chi2_ndf))
        {
            best.peak = mode;
            best.chi2_ndf = chi;
            best.chi2 = f.GetChisquare();
            best.ndf = f.GetNDF();
            best.edm = result->Edm();
            best.status = int(result);
            best.cov_status = result->CovMatrixStatus();
            best.root_valid = result->IsValid();
            best.accepted = true;
            best.attempt = attempt;  // Indice da 0 a 3.
            best.fit_min = low;
            best.fit_max = high;
            for (int p = 0; p < f.GetNpar(); ++p)
                best.params[p] = f.GetParameter(p);
            best_status = best.status;
        }
    }
    cout << h.GetName() << ' ' << (double_exp?"DoubleExpGauss":"gauss")
         << " status=" << best_status << " picco=" << setprecision(10) << best.peak
         << " chi2/ndf=" << best.chi2_ndf << '\n';
    return best;
}

vector<TH1D> load_histograms(int run, double z) {
    
    const string path = "output2/output_"+to_string(run)+".root";
    
    TFile file(path.c_str(),"READ");

    if (file.IsZombie()) throw runtime_error("Impossibile aprire "+path);
    
    TTree* tree = nullptr;

    file.GetObject("pmtRes",tree);
    
    if (!tree) throw runtime_error("pmtRes assente in "+path);
    
    double cosine = 0.;
    vector<double>* times = nullptr;
    
    if (tree->SetBranchAddress("cos_theta",&cosine)<0 ||
        tree->SetBranchAddress("residual_times",&times)<0)
        throw runtime_error("Branch mancanti in "+path);
    
    vector<TH1D> hist;
    hist.reserve(n_theta);
    
    for (int b=0;b<n_theta;++b) {
        hist.emplace_back(Form("h_%d_%d",run,b),"",n_time,time_min,time_max);
        hist.back().SetDirectory(nullptr);
    }

    for (Long64_t entry=0;entry<tree->GetEntries();++entry) {
        
        if (tree->GetEntry(entry)<=0) throw runtime_error("Errore lettura "+path);
        if (!times || !isfinite(cosine) || cosine<cos_min || cosine>1.) continue;
        
        const int b = min(n_theta-1,int((cosine-cos_min)/(1.-cos_min)*n_theta));
        
        for (double t:*times) if (isfinite(t)) hist[b].Fill(t);
    }

    tree->ResetBranchAddresses();
    delete times;
    cout << "Run " << run << ", z = " << z << " mm\n";

    return hist;
}

void save_run_output(int run, double z, vector<TH1D>& hist,
                     const vector<Result>& fits, TH1D& reference, Result& fit ) {
    if (hist.size() != fits.size() || hist.empty())
        throw runtime_error("Istogrammi e risultati non corrispondono");
    
    const string path = "fit_run_" + to_string(run) + ".root";

    TFile output(path.c_str(), "RECREATE");
    
    if (output.IsZombie()) throw runtime_error("Impossibile creare " + path);

    TTree tree("fit_summary", "Miglior DoubleExpGauss accettato per bin");
    
    tree.SetDirectory(nullptr);
    
    int bin_id = 0;
    double cos_low = 0., cos_high = 0., delta_t = missing;
    
    Result saved;
    
    tree.Branch("run", &run, "run/I");
    tree.Branch("source_z", &z, "source_z/D");
    tree.Branch("bin_id", &bin_id, "bin_id/I");
    tree.Branch("cos_low", &cos_low, "cos_low/D");
    tree.Branch("cos_high", &cos_high, "cos_high/D");
    tree.Branch("peak", &saved.peak, "peak/D");
    tree.Branch("delta_t", &delta_t, "delta_t/D");
    tree.Branch("chi2", &saved.chi2, "chi2/D");
    tree.Branch("ndf", &saved.ndf, "ndf/I");
    tree.Branch("chi2_ndf", &saved.chi2_ndf, "chi2_ndf/D");
    tree.Branch("fit_min", &saved.fit_min, "fit_min/D");
    tree.Branch("fit_max", &saved.fit_max, "fit_max/D");
    tree.Branch("status", &saved.status, "status/I");
    tree.Branch("cov_status", &saved.cov_status, "cov_status/I");
    tree.Branch("edm", &saved.edm, "edm/D");
    tree.Branch("accepted", &saved.accepted, "accepted/O");
    tree.Branch("root_valid", &saved.root_valid, "root_valid/O");
    tree.Branch("attempt", &saved.attempt, "attempt/I");
    tree.Branch("params", saved.params.data(), "params[7]/D");


    for (bin_id = 0; bin_id < static_cast<int>(hist.size()); ++bin_id) {
        // Assegnazione in-place: gli indirizzi dei branch restano validi.
        saved = fits[bin_id];
        cos_low = cos_min + (1. - cos_min) * bin_id / hist.size();
        cos_high = cos_min + (1. - cos_min) * (bin_id + 1) / hist.size();
        const double ref_peak = fit.peak;
        delta_t = (isfinite(saved.peak) && isfinite(ref_peak))
                    ? saved.peak - ref_peak : missing;
        output.cd();
        if (hist[bin_id].Write(Form("h_%d", bin_id)) <= 0)
            throw runtime_error("Errore scrittura istogramma in " + path);
        if (tree.Fill() < 0)
            throw runtime_error("Errore riempimento fit_summary");
    }
// Aggiungo il fit di riferimento
    bin_id= n_theta;
    saved = fit;
    cos_low = -0.2;
    cos_high = 0.2;
    delta_t = 0;
    if(reference.Write(Form("h_%d", bin_id))<= 0)
        throw runtime_error("Errore scrittura istogramma di riferimento in " + path);
    
    if(tree.Fill()<0) throw runtime_error("Errore riempimento in " + path);

    output.cd();
    if (tree.Write() <= 0) throw runtime_error("Errore scrittura fit_summary");
    output.Close();
    cout << "Istogrammi e parametri DoubleExpGauss salvati in " << path << '\n';
}

int main() {
    try {
        ROOT::Math::MinimizerOptions::SetDefaultMinimizer("Minuit2","Migrad");
        ROOT::Math::MinimizerOptions::SetDefaultMaxFunctionCalls(40000);
        
        ifstream runs("output_con_picchi.txt");
        
        if (!runs) throw runtime_error("Impossibile aprire run.dat");
        
        vector<vector<Result>> results[2];

        vector<Result> references[2];
        string line;

        while (getline(runs,line)) {
            if (line.find_first_not_of(" \t\r")==string::npos) continue;
            
            istringstream row(line);
            
            int run;
            double z;
            
            if (!(row>>run>>z)) throw runtime_error("Riga non valida in run.dat");
            // Le eventuali colonne successive non servono per questa analisi.
            
            auto hist = load_histograms(run,z);
            
            TH1D reference = hist[reference_bin_min];

            

            reference.SetName(Form("h_reference_%d", run));
            reference.SetDirectory(nullptr);
            
            
            for (int b = reference_bin_min+ 1 ; b <= reference_bin_max; ++b)
            reference.Add(&hist[b]);    
        
            for (int method=0;method<2;++method) {
                
                vector<Result> fits;
            
                Result reference_fit = fit_peak(reference, method ==0);
                
                references[method].push_back(reference_fit);
                
                for (int b=0;b<n_theta;++b) {
                
                    fits.push_back(fit_peak(hist[b],method==0));
                    if (!isfinite(fits.back().peak))
                        cerr << "Fit fallito: run " << run << ", bin " << b
                             << ", metodo " << method << " (0=DoubleExpGauss, 1=gauss)\n";
                }
                results[method].push_back(fits);

                if(method==0)
                    save_run_output(run, z, hist, fits, reference, references[method][references[0].size()-1]);
            }

        }

        if (results[0].empty()) throw runtime_error("Nessuna run in run.dat");
        
        for (int method=0;method<2;++method) {
            
            const string name = method==0 ? "doubleexpgauss" : "gauss";
            
            ofstream delta("delta_"+name+".txt"), chi("chi2_ndf_"+name+".txt");
            
            if (!delta || !chi) throw runtime_error("Errore apertura file di output");
            
            delta << setprecision(10); chi << setprecision(10);
            
            for (size_t r =0; r< references[method].size(); r++ ) {
                for (int b=0;b<n_theta;++b) {
                    if (b) { delta << ' '; chi << ' '; }
                    delta << results[method][r][b].peak-references[method][r].peak;
                    chi << results[method][r][b].chi2_ndf;
                }
                delta << '\n'; chi << '\n';
            }
            
            delta.close(); chi.close();
            if (!delta || !chi) throw runtime_error("Errore scrittura file di output");
        }
    } catch (const exception& e) { cerr << e.what() << '\n'; return 1; }
}