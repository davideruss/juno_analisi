# Ricostruzione C++ e mappa NLL

| File o cartella | Funzione |
|---|---|
| `macro2.cpp`, `pmt.h` | Estrazione dei residui e delle informazioni evento/PMT. |
| [mappa_nll/](mappa_nll/) | Fit C++ della mappa e [notebook di PDF e correzioni](mappa_nll/mappa_e_pdf.ipynb), con tabelle incluse. |
| `reco_approx.cpp` | Prima ricostruzione libera di posizione e t0, senza mappa. |
| `reco_corrected.cpp`, `CorrectionMap.h` | Secondo passaggio con correzione scelta a partire dalla prima posizione ricostruita. |
| `map_positive/` | Mappa numerica utilizzabile nel secondo passaggio. |
| `pdf/` | Parametri delle PDF approssimate e relativa descrizione numerica. |

I C++ sono copie integrali di `~/juno/prova/`; il machine learning e i wrapper Python sono esclusi dalla selezione. La ricostruzione richiede ROOT/Minuit2, dati ROOT esterni e la tabella `~/juno/prova/pdf_table.bin`, non inclusa. Gli [esempi di compilazione e invocazione diretta](../docs/COMPILAZIONE.md) usano questi input esplicitamente.

Le note scientifiche originali sono conservate in [docs/originali/](../docs/originali/). I numeri e le valutazioni riportati in quelle note descrivono le elaborazioni precedenti, non nuovi test della raccolta organizzata.
