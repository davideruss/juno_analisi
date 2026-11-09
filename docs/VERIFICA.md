# Verifica della raccolta

Controlli effettuati il 10 novembre 2026, dopo la copia e l’aggiunta dei titoli. Nessun programma scientifico originale è stato modificato per far superare i controlli.

## Integrità e notebook

- **93 file importati** verificati contro gli hash del [manifest](provenienza.json).
- **82 file copiati integralmente**, compresi tutti i 29 sorgenti/header/file di compilazione.
- **11 notebook** validati con `nbformat`; il codice Python delle celle conservate supera il controllo sintattico.
- **95 celle di codice conservate identiche**, inclusi metadata, conteggi di esecuzione e output. Anche le celle Markdown originali sono preservate.
- **83 celle Markdown aggiunte** per titoli, sezioni e note di provenienza.
- **2 celle rimosse**: una vuota, una interamente commentata e priva di output. Nessun output di errore eliminato.
- Gli hash dei file di origine corrispondono ancora a quelli registrati al momento della copia.

## Esecuzione di due notebook con i soli dati inclusi

Le celle sono state eseguite in ordine tramite Python, in copie temporanee delle rispettive cartelle, con Matplotlib in modalità non interattiva. Non sono stati riscritti i notebook o i risultati salvati nel repository e non sono stati ricalcolati i fit C++.

| Notebook | Celle di codice eseguite | Esito |
|---|---:|---|
| `ricostruzione/mappa_nll/mappa_e_pdf.ipynb` | 4 | Superato: lettura delle tabelle, controlli dimensionali e grafici. |
| `geometria/calcolo/graphs.ipynb` | 3 | Superato: lettura delle tabelle e grafici. |

La mappa NLL contiene 39 run e una mappa positiva di forma 21 × 20; le correzioni relative salvate vanno da circa −3,513 a +10,025 ns. È un controllo della lettura dei risultati esistenti, non una nuova validazione fisica della calibrazione. Gli altri notebook non sono stati eseguiti integralmente: richiedono dati ROOT esterni e alcuni contengono già errori salvati.

## C++

Controllo sintattico con `g++ -fsyntax-only`, C++17 e gli include restituiti da `root-config`. Ambiente: `g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0`, ROOT `6.40.04`. Non equivale a compilazione con linking né a un’esecuzione sui dati.

**17 programmi su 19 superano il controllo.** I due errori sono presenti nei sorgenti copiati integralmente:

| File | Problema originale |
|---|---|
| `archivio/prototipi_tof/prog1.cpp` | Riga 109: variabile `j` non dichiarata; riga 150: `get_type()` chiamato sul vettore `pmts`. |
| `geometria/analisi/macro2.cpp` | Riga 69: carattere `s` dopo `float out_rmstime;`. |

I sorgenti JVertex sono esclusi dal controllo autonomo perché richiedono header del framework JUNO/SNiPER. I loro contenuti sono comunque verificati con gli hash.

<details>
<summary>Esito per programma</summary>

| File | Controllo sintattico |
|---|---|
| `archivio/prototipi_tof/main.cpp` | Superato |
| `archivio/prototipi_tof/prog1.cpp` | Errore preesistente, vedere sopra |
| `archivio/prototipi_tof/prog2.cpp` | Superato |
| `bias/macro_positions.cc` | Superato |
| `calibrazione/nll/test.cpp` | Superato |
| `calibrazione/src/conv.cpp` | Superato |
| `calibrazione/src/fit.cpp` | Superato |
| `calibrazione/src/macro.cpp` | Superato |
| `geometria/analisi/macro.cpp` | Superato |
| `geometria/analisi/macro2.cpp` | Errore preesistente, vedere sopra |
| `geometria/analisi/main.cpp` | Superato |
| `geometria/calcolo/Main.cpp` | Superato |
| `geometria/preparazione/a.cpp` | Superato |
| `geometria/preparazione/test.cpp` | Superato |
| `geometria/rifrazione/main.cpp` | Superato |
| `ricostruzione/macro2.cpp` | Superato |
| `ricostruzione/mappa_nll/fit.cpp` | Superato |
| `ricostruzione/reco_approx.cpp` | Superato |
| `ricostruzione/reco_corrected.cpp` | Superato |

</details>

## Errori salvati nei notebook originali

Sono conservati un `TypeError` in `calibrazione/src/graph.ipynb` e un `FileNotFoundError` in `calibrazione/src/risultati/graph.ipynb`. Le celle interessate hanno ora un titolo esplicito. Dettagli e altri limiti di esecuzione sono in [DATI_E_PERCORSI.md](DATI_E_PERCORSI.md).

## Preparazione Git

Il repository è inizializzato sul ramo `main`, senza commit e senza remote. La configurazione `.gitignore` è stata verificata su ROOT, binari PDF, eseguibili, ambienti virtuali, checkpoint, log, file `.env` e risultati temporanei. Tutti i file destinati alla raccolta sono visibili a Git; nessuna tabella necessaria è esclusa per estensione. I collegamenti locali della nuova documentazione sono stati controllati.

La raccolta pesa circa 19 MiB; il file più grande è un notebook di circa 4,6 MiB. Nessun dato ROOT, eseguibile, ambiente virtuale o modello di machine learning è stato importato. Non è stato effettuato alcun push.
