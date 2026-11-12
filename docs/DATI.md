# Dati e convenzioni

## Provenienza

| Parte | Origine in `~/juno/` | Selezione presentata |
|---|---|---|
| 0 | `bias/macro_positions.cc`, `bias/output6.root` | Bias, sigma e relativi errori per 39 posizioni. |
| 1 | `pos/test/main.cpp`, `pos/test/pmt.h`, `pos/test/output5.root` | Media di Fermat − rettilineo in 50 × 89 celle, su tutti i PMT; esempio del PMT 740 a z = 16 m. |
| 2 | `src/macro.cpp`, `src/pmt.h`, `src/fit.cpp`, `src/fit_run_14383.root`, `src/delta_doubleexpgauss.txt` | Istogrammi e fit dell’anello 35 e del riferimento; mappa radiale dei picchi. |
| 3 | `prova/macro2.cpp`, `prova/pmt.h`, `prova/src/fit.cpp`, `prova/src/*.txt` | Mappa relativa e simmetrizzata. Il profilo NLL usa 97.082 hit dell’anello 35 di `prova/output/output_14383.root`, compressi in bin di 0,25 ns per classe di carica. |

Gli istogrammi e i parametri dei picchi non sono stati rifittati. Le tabelle estratte dai ROOT conservano i valori originali; il notebook della likelihood ripete la minimizzazione per l’anello illustrativo. Geometria e lista delle run sono condivise in `dati_comuni/`.

## Convenzioni essenziali

- **Fermat:** nLS = 1,54, nacqua = 1,33, raggio LS = 17,82 m; Δt = tFermat − tlineare. La mappa include i minimi al bordo dell’intervallo della scansione originale, non necessariamente stazionari.
- **Picchi:** ToF rettilineo con indice 1,59; riferimento angolare [−0,2, 0,2). Una correzione positiva si sottrae ai residui.
- **Likelihood:** indice 1,60; si minimizza sui residui FHT − ToF − t0, con t0 fissato. Il riferimento unisce gli hit in [−0,1, 0,1). Le PDF sono approssimate da una figura, non quelle ufficiali di JVertex.
- Le mappe radiali mostrano cos θ > 0. A 17,5 e 17,7 m manca il lato negativo. Al centro, la parte 2 conserva la riga originale; la parte 3 media le due semisfere come nel proprio C++.

Le parti 2 e 3 usano indici e riferimenti differenti. Le mappe da sole non misurano un miglioramento del bias: occorre una successiva ricostruzione su dati indipendenti.

## Verifica della sintesi

Verifica del 12 novembre 2026: i quattro notebook sono stati eseguiti da zero e le sette figure controllate. I sei programmi C++ compilano con ROOT 6.40.04; i nove sorgenti/header selezionati sono identici agli originali.

I calcoli del notebook e del C++ concordano negli esempi: tFermat = 41,000649 ns e minimo NLL a δ = 2,800 ns. Non è stata rieseguita l’intera elaborazione dei ROOT: le mappe conservano i risultati originali. I ROOT e gli originali in `~/juno/` rimangono esterni al repository.
