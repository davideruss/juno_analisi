# JUNO — dal bias alla calibrazione temporale

Quattro passaggi, un notebook per parte. I notebook contengono **7 figure complessive** e si eseguono con le piccole tabelle incluse; i dati ROOT rimangono esterni.

| Parte | Obiettivo | Risultati |
|---|---|---|
| **0 · Bias iniziale** | Misurare bias e risoluzione lungo z. | [00_bias/risultati.ipynb](00_bias/risultati.ipynb) |
| **1 · Fermat** | Confrontare il minimo del tempo di volo con la traiettoria rettilinea in due mezzi. | [01_fermat/risultati.ipynb](01_fermat/risultati.ipynb) |
| **2 · Picchi** | Confrontare ciascun picco con il riferimento e costruire la correzione. | [02_picchi/risultati.ipynb](02_picchi/risultati.ipynb) |
| **3 · Likelihood** | Minimizzare la NLL per determinare gli spostamenti temporali e la mappa relativa. | [03_likelihood/risultati.ipynb](03_likelihood/risultati.ipynb) |

Ogni parte contiene `src/` per i C++, `dati/` per le tabelle e `risultati.ipynb`. Geometria PMT e lista delle run sono condivise in `dati_comuni/`.

## Aprire i notebook

Dalla radice del repository:

```bash
python3 -m venv .venv
.venv/bin/python -m pip install -r requirements-notebooks.txt
.venv/bin/jupyter lab
```

Aprire i notebook nell’ordine **0 → 1 → 2 → 3**, eseguendo ciascuno dalla propria cartella. I grafici salvati sono visibili anche su GitHub.

I **9 sorgenti/header C++** selezionati sono invariati. I notebook mostrano metodo e risultato, con le prove diagnostiche e i grafici ripetuti rimossi.

- [Compilazione e input dei programmi](docs/ESECUZIONE.md).
- [Provenienza dei dati, convenzioni e verifiche](docs/DATI.md).
