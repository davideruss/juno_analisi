# JVertex approssimato, prima ricostruzione SENZA mappa

Ricostruzione evento per evento dei ROOT `output/output_RUN.root`, raggruppando gli hit per `(run, evtID)`. Usa solo gli Hamamatsu già salvati, carica per hit, PDF stimate dalla figura e ToF rettilineo con n=1.60. Tutti gli eventi disponibili vengono ricostruiti: nessun nuovo taglio energetico o sulla distanza dalla sorgente viene applicato.

## Likelihood e recupero dei tempi

Gli input contengono `residual_times = FHT - ToF_nominale`. Recuperiamo:

`FHT = residual_times + |r_PMT-(0,0,z_nominale)|*1.60/299.792458`.

Poi minimizziamo, separatamente per evento:

`NLL(x,y,z,t0) = -sum_i log(max(p_k(FHT_i - |r_PMT-r|*1.60/299.792458 - t0), 1e-9))`.

Le quattro coordinate x,y,z,t0 sono LIBERE. Il t0 salvato da JVertex è conservato per confronto e controlli, non è fissato e non viene usato come seed. La posizione nominale serve unicamente a ripristinare FHT e a calcolare gli errori della ricostruzione; non entra nel seed né in vincoli sulla posizione cercata. Non viene letta o applicata alcuna mappa correttiva.

L'origine temporale comune delle PDF è assorbibile da t0 libero (lontano dai limiti del parametro); la forma delle PDF e le differenze tra cariche restano approssimazioni del test.

## Minimizzatore

Minuit2 Simplex seguito da Migrad, con un secondo tentativo Migrad se necessario. Ogni evento viene provato con due seed spaziali: baricentro pesato con la carica dei PMT selezionati e centro del detector. In entrambi t0 parte da zero. Si conserva la soluzione con NLL minore, registrando differenza di NLL e distanza fra le due soluzioni.

Questo controllo è importante: nel campione di prova centrale i due seed danno minimi equivalenti, ma vicino al bordo alcuni eventi hanno minimi locali diversi anche quando Minuit segnala convergenza. Due seed riducono il problema senza garantire matematicamente il minimo globale.

Limiti come nel JVertex fornito: ciascuna coordinata fra -20000 e 20000 mm, t0 fra -500 e 500 ns. Non è imposto un vincolo sferico aggiuntivo. ErrorDef=0.5. È un'implementazione approssimata della likelihood e della procedura di minimizzazione, non una copia identica dell'intero software JVertex: strategia e budget dei tentativi sono più contenuti e il baricentro è normalizzato alla carica degli Hamamatsu salvati (la carica totale originale non è disponibile negli output).

Le log-PDF non troncate sono interpolate con Hermite cubica su griglia da 0.05 ns e poi limitate al pavimento 1e-9. Confronto su 10000 punti con SciPy: errore massimo della log-likelihood ~1.33e-8. Le classi sono q<2, [2,3), ..., q>=10, come nel codice JVertex fornito.

## Output

`results_reco_baseline/reco_RUN.root`: TTree `reco`, una riga per evento. Contiene posizione e t0 ricostruiti, t0 originale, NLL, EDM, stato e numero di chiamate, seed, diagnostica del secondo seed e flag di convergenza. I metadati indicano posizione nominale, indice e numero di eventi di input/ricostruiti. Il ROOT originale non viene modificato.

`success=1` richiede status Minuit 0 o 1, NLL finita, EDM <=0.001, soluzione non entro 0.1 mm/ns dai limiti cartesiani/temporali, almeno 20 hit. I fit che non soddisfano questo controllo restano negli output. Il numero di chiamate salvato riguarda la sequenza Simplex/Migrad che ha prodotto la soluzione selezionata, non la somma delle due partenze.

`nll_stored_nominal` valuta la NLL alla posizione nominale con t0 originale. `nll_at_nominal_fitted_t0` usa invece t0 del fit libero: non è un fit aggiuntivo di t0 a posizione fissata. Questi valori sono diagnostici e non guidano l'ottimizzazione.

`summary.csv` e `summary.json`: bias medio, deviazione standard, semilarghezza centrale 68% `(P84-P16)/2`, errore standard sulla media e raggio contenente il 68% degli errori di posizione. Il raggio 68% include anche il bias e non coincide con una sigma per coordinata. Il JSON conserva statistiche sia per tutti i fit sia per quelli convergenti. I grafici usano i fit convergenti e mostrano separatamente la frazione di convergenza.

