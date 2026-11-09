#include <iostream>
#include <fstream>

#include "TH2D.h"
#include "TFile.h"
#include "TFile.h"


using namespace std;


void convert_txt() {
    // 1. Definisci i binning

    // Asse Y (ORA): cos(theta) da -1 a 1 in 20 bin (centri da -0.95 a 0.95)
    const int n_cos = 40;
    double cos_edges[n_cos + 1];
    for (int i = 0; i <= n_cos; ++i) {
        cos_edges[i] = -1.0 + i * 0.05;
    }

    // Asse X (ORA): z_mm in 39 bin a passo variabile
    // double z_centers[39] = {
    //     -17300, -17100, -16900, -16700, -16500, -16300, -16100, -15900, -15700, -15500,
    //     -15000, -14000, -13000, -11000,  -9000,  -6000,  -3000,  -1000,      0,   1000,
    //       3000,   6000,   9000,  11000,  13000,  14000,  15000,  15500,  15700,  15900,
    //      16100,  16300,  16500,  16700,  16900,  17100,  17300,  17500,  17700
    // };

    double z_centers[21] = {
        0,   1000,
        3000,   6000,   9000,  11000,  13000,  14000,  15000,  15500,  15700,  15900,
        16100,  16300,  16500,  16700,  16900,  17100,  17300,  17500,  17700
    };
    
    const int n_z = 21;
    double z_edges[n_z + 1];
    z_edges[0] = z_centers[0] - (z_centers[1] - z_centers[0]) / 2.0;
    for (int i = 0; i < n_z - 1; ++i) {
        z_edges[i + 1] = (z_centers[i] + z_centers[i + 1]) / 2.0;
    }
    z_edges[n_z] = z_centers[n_z - 1] + (z_centers[n_z - 1] - z_centers[n_z - 2]) / 2.0;

    // 2. Crea l'istogramma TH2D (Attenzione all'ordine: prima X (z), poi Y (cos_theta))
    TH2D* h2 = new TH2D("h2_delta", "Delta;z [mm];cos(#theta)", n_z, z_edges, n_cos, cos_edges);

    // 3. Leggi il file di testo e riempi il TH2D
    std::ifstream file("../nll/risultati/delta_rel.txt");
    double val;
    
    // Le righe del file sono z, le colonne sono cos(theta)
    for (int iz = 0; iz < n_z; ++iz) {
        for (int icos = 0; icos < n_cos; ++icos) {
            if (file >> val) {
                // SetBinContent(binX, binY, value) -> Ora binX è iz, binY è icos
                h2->SetBinContent(iz + 1, icos + 1, val);
            } else {
                std::cerr << "Errore: fine inattesa del file txt." << std::endl;
                return;
            }
        }
    }
    file.close();
    
    // 4. Salva il TH2D in un file .root
    TFile* f_out = new TFile("../nll/risultati/delta_2.root", "RECREATE");
    h2->Write();
    f_out->Close();
    
    std::cout << "Fatto! Creato il file delta_doubleexpgauss.root con z su X e cos(theta) su Y." << std::endl;
}

int main (){

    convert_txt();
    return 0;
    
}