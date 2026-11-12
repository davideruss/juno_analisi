# Eseguire le quattro parti

I notebook richiedono solo le dipendenze di `requirements-notebooks.txt`. Per i C++ servono `g++`, ROOT e `root-config`; il fit dei picchi usa Minuit2.

## Compilazione

Dalla radice del repository:

```bash
mkdir -p build
g++ -O2 -std=c++17 00_bias/src/macro_positions.cc -o build/bias $(root-config --cflags --libs)
g++ -O2 -std=c++17 01_fermat/src/main.cpp -o build/fermat $(root-config --cflags --libs)
g++ -O2 -std=c++17 02_picchi/src/macro.cpp -o build/estrai_picchi $(root-config --cflags --libs)
g++ -O2 -std=c++17 02_picchi/src/fit.cpp -o build/picchi $(root-config --cflags --libs) -lMinuit2
g++ -O2 -std=c++17 03_likelihood/src/macro2.cpp -o build/estrai_likelihood $(root-config --cflags --libs)
g++ -O2 -std=c++17 03_likelihood/src/fit.cpp -o build/likelihood $(root-config --cflags --libs)
```

## Input e output

| Eseguibile | Input essenziale | Output |
|---|---|---|
| `bias` | Lista a due colonne: run, z in mm. I ROOT `CdEvents` sono cercati nel percorso `/storage/gpfs_data/...` incorporato nel sorgente. | ROOT con `bias` e `resolution`. |
| `fermat` | Lista delle z e geometria PMT. | ROOT con tempi rettilinei, tempi di Fermat e differenza. |
| `estrai_picchi` | Geometria e lista a tre colonne: run, z, picco energetico. Cerca i file `tof_RUN_nLS1.597_nW1.33_N.root` nella directory corrente. | ROOT `output_RUN.root` con i residui. |
| `picchi` | Nessun argomento: cerca `output_con_picchi.txt` e `output3/output_*.root` nella directory corrente. | Fit ROOT e tabelle `delta_*.txt`, `chi2_ndf_*.txt`. |
| `estrai_likelihood` | Geometria, lista delle run, directory di input/output e numero di file. | ROOT con residui, cariche, t0 e ID degli eventi. |
| `likelihood` | Directory ROOT, parametri PDF e directory di output. | Mappe NLL assolute, relative e simmetrizzate. |

Esempi con i dati locali originali:

```bash
mkdir -p scratch
build/fermat 01_fermat/dati/z_mm.txt dati_comuni/pmt_positions.csv scratch/fermat.root
build/likelihood "$HOME/juno/prova/output" 03_likelihood/dati/pdf_parameters.txt scratch/likelihood
```

Per il fit dei picchi, preparare una directory di lavoro con una copia di `dati_comuni/run_calibrazione.txt` chiamata `output_con_picchi.txt` e i ROOT di `~/juno/src/output3/` in `output3/`. Avviare lì l’eseguibile `picchi` usando il suo percorso. Il programma riscrive i risultati nella directory corrente.

Le tabelle incluse fotografano i risultati originali. I C++ producono nuovi ROOT/tabelle, ma non aggiornano automaticamente le esportazioni sintetiche dei notebook. I due estrattori dei residui conservano convenzioni differenti e non sono intercambiabili.
