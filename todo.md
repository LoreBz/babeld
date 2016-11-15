# TO DO, estendere babel
## operativamente
#### creare una rete decente e controllabile ai fini di checkare consistenza calcolo centralità

**attualmente ho questo meccanismo
1) flushupdate va a pescare i contributors e aggrega i contributi a partire da una rotta! not bad
2) really_send_update manda tutto di là
3) di là arriva e il pacchetto è parsato correttamente**
###cosa manca allora?
**1) Implmentare la logica di onReceive(UPDATE), magari smanettando su parse_update_subtlv
2) mettere belle istruzioni di stampa quando si fa flush->cerca rotta & calcola contributi->manda update; ricevi e aggiorna contribuenti **
- definire la struttura lista di contribuenti e formato di un contribuente
- cambiare formato *xroute* e *route* di modo che abbiano entrambre una lista di contribuenti e un contatore di contributi
  - per xroute va fatto davvero???
- cambiare *find_xroute* e *find_installed_route* di modo che restituiscano rispettivamente una xroute o una route con il campo contributo correttamente calcolato
- modificare la firma e le chiamate a *really_send_update* includendo il contributo relativo al prefix per cui si manda l'update
- aggiungere (in gergo "accumulare") nel buffer di update il contributo; prestare attenzione al calcolo della lunghezza del pacchetto
- modificare il parsing degli update (*parse_packet*):
  - **come ci si assicura che in tutti gli update ci sia il campo NH? Come si fa a vedere se siamo noi il NH? Come si fa a mantenere questa info durante il parsing del SUBSEQUENT update tlv?**
  - **come si pesca l'id del vicino da cui abbiamo ricevuto il messaggio?**
  - le info sopra ci servono per mantenere la lista dei contribuenti; il prefix e il contributo da gestire ci arrivano come campi del messagio. !!!ricordarsi di aggiornare nel codice i puntatori ai vari field. :( aritmetica coi putantori SOB
  - implementare aggiornamento lista dei contribuenti
- *DA DISCUTERE LA validity DEI CONTRIBUTI*

### Logica
- Estendere le tabelle/strutture dati esistenti affinché si mantengano correttamente i flag “through me” e i “contributi”
  - Un elemento della lista contribuenti è un coppia <vref,con>. *vref* è il riferimento al vicino che sappiamo passa attraverso noi per raggiungere la rotta S; *con* è un intero ad indicare il contributo per S che *vref* stesso ci ha comunicato.  Con la lista dei contribuenti non è necessario mantenere i contributi dei vicini che non contribuiscono! Si risparmia memoria XD. Prima infatti si diceva di mantenere il contributo dei vicini per le varie destinazioni anche se essi non passavano attraverso di noi, ma per il calcolo della centralità non è necessario
  - Per calcolare c(S) bisogna scorrere tutta la lista dei contribuenti e questo può essere ineffeciente quando il numero di diversi vicini contribuenti è alto. Piuttosto si può decidere di mantenere un contatore per ogni entry S nella routing table. Quando si riceve un *{v,NH(S),c(S)}* bisogna a questo punto: Aggiornare la lista dei contribuenti (complex lineare nel numero di vicini perchè bisogna cercare il contributo precedente). + costo costante di aggiornare il contatore (==> si sottrae il vecchio contributo e si somma il nuovo, non si fa niente se il contributo è uguale al precedente, si elimina il contributo quando non viene rinnovato entro una scadenza! da definire tempo validità...)

- Cambiare il formato degli update e modificare l'invio degli stessi, affinchè trasportino le centrality info
  - per ora è stato abbastanza facile aggiungere un campo.
  - Studierei bene la logica di *flushupdates* che riesce a recuperare dalle varie tabelle tutte le info mancanti prima di fare *really_send_update*. Con le giuste modifiche dovrei essere in grado di fare qui il processing delle rotte di modo che si renda disponibile il c(S) e l'NH(S) da includere nell'update
- Aggiornare la routine che fa il parsing degli update di modo che gestisca correttamente il calcolo della centralità
  - se mi vengono i punti sopra allora non dovrebbe essere difficile, basta manipolare di conseguenza le strutture dati dopo aver correttamente letto un update esteso
