# Mappa NLL con carica per hit e t0 JVertex fisso

Input: i 39 file `output/output_RUN.root` generati con la macro aggiornata. Non modifica input, macro o risultati legacy.

## Significato

La likelihood è `-sum log(max(p_k(residual_times - t0 - delta_anello), 1e-9))`. `residual_times` contiene già FHT-ToF con n=1.60: il ToF NON viene sottratto una seconda volta. La carica seleziona PDF1 per q<2, PDF2 per 2<=q<3, ..., PDF10 per q>=10. Quaranta anelli uniformi in cos(theta_z), come in src. Il t0 salvato è fisso, comune ai PMT dello stesso evento; non viene riadattato.

`delta_nll.txt` e `h2_delta` nel ROOT contengono gli shift effettivamente minimizzati con t0 fisso. Nessun vincolo di riferimento è imposto: con t0 fisso non c'è degenerazione globale da rimuovere. Il segno è quello da SOTTRARRE al residuo.

`delta_relative.txt` e `h2_delta_relative` sottraggono a ogni riga lo shift ottenuto dal fit congiunto degli anelli 18–21. Servono per confrontare la FORMA con la mappa precedente. Applicarli da soli con t0 fisso non riproduce la NLL del fit: per recuperare gli shift originali occorre sommare `reference_shift_ns`, diverso per ogni run.

## Limiti delle PDF e dell'origine temporale

Le PDF sono ancora le stime da immagine in `pdf/pdf_estimate.json`: curve 1,2,10 digitalizzate, 3–9 interpolate. Manteniamo provvisoriamente l'origine temporale della figura (`pdf_origin_ns=0`); i primi residui corretti sono compatibili con quell'asse, ma questo NON dimostra che le funzioni ufficiali di JVertex coincidano con le nostre. Un offset comune può assorbire differenze di origine e forma delle PDF, non soltanto effetti del detector. Le code fuori dall'immagine sono estrapolate. La mappa non è una calibrazione assoluta validata.

Il pavimento 1e-9 replica la regola della NLL del JVertex fornito, applicata qui a densità normalizzate; la normalizzazione delle funzioni ufficiali non è verificata.

## Calcolo e controlli

- Lettura C++/ROOT dei vettori raggruppati per PMT; verifica uguaglianza delle lunghezze, metadato n=1.60, cariche positive, valori finiti, evtID interi, assenza di PMT duplicati/hit duplicati nello stesso PMT/evento, coerenza di t0 fra tutti i PMT dello stesso evento.
- Compressione in istogrammi per anello, classe di carica e parità di evtID, con bin temporali di 0.25 ns. Nessuna finestra di selezione temporale aggiuntiva; tutti gli hit vengono contati.
- Per t0 fisso la NLL è separabile fra gli anelli. Fit scalare per anello con scansione ampia e raffinamento del minimo.
- Interpolazione della log-PDF su una griglia di 0.025 ns per accelerare il fit; alle estremità la likelihood ha già raggiunto il pavimento.
- Mappa finale adattata a tutti gli eventi. Controllo separato: fit sui soli evtID pari, valutazione sugli evtID dispari. Gli hit di uno stesso evento non vengono divisi fra training e test.
- La verifica confronta la NLL su eventi esclusi dal fit con/senza correzione. È una verifica all'interno della stessa run, NON una validazione su run indipendenti né del bias della posizione ricostruita. Nessuna incertezza statistica/sistematica completa viene stimata.
- Il confronto con i vecchi risultati legacy coinvolge anche campioni/tagli diversi: non attribuire tutte le differenze al metodo NLL o all'uso della carica.

## File

`results_charge_t0/` contiene le due matrici, il ROOT, grafici, `run_z.txt` (ordine delle righe) e un riepilogo JSON. Per ogni run: `map.csv` (shift effettivo, shift relativo, popolazione e fit di training), `summary.json` con configurazione e PDF effettive, e istogrammi `hist.csv`/`hist.json` per ripetere rapidamente i fit. Nei TH2 con bin z variabili i centri geometrici non coincidono necessariamente con le posizioni nominali: usare `source_z_mm` salvato come TVectorD o `run_z.txt`.

## Riproduzione

```bash
cd ~/juno/prova
g++ -O2 -std=c++17 grouped_hist.cpp -o grouped_hist.exe $(root-config --cflags --libs)
MPLCONFIGDIR=/tmp/juno-mpl OPENBLAS_NUM_THREADS=1 ../.venv/bin/python fit_grouped_map.py --out results_charge_t0
MPLCONFIGDIR=/tmp/juno-mpl OPENBLAS_NUM_THREADS=1 ../.venv/bin/python finalize_grouped.py results_charge_t0
python3 export_grouped_root.py results_charge_t0
```

`--runs 14417` limita il calcolo alla run centrale; `--width 0.125` consente il controllo del binning in una cartella di output NUOVA. Gli istogrammi esistenti vengono riutilizzati solo se percorso, dimensione, timestamp del ROOT e binning coincidono. Usare comunque una cartella nuova quando cambia la selezione o il codice di estrazione. I risultati della verifica del binning sono in `verifiche_mappa_t0.json`.

## Risultati di questa esecuzione

39 run, 293285 eventi e 133231686 hit; tutti i 1560 bin della mappa hanno statistiche sufficienti. Gli shift con t0 fisso vanno da -8.450 a +5.125 ns; gli shift relativi da -3.925 a +10.025 ns. La NLL migliora su tutte le 39 run anche nel campione di eventi dispari esclusi dal fit di verifica.

Tra il 4.52% e l'8.07% degli hit di ciascuna run raggiunge il pavimento 1e-9 dopo la correzione; per questi hit la NLL è localmente costante. Sono inclusi nel calcolo, non tagliati dalla selezione.

Controllo binning nella run centrale: differenza massima degli shift 0.01251 ns passando da 0.25 a 0.125 ns. Sei controlli di minimizzazione con PDF valutata direttamente (anelli 0,20,39 nelle run 14417 e 14377): differenza massima rispetto alla lookup 0.01136 ns. Questi numeri misurano l'approssimazione numerica nei controlli effettuati, non l'incertezza fisica sulle correzioni.
