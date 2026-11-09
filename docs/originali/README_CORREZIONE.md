# Secondo passaggio con mappa relativa simmetrizzata

Il primo passaggio è quello salvato in `results_reco_baseline`: nessuna mappa,
x,y,z,t0 liberi, due inizializzazioni (baricentro delle cariche Hamamatsu e centro).
Il secondo passaggio usa esattamente quegli eventi e quei vertici, senza doverli
ricostruire nuovamente. I dati originali e i risultati del primo passaggio restano invariati.

## Costruzione della mappa

Da `results_charge_t0/delta_relative.txt` e `run_z.txt`:

    delta(r,c) = [delta_relative(+r,+c) + delta_relative(-r,-c)] / 2

Si usa la media aritmetica, con uguale peso ai due lati, ai medesimi raggi e
centri degli anelli. Si conservano soltanto r>=0 e c>0. In particolare il punto
(17100,+0.1) corrisponde a (-17100,-0.1); il codice usa i centri reali dei bin.
A 17500 e 17700 mm non ci sono run opposte: si conserva il lato positivo senza
inventare misure mancanti. La riga misurata a r=0 è mediata tra anelli opposti.

Il grafico `map_positive/map_positive.png` mostra i valori misurati, con bianco
esattamente a zero. Sono presenti 21 raggi e 20 anelli. Non vengono applicati
tagli arbitrari in ampiezza o maschere TIR: i piccoli valori fuori dalla zona TIR
rimangono nella mappa. Non vengono eliminati neppure i valori negativi.
`metadata.json` elenca le run che contribuiscono a ogni riga.

## Applicazione a un vertice generico

Per ogni evento si fissa la posizione del primo passaggio r1:

    r = |r1|
    c_i = dot(r1, posizione_PMT_i) / (|r1| * |posizione_PMT_i|)
    delta_i = mappa(r, c_i)
    residuo_i = FHT_i - |posizione_PMT_i - vertice_fit| * 1.60/c - t0_fit - delta_i

L'angolo è quello polare del PMT rispetto alla direzione centro–vertice; non è
l'angolo del raggio che va dalla sorgente al PMT. La rotazione è realizzata con
il prodotto scalare, senza ruotare esplicitamente tutte le coordinate.
I delta vengono calcolati una volta sola e restano FISSI durante il secondo fit.
La posizione nominale serve solo per recuperare FHT dai residui salvati e per
valutare il risultato, mai per selezionare la mappa o inizializzare il fit.

Scelte ai bordi:

- delta=0 per c<=0; raccordo lineare da c=0 al primo centro c=0.025;
- interpolazione bilineare tra i raggi reali e i centri degli anelli;
- ultimo centro angolare c=0.975 mantenuto fino a c=1;
- delta(0,c)=0 per non introdurre un asse privilegiato al centro; interpolazione
  verso la prima riga a 1000 mm. La riga centrale MISURATA è comunque conservata
  nella tabella e nel grafico;
- r>17700 mm: ultima riga mantenuta, nessuna estrapolazione; flag dedicato.

t0 è nuovamente libero nel secondo fit. Si parte da x,y,z,t0 del primo passaggio
e si prova anche (0,0,0,0), conservando la NLL corretta più bassa. Questa seconda
inizializzazione è un controllo dei minimi locali, distinto dai due passaggi
senza/con correzione. Non garantisce di trovare sempre il minimo globale.
Se il PRIMO passaggio non è convergente, non si seleziona una mappa da quel
vertice: l'evento è riportato invariato e resta segnalato come fallito.

## Risultati e diagnostica

`results_reco_corrected/reco_RUN.root` contiene un evento per riga, i risultati
del fit e i campi `first_x/y/z/t0`, `first_success`, `correction_applied`,
`map_radius_mm`, `map_radius_clamped`, `delta_mean/min/max_ns` e
`nll_corrected_at_first`. Quest'ultima NLL usa già le correzioni ma valuta il
vertice/t0 del primo passaggio: è il confronto appropriato per verificare che
il secondo minimizzatore migliori il punto iniziale sotto la STESSA funzione.
La NLL senza mappa e quella con mappa sono funzioni differenti.

`comparison.png`, `comparison.csv` e `comparison.json` confrontano bias,
larghezze centrali (P84-P16)/2 ed errore spaziale R68 sugli STESSI eventi
convergenti in entrambi i passaggi. Il JSON conserva anche le statistiche su
TUTTI gli eventi; nessun fallimento viene eliminato dai ROOT/CSV.
`coverage.json` e `report.json` riportano conteggi e fallimenti.

Questa è una prova sugli stessi dati usati per stimare la mappa. Non è una
validazione indipendente del bias. L'estensione per rotazione assume simmetria
sferica e viene implementata anche fuori asse, ma i dati disponibili per questo
confronto hanno sorgenti sull'asse z. Le PDF rimangono quelle approssimate.

## Riproduzione

Nella cartella `/home/davide/juno/prova`:

```bash
bash build_corrected.sh
MPLCONFIGDIR=/tmp/juno-mpl ../.venv/bin/python make_symmetric_map.py --out map_positive
python3 export_positive_root.py map_positive
python3 run_corrected.py --out results_reco_corrected --workers 2 --resume
python3 export_corrected.py results_reco_corrected results_reco_baseline
MPLCONFIGDIR=/tmp/juno-mpl ../.venv/bin/python compare_corrected.py results_reco_corrected
```

Il resume controlla hash di mappa, eseguibile, PDF, geometria e ROOT del primo
passaggio, oltre a dimensione/data dei dati di input e degli output. Un cambio
di percorso dopo il calcolo invalida prudenzialmente il resume.
Per una prova breve: aggiungere `--max-events 100 --runs 14377 14417 14453` e
usare una cartella di output diversa.

## Verifiche

`check_correction.cpp` verifica nodi della tabella, interpolazione,
semisfera, origine, raggi oltre limite e rotazioni verso -z e fuori asse.
`make_symmetric_map.py` verifica anche la coppia +/-17100 mm.
`extract_check_hits.py` e `check_direct_nll.py` ricalcolano indipendentemente
delta e NLL dalle PDF analitiche per eventi delle tre run campione.
Gli esiti sono in `results_reco_corrected/checks/`.

Il controllo con mappa nulla su 300 eventi ha riprodotto le posizioni entro
1,77 mm, con differenze di NLL inferiori a 1,2e-5. Il ricalcolo diretto dalle
PDF su tre eventi ha dato differenze di NLL inferiori a 1,4e-8.

## Esito del calcolo completo (39 run, 293285 eventi)

- Correzione applicata ai 292663 eventi convergenti al primo passaggio;
  622 fallimenti precedenti conservati invariati.
- 292145 fit convergenti dopo la correzione (99,61%); 518 nuovi fallimenti.
- 29269 eventi usano l’ultima riga perché il primo raggio supera 17700 mm.
- Nessun fit corretto ha NLL superiore di oltre 0,001 alla NLL corretta
  valutata nella posizione/t0 del primo passaggio.
- Sugli stessi eventi convergenti, massimo |bias medio z|: 44,61 → 22,03 cm.
- Al centro la larghezza centrale z resta circa 28,1 cm.
- La larghezza z migliora in 26/39 run e peggiora nelle altre; l’errore
  spaziale R68 migliora in 38/39 run. Riduzione del bias e miglioramento della
  risoluzione non sono equivalenti.
- Il campione pilota riproduce esattamente i corrispondenti eventi nel calcolo
  completo (x,y,z,t0,NLL e diagnostica della correzione).
