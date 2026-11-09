# JUNO — calibrazione temporale e ricostruzione della posizione

Raccolta delle analisi C++ e dei notebook Jupyter provenienti da `~/juno/`, organizzata per argomento. Comprende **11 notebook** e **29 file sorgente o di compilazione**, con piccole tabelle di supporto. Il machine learning è escluso.

I programmi sono copie integrali degli originali. Nei notebook sono stati aggiunti titoli di sezione e rimosse soltanto due celle senza istruzioni eseguibili. I grafici e gli altri output già salvati sono conservati: non rappresentano una nuova esecuzione delle analisi.

## Da dove iniziare

| Argomento | Notebook | Contenuto |
|---|---|---|
| Mappa NLL | [Mappa e PDF](ricostruzione/mappa_nll/mappa_e_pdf.ipynb) | PDF temporali, correzione relativa e mappa simmetrizzata. Tutte le tabelle necessarie ai grafici sono incluse. |
| Calibrazione temporale | [Mappe e fit](calibrazione/src/graph.ipynb) | Confronto tra fit gaussiani e doppie ex-Gaussiane, qualità dei fit e mappe temporali. |
| Figure ACU | [Fit e posizioni](calibrazione/src/graph2.ipynb) | Componenti del fit di riferimento e posizioni della sorgente nel rivelatore. |
| Geometria e rifrazione | [Mappe del ToF](geometria/rifrazione/graph2.ipynb) | Correzione geometrica in funzione della posizione e dell’angolo. |
| Prestazioni | [Bias e risoluzione](bias/graphs.ipynb) | Distribuzioni delle posizioni e confronto con e senza correzione. |

## Cartelle

```text
calibrazione/
  src/                  Estrazione, fit temporali e notebook principali
    risultati/          Analisi precedente con 20 anelli
  nll/                  Variante C++ del fit di likelihood e relative mappe
geometria/
  preparazione/         Selezione dei PMT e della geometria
  calcolo/              Modelli geometrici del tempo di volo
  analisi/              Segnali sui PMT e confronto dei tempi
  rifrazione/           Mappe geometriche e verifiche del ToF
bias/                   Bias e risoluzione della posizione
ricostruzione/          Estrazione e ricostruzione C++, con e senza mappa
  mappa_nll/            Fit della mappa, PDF e notebook documentato
  pdf/                  Parametri delle PDF approssimate
  map_positive/         Piccola mappa di correzione usata dal secondo passaggio
riferimenti/jvertex/    Sorgenti di riferimento del framework JUNO
archivio/prototipi_tof/  Prime prove di confronto del ToF
docs/                   Provenienza, percorsi, istruzioni e verifica
```

La coppia `calibrazione/src/`–`calibrazione/nll/` conserva il percorso relativo usato da `conv.cpp`. Gli header `pmt.h` restano accanto ai rispettivi programmi perché le implementazioni differiscono.

## Consultazione ed esecuzione

I notebook si possono consultare su GitHub con i grafici già presenti. Per aprirli localmente, dalla radice del repository:

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements-notebooks.txt
.venv/bin/jupyter lab
```

Eseguire ciascun notebook con la sua cartella come directory di lavoro. La compilazione richiede `g++` e, per le analisi ROOT, ROOT con `root-config`; la ricostruzione richiede anche Minuit2. I sorgenti JVertex dipendono dal framework JUNO/SNiPER.

I dati ROOT e la tabella binaria delle PDF non sono inclusi. Alcuni sorgenti mantengono percorsi della macchina originale; due programmi C++ hanno errori sintattici preesistenti e due notebook contengono già output di errore. Questa prima organizzazione conserva tali condizioni: leggere [verifiche e problemi individuati](docs/VERIFICA.md) e [dati e percorsi](docs/DATI_E_PERCORSI.md) prima di rieseguire. Per i programmi principali sono disponibili [esempi di compilazione](docs/COMPILAZIONE.md).

## Provenienza e controlli

- [Selezione dei file e corrispondenza con gli originali](docs/SELEZIONE.md).
- [Manifest con hash SHA-256](docs/provenienza.json).
- [Esito dei controlli](docs/VERIFICA.md).

La cartella originale `~/juno/` è rimasta intatta. La raccolta contiene circa **19 MiB**; dati ROOT, eseguibili, ambienti virtuali e file generati sono esclusi tramite `.gitignore`.

## Preparazione per GitHub

Dalla radice del repository, dopo aver rivisto i file:

```bash
git add .
git diff --cached --stat
git commit -m "Organizza analisi JUNO C++ e notebook"
```

Poi collegare il repository GitHub desiderato con `git remote add origin URL_DEL_REPOSITORY` e pubblicare con `git push -u origin main`. Nessun commit o push viene eseguito dalla sola preparazione della raccolta.
