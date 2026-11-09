# Compilazione e uso dei programmi principali

Esempi da eseguire dalla radice del repository con un ambiente ROOT già configurato. `root-config --version` deve funzionare; i programmi di ricostruzione richiedono Minuit2. I comandi sono separati dall’esecuzione sui dati, che richiede gli input elencati in [DATI_E_PERCORSI.md](DATI_E_PERCORSI.md).

Gli eseguibili vengono collocati in `build/`, esclusa da Git. I comandi di esecuzione possono produrre o sostituire risultati: negli esempi viene usata `scratch/`, anch’essa esclusa da Git, quando il programma permette di scegliere l’output.

## Mappa correttiva NLL

```bash
mkdir -p build scratch/mappa_nll
g++ -O2 -std=c++17 ricostruzione/mappa_nll/fit.cpp \
  -o build/fit_mappa $(root-config --cflags --libs)

build/fit_mappa "$HOME/juno/prova/output" \
  ricostruzione/mappa_nll/pdf_parameters.txt scratch/mappa_nll
```

Il programma legge `output_*.root` e scrive le tabelle della mappa. Il [notebook](../ricostruzione/mappa_nll/mappa_e_pdf.ipynb) usa invece le tabelle storiche già incluse nella propria cartella. Per consultarlo non occorre ricalcolare la mappa.

La variante in `calibrazione/nll/test.cpp` accetta gli stessi tre tipi di argomento:

```bash
mkdir -p build scratch/nll
g++ -O2 -std=c++17 calibrazione/nll/test.cpp \
  -o build/fit_nll $(root-config --cflags --libs)
build/fit_nll "$HOME/juno/prova/output" \
  ricostruzione/mappa_nll/pdf_parameters.txt scratch/nll
```

## Ricostruzione della posizione

```bash
mkdir -p build scratch/ricostruzione
g++ -O3 -std=c++17 ricostruzione/reco_approx.cpp \
  -o build/reco_approx $(root-config --cflags --libs) -lMinuit2
g++ -O3 -std=c++17 ricostruzione/reco_corrected.cpp \
  -o build/reco_corrected $(root-config --cflags --libs) -lMinuit2
```

Esempio sulla run 14449, usando i dati e la tabella PDF dell’installazione originale:

```bash
build/reco_approx "$HOME/juno/prova/output/output_14449.root" \
  calibrazione/src/pmt_positions.csv "$HOME/juno/prova/pdf_table.bin" \
  scratch/ricostruzione/reco_14449.root 100 1

build/reco_corrected "$HOME/juno/prova/output/output_14449.root" \
  calibrazione/src/pmt_positions.csv "$HOME/juno/prova/pdf_table.bin" \
  scratch/ricostruzione/reco_14449_corrected.root 100 1 \
  scratch/ricostruzione/reco_14449.root \
  ricostruzione/map_positive/map_positive.dat
```

`100` limita il campione di eventi; `1` abilita il secondo punto iniziale del fit. Usare `0` per non limitare gli eventi. La seconda ricostruzione richiede l’output della prima. La tabella binaria `pdf_table.bin` è un input esterno di circa 35 MB e non viene generata dai sorgenti inclusi in questa selezione.

## Estrazione dei residui per la ricostruzione

```bash
mkdir -p build
g++ -O3 -std=c++17 ricostruzione/macro2.cpp \
  -o build/estrai_residui $(root-config --cflags --libs)
```

Firma del comando:

```text
build/estrai_residui posizioni_pmt lista_run directory_input directory_output numero_file
```

La lista delle run ha colonne `run z_mm picco_energia`; una copia è in `calibrazione/src/output_con_picchi.txt`. La struttura e i nomi dei ROOT grezzi devono corrispondere a quelli attesi dal programma. Questo estrattore usa un indice 1.60 e salva informazioni per evento; non è intercambiabile senza verifica con l’estrattore storico in `calibrazione/src/macro.cpp`.

## Calibrazione temporale con fit dei picchi

```bash
mkdir -p build
g++ -O2 -std=c++17 calibrazione/src/fit.cpp \
  -o build/fit_tempi $(root-config --cflags --libs) -lMinuit2
g++ -O2 -std=c++17 calibrazione/src/macro.cpp \
  -o build/estrai_tempi $(root-config --cflags --libs)
g++ -O2 -std=c++17 calibrazione/src/conv.cpp \
  -o build/converti_mappa $(root-config --cflags --libs)
```

`fit_tempi` e `converti_mappa` devono essere avviati da `calibrazione/src/`: usano percorsi relativi incorporati nel codice. Il fit legge `output3/output_*.root` e riscrive le tabelle `delta_*.txt`, `chi2_ndf_*.txt` e i ROOT dei fit. Per preservare i risultati inclusi nel repository, eseguirli in una copia di lavoro con la stessa disposizione dei dati.

## Geometria senza ROOT

```bash
mkdir -p build
g++ -O2 -std=c++11 geometria/calcolo/Main.cpp -o build/tof_geometrico
g++ -O2 -std=c++11 geometria/preparazione/a.cpp -o build/seleziona_hamamatsu
g++ -O2 -std=c++11 geometria/preparazione/test.cpp -o build/seleziona_sezione
```

Consultare il rispettivo `main` per gli argomenti e la directory di lavoro. I file `JVertex.cc` e `JVertex.h` in `riferimenti/` richiedono invece la compilazione all’interno del framework JUNO; non rientrano nei comandi autonomi sopra.