`baseline_summary.png`: bias, larghezze 68% e convergenza in funzione dello z nominale. `central_residuals.png`: distribuzioni degli errori nella run centrale. CSV per evento disponibili accanto ai ROOT, per analisi successive e confronto appaiato con la seconda ricostruzione.

## Riproduzione

```bash
cd ~/juno/prova
bash build_reco.sh
python3 run_reco.py --out results_reco_baseline --workers 2 --resume
python3 export_reco_csv.py results_reco_baseline
MPLCONFIGDIR=/tmp/juno-mpl OPENBLAS_NUM_THREADS=1 ../.venv/bin/python summarize_reco.py results_reco_baseline
```

Il default è tutti gli eventi (`--max-events 0`) e due seed (`--second-start 1`). `--max-events 100` seleziona un campione casuale riproducibile per run (seed 20261001), non i primi eventi. `--workers` controlla il numero massimo di run simultanee (default 1); ogni fit Minuit è sequenziale. Due processi richiedono indicativamente alcune centinaia di MB, dipendendo dalla run. `--resume` riutilizza solo output con manifest corrispondente a input, parametri, eseguibile, geometria e tabella PDF. Un processo interrotto non lascia una run marcata come completata.

Prima di ricompilare/rigenerare la tabella per una prova diversa, usare una cartella di output nuova per conservare il confronto. I risultati vengono sovrascritti quando non riutilizzabili.

## Test

`results_reco_baseline/checks/verification_reco.json`: confronto della tabella PDF e due prove con dati sintetici generati in posizioni note `(200,-150,100)` e `(200,-150,16100)` mm. Per ciascuna 100 eventi da 600 hit; tutti convergono e i bias medi delle coordinate sono entro 19 mm, compatibili con le fluttuazioni del campione. Il test verifica anche il recupero di FHT dal formato raggruppato per PMT.

`results_reco_baseline/checks/start_checks.json`: confronto dei due seed in 100 eventi centrali e 100 periferici reali. I file `pilot_center.root` e `pilot_edge.root` nella stessa cartella `checks` conservano i dettagli. `pilot_reproducibility.json` conferma che i risultati degli stessi 100 eventi nei campioni e nella ricostruzione completa coincidono.

Questa è la baseline per la procedura concordata: prima ricostruzione libera; scelta della mappa dalla prima posizione ricostruita; seconda ricostruzione. Il secondo passaggio non è ancora implementato qui. Per valutarlo senza riutilizzare il campione di calibrazione della mappa, mantenere separati eventi di calibrazione e di test tramite evtID. La mappa finale precedente usa tutti gli eventi, mentre i suoi controlli pari/dispari erano separati.

Per ripetere i test sintetici:

```bash
g++ -O3 -std=c++17 check_pdf.cpp -o check_pdf.exe $(root-config --cflags --libs) -lMinuit2
MPLCONFIGDIR=/tmp/juno-mpl OPENBLAS_NUM_THREADS=1 ../.venv/bin/python check_reco.py
```

## Numeri della baseline completa

39 run, 293285 eventi ricostruiti; 292663 (99.79%) superano i controlli numerici di convergenza. 622 non li superano, di cui 528 hanno una soluzione vicina ai limiti dei parametri. I dati di tutti gli eventi sono conservati.

Nella run centrale: bias medi x=+1.64 cm, y=+0.26 cm, z=-1.27 cm; deviazioni standard 30.92, 34.40, 33.27 cm; semilarghezze centrali 68% 26.83, 26.41, 28.10 cm. Le code spiegano la differenza tra deviazione standard e larghezza del nucleo. Il massimo bias medio assoluto su z nelle 39 run è 44.57 cm.

Diagnostica aggiuntiva: 571 eventi hanno NLL finale peggiore di quella valutata alla posizione nominale con t0 originale; 362 di questi passano comunque i controlli numerici del minimizzatore. Indicano minimi locali residui, non una garanzia di minimo globale. Questo confronto usa la posizione nominale solo DOPO il fit; non guida i seed e non esclude automaticamente gli eventi dalle statistiche. I grafici principali usano i fit numericamente convergenti, compresi questi 362, e mostrano quindi anche i limiti attuali della procedura. Per analisi delle code o confronti molto precisi va considerata questa diagnostica.

I due seed differiscono di oltre 0.001 in NLL in 12152 eventi; il secondo migliora la NLL di oltre 0.001 in 4948 eventi. Le posizioni periferiche sono le più sensibili. Non è stato introdotto alcun vincolo basato sulla posizione vera per migliorare artificialmente la ricostruzione.
