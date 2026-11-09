// Compilazione (ROOT e C++17):
// g++ -O2 -Wall -Wextra -std=c++17 fit.cpp -o /tmp/fit_mappa $(root-config --cflags --libs)
// Esecuzione dalla cartella src:
// /tmp/fit_mappa ../output pdf_parameters.txt .

#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include <TFile.h>
#include <TParameter.h>
#include <TTree.h>
#include <Math/BrentMinimizer1D.h>
#include <Math/Functor.h>

using namespace std;
namespace fs = std::filesystem;

// Impostazioni della prova: 40 anelli in cos(theta), PDF dipendenti dalla carica.
constexpr int n_anelli = 40;
constexpr int n_pdf = 10;
constexpr int riferimento_min = 18;
constexpr int riferimento_max = 21;
constexpr double bin_tempo = 0.25;    // ns; nessun taglio sulle code temporali
constexpr double indice = 1.60;
constexpr double epsilon_pdf = 1.e-9;
constexpr double tabella_min = -1000.;
constexpr double tabella_max = 10000.;
constexpr double passo_pdf = 0.025;   // ns

struct ParametriPDF {
    double mu, sigma, tau1, tau2, peso1;
};

struct Punto {
    int k;
    double tempo;
    long long conteggio;
};

struct Run {
    int numero = 0;
    double z = 0.;
    long long hits = 0;
    array<vector<Punto>, n_anelli> istogrammi;
    vector<double> delta, relativa;
};

struct Risultato {
    double delta, nll_prima, nll_dopo;
};

// Gaussiana convoluta con un'esponenziale, normalizzata a integrale 1.
double exp_gauss(double t, double mu, double sigma, double tau) {
    const double a = (mu + sigma * sigma / tau - t) / (sqrt(2.) * sigma);
    const double esponente = (mu - t) / tau + sigma * sigma / (2. * tau * tau);

    // Forma stabile per evitare exp(grande) * erfc(grande) = inf * 0.
    if (a > 20.) {
        const double u = 1. / (2. * a * a);
        const double serie = 1. - u + 3. * u * u - 15. * pow(u, 3) + 105. * pow(u, 4);
        const double erfc_scalata = serie / (sqrt(acos(-1.)) * a);
        return exp(-0.5 * pow((t - mu) / sigma, 2)) * erfc_scalata / (2. * tau);
    }

    return exp(esponente) * erfc(a) / (2. * tau);
}

class PDF {
    array<vector<double>, n_pdf> log_tabella;

public:
    explicit PDF(const string& nome) {
        ifstream file(nome);
        if (!file) throw runtime_error("Impossibile aprire " + nome);

        vector<ParametriPDF> parametri;
        string riga;
        while (getline(file, riga)) {
            if (riga.find_first_not_of(" \t\r") == string::npos) continue;
            if (riga[riga.find_first_not_of(" \t")] == '#') continue;
            istringstream lettura(riga);
            ParametriPDF p;
            if (!(lettura >> p.mu >> p.sigma >> p.tau1 >> p.tau2 >> p.peso1))
                throw runtime_error("Riga PDF non valida");
            if (!isfinite(p.mu) || !isfinite(p.sigma) || !isfinite(p.tau1) ||
                !isfinite(p.tau2) || !isfinite(p.peso1) || p.sigma <= 0. ||
                p.tau1 <= 0. || p.tau2 <= 0. || p.peso1 < 0. || p.peso1 > 1.)
                throw runtime_error("Parametri PDF non validi");
            parametri.push_back(p);
        }
        if (parametri.size() != n_pdf) throw runtime_error("Servono 10 PDF");

        const int n = lround((tabella_max - tabella_min) / passo_pdf) + 1;
        for (int k = 0; k < n_pdf; ++k) {
            const auto& p = parametri[k];
            log_tabella[k].resize(n);
            for (int i = 0; i < n; ++i) {
                const double t = tabella_min + i * passo_pdf;
                const double densita = p.peso1 * exp_gauss(t, p.mu, p.sigma, p.tau1)
                    + (1. - p.peso1) * exp_gauss(t, p.mu, p.sigma, p.tau2);
                log_tabella[k][i] = log(max(densita, epsilon_pdf));
            }
            if (abs(log_tabella[k].front() - log(epsilon_pdf)) > 1.e-10 ||
                abs(log_tabella[k].back() - log(epsilon_pdf)) > 1.e-10)
                throw runtime_error("Ampliare l'intervallo della tabella PDF");
        }
    }

