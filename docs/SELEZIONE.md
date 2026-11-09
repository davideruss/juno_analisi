# Selezione e provenienza
Importazione da `~/juno/` del 10 novembre 2026. Gli originali non sono stati modificati.
## Criteri
- Incluse le analisi C++ e tutti gli 11 notebook distinti individuati fuori dagli ambienti virtuali e dai checkpoint.
- Sorgenti e header copiati senza modifiche. Le diverse versioni di `pmt.h` restano nei rispettivi moduli.
- I notebook mantengono ordine, contenuto e output delle celle conservate. Sono stati aggiunti solo titoli Markdown.
- Conservate le tabelle necessarie ai grafici e alla geometria, nei percorsi attesi dalle analisi. Le copie della geometria sono intenzionali.
- I risultati con 20 anelli restano in `calibrazione/src/risultati/`: differiscono da quelli a 40 anelli e il notebook usa `../output_con_picchi.txt`.
- I prototipi di ToF precedenti restano in `archivio/prototipi_tof/`; `JVertex` è separato come riferimento per il software JUNO.
## Programmi e notebook
| Percorso originale in `~/juno/` | Copia nel repository |
|---|---|
| `pos/prova/graph.ipynb` | [archivio/prototipi_tof/graph.ipynb](../archivio/prototipi_tof/graph.ipynb) |
| `pos/prova/main.cpp` | [archivio/prototipi_tof/main.cpp](../archivio/prototipi_tof/main.cpp) |
| `pos/prova/pmt.h` | [archivio/prototipi_tof/pmt.h](../archivio/prototipi_tof/pmt.h) |
| `pos/prova/prog1.cpp` | [archivio/prototipi_tof/prog1.cpp](../archivio/prototipi_tof/prog1.cpp) |
| `pos/prova/prog2.cpp` | [archivio/prototipi_tof/prog2.cpp](../archivio/prototipi_tof/prog2.cpp) |
| `bias/graphs.ipynb` | [bias/graphs.ipynb](../bias/graphs.ipynb) |
| `bias/macro_positions.cc` | [bias/macro_positions.cc](../bias/macro_positions.cc) |
| `nll/graph.ipynb` | [calibrazione/nll/graph.ipynb](../calibrazione/nll/graph.ipynb) |
| `nll/test.cpp` | [calibrazione/nll/test.cpp](../calibrazione/nll/test.cpp) |
| `src/conv.cpp` | [calibrazione/src/conv.cpp](../calibrazione/src/conv.cpp) |
| `src/fit.cpp` | [calibrazione/src/fit.cpp](../calibrazione/src/fit.cpp) |
| `src/graph.ipynb` | [calibrazione/src/graph.ipynb](../calibrazione/src/graph.ipynb) |
| `src/graph2.ipynb` | [calibrazione/src/graph2.ipynb](../calibrazione/src/graph2.ipynb) |
| `src/macro.cpp` | [calibrazione/src/macro.cpp](../calibrazione/src/macro.cpp) |
| `src/pmt.h` | [calibrazione/src/pmt.h](../calibrazione/src/pmt.h) |
| `src/risultati/graph.ipynb` | [calibrazione/src/risultati/graph.ipynb](../calibrazione/src/risultati/graph.ipynb) |
| `pos/analisi/graph.ipynb` | [geometria/analisi/graph.ipynb](../geometria/analisi/graph.ipynb) |
| `pos/analisi/macro.cpp` | [geometria/analisi/macro.cpp](../geometria/analisi/macro.cpp) |
| `pos/analisi/macro2.cpp` | [geometria/analisi/macro2.cpp](../geometria/analisi/macro2.cpp) |
| `pos/analisi/main.cpp` | [geometria/analisi/main.cpp](../geometria/analisi/main.cpp) |
| `pos/analisi/pmt.h` | [geometria/analisi/pmt.h](../geometria/analisi/pmt.h) |
| `pos/calcolo/Main.cpp` | [geometria/calcolo/Main.cpp](../geometria/calcolo/Main.cpp) |
| `pos/calcolo/graphs.ipynb` | [geometria/calcolo/graphs.ipynb](../geometria/calcolo/graphs.ipynb) |
| `pos/calcolo/makefile` | [geometria/calcolo/makefile](../geometria/calcolo/makefile) |
| `pos/calcolo/pmt.h` | [geometria/calcolo/pmt.h](../geometria/calcolo/pmt.h) |
| `pmt/a.cpp` | [geometria/preparazione/a.cpp](../geometria/preparazione/a.cpp) |
| `pmt/test.cpp` | [geometria/preparazione/test.cpp](../geometria/preparazione/test.cpp) |
| `pos/test/graph.ipynb` | [geometria/rifrazione/graph.ipynb](../geometria/rifrazione/graph.ipynb) |
| `pos/test/graph2.ipynb` | [geometria/rifrazione/graph2.ipynb](../geometria/rifrazione/graph2.ipynb) |
| `pos/test/main.cpp` | [geometria/rifrazione/main.cpp](../geometria/rifrazione/main.cpp) |
| `pos/test/pmt.h` | [geometria/rifrazione/pmt.h](../geometria/rifrazione/pmt.h) |
| `prova/CorrectionMap.h` | [ricostruzione/CorrectionMap.h](../ricostruzione/CorrectionMap.h) |
| `prova/macro2.cpp` | [ricostruzione/macro2.cpp](../ricostruzione/macro2.cpp) |
| `prova/src/fit.cpp` | [ricostruzione/mappa_nll/fit.cpp](../ricostruzione/mappa_nll/fit.cpp) |
| `prova/src/mappa_e_pdf.ipynb` | [ricostruzione/mappa_nll/mappa_e_pdf.ipynb](../ricostruzione/mappa_nll/mappa_e_pdf.ipynb) |
| `prova/pmt.h` | [ricostruzione/pmt.h](../ricostruzione/pmt.h) |
| `prova/reco_approx.cpp` | [ricostruzione/reco_approx.cpp](../ricostruzione/reco_approx.cpp) |
| `prova/reco_corrected.cpp` | [ricostruzione/reco_corrected.cpp](../ricostruzione/reco_corrected.cpp) |
| `pos/test/JVertex.cc` | [riferimenti/jvertex/JVertex.cc](../riferimenti/jvertex/JVertex.cc) |
| `pos/test/JVertex.h` | [riferimenti/jvertex/JVertex.h](../riferimenti/jvertex/JVertex.h) |

