# TO DO

- supporto per IPv6...
- documentarsi su come networkx calcola la centralità
- implementare onReceive(SUBTLV_CENTRALITY)//caso in cui !troughME
  - per cui (invece che ignorare), si cerca se c'è un vecchio contributo da rimuovere

### NePa

1) preparare alcune topologie notevoli per cui è chiaro come calcolare la centralità nelle sue varie "accezzioni"
  - senza/con contare gli endpoint
  - con/senza i self-loop
  - ergo lanciare babel con centraility dump on per confrontare

2) preparare le stesse emulazioni ma con interruzioni schedulate sui nodi