    double log_pdf(double t, int k) const {
        if (t <= tabella_min || t >= tabella_max) return log(epsilon_pdf);
        const double posizione = (t - tabella_min) / passo_pdf;
        const int i = int(posizione);
        const double frazione = posizione - i;
        return (1. - frazione) * log_tabella[k][i] + frazione * log_tabella[k][i + 1];
    }
};

// Lettura del ROOT raggruppato per PMT prodotto da macro2.cpp.
Run leggi_run(const fs::path& percorso) {
    TFile file(percorso.c_str(), "READ");
    if (file.IsZombie()) throw runtime_error("Impossibile aprire " + percorso.string());

    auto* tree = file.Get<TTree>("pmtRes");
    auto* z = file.Get<TParameter<double>>("source_z_mm");
    auto* n = file.Get<TParameter<double>>("refractive_index");
    if (!tree || !z || !n || !isfinite(z->GetVal()) || !isfinite(n->GetVal()) ||
        abs(n->GetVal() - indice) > 1.e-8)
        throw runtime_error("Metadati mancanti o indice diverso da 1.60");

    Run run;
    run.numero = stoi(percorso.stem().string().substr(7));
    run.z = z->GetVal();
    int pmt_id;
    double cos_theta;
    vector<double> *tempi = nullptr, *t0 = nullptr, *carica = nullptr, *eventi = nullptr;

    auto collega = [&](const char* nome, auto* indirizzo) {
        if (tree->SetBranchAddress(nome, indirizzo) < 0)
            throw runtime_error(string("Branch mancante: ") + nome);
    };
    collega("pmt_id", &pmt_id);
    collega("cos_theta", &cos_theta);
    collega("residual_times", &tempi);
    collega("t0", &t0);
    collega("charge", &carica);
    collega("evtID", &eventi);

    // Chiave = (classe di carica, bin temporale). Si memorizzano solo bin occupati.
    array<map<pair<int, long long>, long long>, n_anelli> conteggi;
    unordered_map<long long, double> t0_evento;
    set<int> pmt_letti;

    for (Long64_t riga = 0; riga < tree->GetEntries(); ++riga) {
        if (tree->GetEntry(riga) <= 0) throw runtime_error("Errore lettura PMT");
        if (!tempi || !t0 || !carica || !eventi || tempi->size() != t0->size() ||
            tempi->size() != carica->size() || tempi->size() != eventi->size())
            throw runtime_error("Vettori PMT non allineati");
        if (!isfinite(cos_theta) || cos_theta < -1. || cos_theta > 1. ||
            pmt_id < 0 || !pmt_letti.insert(pmt_id).second)
            throw runtime_error("PMT duplicato o cos(theta) non valido");

        const int anello = min(n_anelli - 1, int((cos_theta + 1.) * n_anelli / 2.));
        set<long long> eventi_pmt;
        for (size_t j = 0; j < tempi->size(); ++j) {
            const double t = (*tempi)[j], zero = (*t0)[j];
            const double q = (*carica)[j], e = (*eventi)[j];
            if (!isfinite(t) || !isfinite(zero) || !isfinite(q) || !isfinite(e) ||
                abs(t) > 1.e8 || abs(zero) > 1.e8 || q <= 0. ||
                e < 0. || e > 1.e15 || floor(e) != e)
                throw runtime_error("Hit non valido");

            const long long id = static_cast<long long>(e);
            if (!eventi_pmt.insert(id).second) throw runtime_error("Evento/PMT duplicato");
            auto [it, nuovo] = t0_evento.emplace(id, zero);
            if (!nuovo && abs(it->second - zero) > 1.e-8)
                throw runtime_error("t0 non comune ai PMT dello stesso evento");

            const int k = q >= 10. ? 9 : max(0, int(floor(q)) - 1);
            // residual_times contiene GIA' FHT - ToF(n=1.60) alla posizione nota.
            // Non sottrarre nuovamente il ToF; t0 di JVertex resta fisso nel fit.
            const double residuo = t - zero;
            const long long bin = static_cast<long long>(floor(residuo / bin_tempo));
            conteggi[anello][{k, bin}]++;
            run.hits++;
        }
    }

    for (int a = 0; a < n_anelli; ++a) {
        for (const auto& [chiave, conta] : conteggi[a]) {
            const auto [k, bin] = chiave;
            run.istogrammi[a].push_back({k, (bin + 0.5) * bin_tempo, conta});
        }
    }
    tree->ResetBranchAddresses();
    delete tempi;
    delete t0;
    delete carica;
    delete eventi;
    return run;
}

