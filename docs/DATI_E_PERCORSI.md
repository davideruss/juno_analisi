# Dati, percorsi e condizioni di esecuzione

Questa raccolta conserva il codice originale. I percorsi di input, i parametri numerici e i nomi degli output non sono stati riscritti. Il contenuto dei notebook va letto anche come documentazione delle elaborazioni precedenti: non tutti sono pronti per un’esecuzione completa da una nuova macchina.

## Tabelle incluse

Le tabelle sono copie degli originali, senza trasformazioni. Sono state mantenute accanto ai programmi per rispettare i percorsi relativi.

| Cartella | Tabelle principali |
|---|---|
| `calibrazione/src/` | Geometria PMT, run e posizioni ACU, mappe temporali e chi² a 40 anelli. |
| `calibrazione/src/risultati/` | Mappe e chi² della variante a 20 anelli; usa la tabella delle posizioni nella cartella superiore. |
| `calibrazione/nll/risultati/` | Mappe NLL, mappe relative, coordinate radiali e angolari. |
| `geometria/` | Geometria PMT, sezioni e piccoli risultati numerici richiesti dai notebook. |
| `ricostruzione/mappa_nll/` | Parametri PDF e tutte le sette tabelle lette dal notebook. |
| `ricostruzione/map_positive/` | Mappa positiva, assi e metadati originali. |

I valori inclusi sono risultati storici; non sono stati rigenerati. Le geometrie ripetute sono intenzionali: centralizzarle richiederebbe modificare i programmi. Le mappe di calibrazione sono piccole e utili alla consultazione, mentre gli eventi ROOT rimangono esterni.

## Dati ROOT da fornire

Le destinazioni della tabella sono relative alla radice del repository. Per eseguire le sezioni indicate, copiare o collegare localmente soltanto i dati necessari. `.gitignore` esclude i ROOT anche quando sono nella cartella di lavoro.

| Origine in `~/juno/` | Destinazione o utilizzo |
|---|---|
| `src/output3/output_*.root` | `calibrazione/src/output3/`, input di `fit.cpp`. |
| `src/fit_run_14383.root`, `src/fit_run_14379.root` | `calibrazione/src/`, sezioni dei fit in `graph.ipynb`. |
| `src/risultati/fit/fit_run_14377.root`, `src/risultati/fit/fit_run_14379.root` | `calibrazione/src/risultati/fit/`, notebook della variante a 20 anelli. |
| `pos/analisi/output.root`, `output2.root`, `output3.root` | `geometria/analisi/`, visualizzazioni degli eventi. |
| `pos/test/output2.root`, `output3.root`, `output5.root`, `skim_14451.root` | `geometria/rifrazione/`, confronti del ToF e mappe geometriche. |
| `pos/prova/output.root` | `archivio/prototipi_tof/`, notebook delle prove precedenti. |
| `bias/output4.root`, `output5.root`, `output6.root`, `RUN14399_26B.root`, `RUN14417_26B.root`, `RUN14453_26B.root` | `bias/`, grafici di bias, risoluzione e spettro energetico. |
| `prova/output/output_*.root` | Input dell’estrazione NLL e della ricostruzione; passare il percorso agli eseguibili. Per le istruzioni originali del notebook, la posizione relativa è `ricostruzione/output/`. |
| `prova/pdf_table.bin` | Tabella binaria richiesta da `reco_approx.cpp` e `reco_corrected.cpp`; passare il percorso come argomento. Non è sostituibile direttamente con `pdf_parameters.txt`. |

I ROOT originali necessari a ripetere l’estrazione dai dati grezzi non sono tutti contenuti in `~/juno/`: alcuni programmi fanno riferimento allo storage della collaborazione. I file di risultato non sostituiscono questi input.

## Percorsi conservati nel codice

- `calibrazione/src/macro.cpp` cerca i file `tof_RUN_nLS1.597_nW1.33_N.root` nella directory corrente. Produce `output_RUN.root` nella stessa directory. `fit.cpp`, invece, li cerca in `output3/`: il passaggio dei file era già esplicito nella struttura originale.
- `calibrazione/src/conv.cpp` usa `../nll/risultati/delta_rel.txt` e scrive `../nll/risultati/delta_2.root`. La relazione tra le cartelle è stata mantenuta.
- `calibrazione/src/graph2.ipynb` legge ancora `Path.home() / "juno/src/fit_run_14383.root"`; contiene anche un percorso `Desktop/Tesi/output/fit_riferimento_17100`. Quest’ultimo è una variabile presente nel codice originale. La figura ACU viene esportata nella directory corrente.
- `bias/macro_positions.cc` legge i ROOT da un percorso assoluto `/storage/gpfs_data/...` incorporato nel sorgente; il primo argomento è una lista testuale di run e posizioni, non un ROOT, nonostante il messaggio di uso originale.
- `ricostruzione/macro2.cpp` ha un percorso di storage predefinito, ma accetta directory di input e output come argomenti: usarli per lavorare su una copia locale.
- `riferimenti/jvertex/` richiede classi e servizi del framework JUNO/SNiPER assenti dalla raccolta. È materiale di riferimento, non un programma autonomo.

## Problemi già presenti nei notebook

I numeri di cella qui riportati si riferiscono agli originali, prima dell’inserimento dei titoli Markdown.

- `src/graph.ipynb`, cella 4: contiene un output `TypeError` nel tentativo di disegnare una mappa con un sottoinsieme delle posizioni. La cella e l’errore sono conservati e identificati da un titolo.
- `src/risultati/graph.ipynb`, cella 4: contiene un `FileNotFoundError` per `run.dat`, assente anche nella cartella originale `src/risultati/`. Non è stata inserita una tabella diversa per aggirare l’errore.
- `bias/graphs.ipynb`, cella 13: l’import usa l’alias `pltù`, mentre le istruzioni successive usano `plt`. L’esecuzione dipende dall’import di `plt` nelle celle precedenti. Il refuso è stato mantenuto.
- Alcune celle riutilizzano variabili delle precedenti, ad esempio `mask`, `energy` e `data`: leggere il notebook in ordine; non è stata effettuata una revisione delle dipendenze tra tutte le celle.
- `geometria/rifrazione/graph2.ipynb` salva `pos_z2.txt` in una cella e l’ultima sezione compila un piccolo programma C++ temporaneo. Richiede quindi `g++`, oltre ai dati ROOT e alle librerie Python.

Gli output salvati non certificano che l’attuale sequenza completa sia rieseguibile. Le verifiche effettuate durante l’organizzazione sono descritte in [VERIFICA.md](VERIFICA.md).

## Documentazione precedente

In [originali/](originali/) sono conservate tre note scientifiche della ricostruzione. Riportano i nomi originali `~/juno/prova/` e comandi con wrapper Python, che non fanno parte di questa selezione. Per i nuovi percorsi e l’invocazione diretta dei C++ usare [COMPILAZIONE.md](COMPILAZIONE.md).

Le PDF della ricostruzione sono approssimazioni ricavate da una figura, come documentato negli originali; non sono le PDF ufficiali di JVertex. Le unità e le convenzioni di segno rimangono quelle dei rispettivi programmi.
