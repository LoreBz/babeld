# TO DO, estendere babel

- Estendere le tabelle/strutture dati esistenti affinché si mantengano correttamente i flag “through me” e i “contributi”
  - Estenderei la tabella dei vicini per farla diventare così
| Source | vicino | metrica | seqno | Next-hop | Flag | Lista contribuenti                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                               |
|--------|--------|---------|-------|----------|------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
|        |        |         |       |          |      | Un elemento della lista contribuenti è un coppia <vref,con>. vref è il riferimento al vicino che sappiamo passa attraverso noi per raggiungere la rotta Source*; con è un intero ad indicare il contributo per S* che vref stesso ci ha comunicato  Con la lista dei contribuenti non è necessario mantenere i contributi dei vicini che non contribuiscono! Si risparmia memoria XD. Prima infatti si diceva di mantenere il contributo dei vicini per le varie destinazioni anche se essi non passavano attraverso di noi, ma per il calcolo della centralità non è necessario |
- ancora meglio in questo senso è mantenere un solo contatore all'interno di ogni struct babel_route;
  - tale contatore (dichiarato "unsigned short contributes"), andrà gestito così. Quando arriva un'update con la  sua tripla <v,H(S),c(S)> allora se H(S)==me-> nella mia tabella di routing per la destinazione S vado a fare
  ME.RoutingTable[S].contributes+=c(S)


- Cambiare il formato degli update e modificare l'invio degli stessi, affinchè traportino le centrality info
  - non capisco quello attuale nè come si fa a crearne uno! sob, concettualmente per estenderlo però non dovrebbe essere tanto difficile
- Aggiornare la routine che fa il parsing degli update di modo che gestisca correttamente il calcolo della centralità
  - se mi vengono i punti sopra allora non dovrebbe essere difficile, basta manipolare di conseguenza le strutture dati dopo aver correttamente letto un update esteso