// Un solo parametro per anello: lo spostamento temporale comune a tutti gli eventi.
Risultato fit_anello(const vector<Punto>& punti, const PDF& pdf) {
    long long totale = 0;
    map<double, long long> tempi;
    for (const auto& p : punti) {
        totale += p.conteggio;
        tempi[p.tempo] += p.conteggio;
    }
    if (totale < 30) throw runtime_error("Meno di 30 hit nell'anello");

    auto nll = [&](double delta) {
        double somma = 0.;
        for (const auto& p : punti)
            somma -= p.conteggio * pdf.log_pdf(p.tempo - delta, p.k);
        return somma;
    };

    long long cumulata = 0;
    double mediana = 0.;
    for (const auto& [tempo, conta] : tempi) {
        cumulata += conta;
        if (cumulata >= totale / 2.) {
            mediana = tempo;
            break;
        }
    }

    // Ricerca larga, poi minimizzazione locale di Brent.
    const double centro = mediana - 290.;
    const double inizio = centro - 300.;
    int migliore = 0;
    double minimo = nll(inizio);
    for (int i = 1; i <= 120; ++i) {
        const double valore = nll(inizio + 5. * i);
        if (valore < minimo) {
            minimo = valore;
            migliore = i;
        }
    }
    if (migliore == 0 || migliore == 120)
        throw runtime_error("Minimo sul bordo: ampliare la scansione");

    ROOT::Math::Functor1D funzione(nll);
    ROOT::Math::BrentMinimizer1D minimizzatore;
    minimizzatore.SetNpx(20);
    minimizzatore.SetFunction(funzione, inizio + 5. * (migliore - 1),
                             inizio + 5. * (migliore + 1));
    if (!minimizzatore.Minimize(200, 1.e-6, 1.e-9))
        throw runtime_error("Minimizzazione non convergente");

    double delta = minimizzatore.XMinimum();
    // La tabella interpolata ha piccoli spigoli ogni 0.025 ns. Controlliamo
    // anche i nodi vicini per non fermarci su un minimo locale numerico.
    const double nodo = round(delta / passo_pdf) * passo_pdf;
    for (int j = -4; j <= 4; ++j) {
        const double candidato = nodo + j * passo_pdf;
        if (nll(candidato) < nll(delta)) delta = candidato;
    }
    Risultato risultato{delta, nll(0.), nll(delta)};
    if (!isfinite(delta) || risultato.nll_dopo > risultato.nll_prima + 1.e-3)
        throw runtime_error("NLL peggiorata: controllare la minimizzazione");
    return risultato;
}

void calcola_mappa(Run& run, const PDF& pdf) {
    vector<Punto> riferimento;
    for (int a = riferimento_min; a <= riferimento_max; ++a)
        riferimento.insert(riferimento.end(), run.istogrammi[a].begin(), run.istogrammi[a].end());

    // Il riferimento viene fittato sugli hit uniti, non mediando quattro delta.
    const double offset = fit_anello(riferimento, pdf).delta;
    double guadagno_nll = 0.;
    for (int a = 0; a < n_anelli; ++a) {
        const Risultato fit = fit_anello(run.istogrammi[a], pdf);
        run.delta.push_back(fit.delta);
        run.relativa.push_back(fit.delta - offset);
        guadagno_nll += fit.nll_prima - fit.nll_dopo;
    }
    cout << "Run " << run.numero << ", z = " << run.z << " mm, " << run.hits
         << " hit, guadagno NLL = " << guadagno_nll << '\n' << flush;
}

