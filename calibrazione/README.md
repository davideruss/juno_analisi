# Calibrazione temporale

- [src/graph.ipynb](src/graph.ipynb): analisi principale con mappe e fit dei tempi residui.
- [src/graph2.ipynb](src/graph2.ipynb): fit di riferimento e posizioni di calibrazione ACU.
- [nll/graph.ipynb](nll/graph.ipynb): mappe ottenute dal fit di likelihood in `nll/test.cpp`.
- [src/risultati/graph.ipynb](src/risultati/graph.ipynb): variante precedente con 20 anelli, conservata per confronto.

`src/macro.cpp` estrae i residui, `src/fit.cpp` effettua i fit e `src/conv.cpp` converte una tabella in un istogramma ROOT. Le cartelle `src` e `nll` mantengono il loro rapporto originale perché il convertitore usa `../nll/risultati/`.

La versione compatta del fit NLL accompagnata da un notebook già documentato è in [ricostruzione/mappa_nll/](../ricostruzione/mappa_nll/). Le diverse mappe non sono state unificate: hanno binning e procedure differenti.

Vedere [compilazione](../docs/COMPILAZIONE.md) e [dati esterni e problemi già presenti](../docs/DATI_E_PERCORSI.md).