## Pulizia dei notebook
- `nll/graph.ipynb`: rimossa la quinta cella, interamente commentata e priva di output.
- `pos/test/graph2.ipynb`: rimossa la quinta cella, vuota e priva di output.
- Nessuna cella eseguibile non vuota rimossa o riscritta. Nessun output di errore nascosto.

## Esclusioni
| Elementi originali | Motivo |
|---|---|
| `parche/` | Machine learning escluso su richiesta. |
| Script Python autonomi in `src/`, `pmt/`, `pos/test/` e `prova/` | Raccolta limitata a C++ e notebook. I wrapper Python non sono necessari per invocare direttamente gli eseguibili C++ selezionati; la tabella binaria PDF della ricostruzione rimane un input esterno. |
| `prova/backup_prima_velocizzazione/` | Backup di versioni precedenti. |
| `pos/prova/macro2.cpp` | Copia identica a `pos/analisi/macro2.cpp`, inclusa in `geometria/analisi/`. Non è un header né una dipendenza degli altri sorgenti. |
| Altri C++ ausiliari in `prova/` | Controlli e istogrammazione della pipeline Python; conservati i programmi principali di estrazione, fit e ricostruzione. |
| `*.root`, eseguibili, `pdf_table.bin`, modelli e archivi numerici voluminosi | Dati esterni e prodotti di elaborazione. |
| `.venv/`, `.git/`, cache, checkpoint e log | Stato locale e file rigenerabili. |
| Esportazioni PNG/PDF e risultati ripetuti per run | I grafici salvati rimangono nei notebook; evitate ulteriori copie degli stessi risultati. |

## Integrità
Il [manifest JSON](provenienza.json) elenca tutti i 93 file importati, con percorsi e hash SHA-256 prima e dopo la copia. Per i notebook registra anche le celle rimosse e il numero di celle Markdown aggiunte. I file di documentazione creati per questa raccolta non sono importazioni e non figurano nel manifest.

L’esito dei controlli è in [VERIFICA.md](VERIFICA.md).