void scrivi_matrice(const fs::path& nome, const vector<vector<double>>& righe) {
    ofstream file(nome);
    if (!file) throw runtime_error("Impossibile scrivere " + nome.string());
    file << setprecision(17);
    for (const auto& riga : righe) {
        for (size_t j = 0; j < riga.size(); ++j) file << (j ? " " : "") << riga[j];
        file << '\n';
    }
    file.close();
    if (!file) throw runtime_error("Errore scrittura " + nome.string());
}

void salva_mappe(const vector<Run>& runs, const fs::path& cartella) {
    fs::create_directories(cartella);
    vector<vector<double>> coordinate, assoluta, relativa;
    map<double, vector<double>> per_z;
    set<double> raggi;
    for (const auto& run : runs) {
        coordinate.push_back({double(run.numero), run.z});
        assoluta.push_back(run.delta);
        relativa.push_back(run.relativa);
        if (!per_z.emplace(run.z, run.relativa).second)
            throw runtime_error("Due run con lo stesso z: definire come combinarle");
        raggi.insert(abs(run.z));
    }

    vector<vector<double>> positiva, r, coseni;
    for (double radius : raggi) {
        const auto pos = per_z.find(radius);
        const auto neg = per_z.find(-radius);
        vector<double> riga;
        for (int a = 20; a < n_anelli; ++a) {
            double somma = 0.;
            int lati = 0;
            if (pos != per_z.end()) { somma += pos->second[a]; ++lati; }
            if (neg != per_z.end()) { somma += neg->second[39 - a]; ++lati; }
            riga.push_back(somma / lati);
        }
        if (pos == per_z.end() || neg == per_z.end())
            cout << "r = " << radius << " mm: disponibile un solo lato\n";
        positiva.push_back(riga);
        r.push_back({radius});
    }
    for (int a = 20; a < n_anelli; ++a) coseni.push_back({-1. + (a + 0.5) / 20.});

    scrivi_matrice(cartella / "run_z.txt", coordinate);
    scrivi_matrice(cartella / "delta_nll.txt", assoluta);
    scrivi_matrice(cartella / "delta_relative.txt", relativa);
    scrivi_matrice(cartella / "delta_r.txt", positiva);
    scrivi_matrice(cartella / "r.txt", r);
    scrivi_matrice(cartella / "cos_theta.txt", coseni);
}

int main(int argc, char** argv) {
    try {
        if (argc != 4) {
            cerr << "Uso: " << argv[0] << " cartella_root pdf_parameters.txt cartella_output\n";
            return 1;
        }
        const PDF pdf(argv[2]);
        vector<fs::path> percorsi;
        for (const auto& voce : fs::directory_iterator(argv[1])) {
            const string nome = voce.path().filename().string();
            if (voce.is_regular_file() && nome.rfind("output_", 0) == 0 && voce.path().extension() == ".root")
                percorsi.push_back(voce.path());
        }
        sort(percorsi.begin(), percorsi.end());
        if (percorsi.empty()) throw runtime_error("Nessun file output_*.root trovato");

        vector<Run> runs;
        for (const auto& percorso : percorsi) {
            Run run = leggi_run(percorso);
            calcola_mappa(run, pdf);
            // Gli istogrammi non servono piu': manteniamo in memoria solo la mappa.
            run.istogrammi = {};
            runs.push_back(std::move(run));
        }
        sort(runs.begin(), runs.end(), [](const Run& a, const Run& b) { return a.z < b.z; });
        salva_mappe(runs, argv[3]);
        cout << "Salvate " << runs.size() << " run nella cartella " << argv[3] << '\n';
    } catch (const exception& errore) {
        cerr << "Errore: " << errore.what() << '\n';
        return 1;
    }
    return 0;
}
