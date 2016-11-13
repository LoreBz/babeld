# Schema invio UPDATE
## allora sto modificando le start_message e end_message per contare byte in più di payload
- ho trovato posti da modificare a linea 1561 1197 1200 di message.c 
## documentazione codice send_update

```c
void
send_update(struct interface *ifp, int urgent,
            const unsigned char *prefix, unsigned char plen,
            const unsigned char *src_prefix, unsigned char src_plen){}
```

- @param ifp: l'interfaccia su cui inviare l'update (se poi trova null allora usa l'interfaccia aux che penso sia la "boradcast")
- @param urgent: un intero che...penso sotto ad 1 voglia dire urgente, altrimenti indica quanti resend fare
- @param prefix and plen: la destinazione della rotta
- @param src_prefix and src_plen: non ho capito cosa sia..

- nel primo blocco di codice si controlla se ifp è NULL, in quel caso si richiama send_update con interfaccia ifp_aux e in più, se prefix non è null, sembra sia necessario fare un piccolo hack con find_installed_route... satisfy_request per poter fare in futuro flushupdates correttamente
- se l'interfaccia non è up si termina

```c
/* Full wildcard update with prefix == src_prefix == NULL,
   Standard wildcard update with prefix == NULL && src_prefix != NULL,
   Specific wildcard update with prefix != NULL && src_prefix == NULL. */
```
- caso standard di send_update è quello in cui sia prefix che src_prefix sono definiti; in quel caso si chiama buffer_update che metterà l'update nel buffer prima di inviarlo...
- se invece uno tra prefix e src_prefix non è specificato
  - si manda un full dump della routing table, mettendo nel buffer degli update relativi a tutte le rotte installate e già che c'è reimposta i timeout per l'interfaccia
- se invece entrambi prefix e src_prefix sono null allora penso che mandi update per la defualt route (zeroes) e...

```c
send_update(ifp, urgent, NULL, 0, zeroes, 0);
send_update(ifp, urgent, zeroes, 0, NULL, 0);
```
tutte le info degli update sono relative all'interfaccia (e al suo buffer). è interessante notare che la pianificazione del flush richiede solo un riferimento all'interfaccia e all'urgenza del flush

```c
schedule_update_flush(ifp, urgent);
```
questo metodo semplicemente imposta un timeout per inviare l'update sull'interfaccia
## devo trovare il posto in cui si reagisce alla scadenza di questo timeout!

```c
send_update(ifp, 0, NULL, 0, NULL, 0); //full dump, ad esempio fatto periodicamente nel main linea 760
send_update(neigh->ifp, 0, NULL, 0, zeroes, 0); //fatta in risposta a wildcard request
send_update(neigh->ifp, 0, prefix, plen, zeroes, 0); //in risposta a request con specifico prefix
send_update(neigh->ifp, 0, prefix, plen, src_prefix, src_plen); //in risposta a request con tutto specificato (la famosa seqno request??)
//ecc ci sono altre chiamate interessanti in route.c quando ad esempio prova a mandare un update per cambio di topologia su tutte le interfacce
```

## documentazione di buffer_update
- mi pare di capire che send_update è praticamente la funzione utilizzata per cominciare a preparare i pacchetti update, ma solo quando si invoca buffer_update si mette davvero roba nel buffer. Mentre flushupdate, che davvero manda gli update, può essere chiamata un po' ovunque quando è necessario svuotare un buffer (di solito a scadenza di un timer).
- pertanto lo considero il posto migliore per infilarci dentro le routine legate a centrality

```c
buffer_update(struct interface *ifp,
              const unsigned char *prefix, unsigned char plen,
              const unsigned char *src_prefix, unsigned char src_plen)
{
```

alloca spazio nel buffer dell'interfaccia per piazzarci l'update e carica le info dell'update correttamente nel buffer.
### vecchie considerazioni
- send_triggered_update in route.c è chiamata in consider_route o route_changed;
- a sua volta send_triggered_update può chiamare send_update_resend o send_update
- in send_update, a riga 1523 si chiama buffer_update(ifp, prefix, plen, src_prefix, src_plen);
  - in quella riga si gestisce il caso stdandard di voler mandare un update con prefix e src_prefix specificati
- buffer_update è sempre in message.c a linea 1452, mette nel buffer l'update appena costruito
- per poi mandare davvero ciò che è nel buffer bisogna invocare flushupdates!
- in flushupdates si va a vedere nel buffer di ogni interfaccia come siamo messi coi vari update e poi finalmente si chiama la funzione really_send_update
- nella really_send_update si usano le start_message accumulate_byte e end_message per mettere in qualche struttura "kernel accessible" i tlv necessari per mandarli definitivamente
### righe di codice esemplari
```c
debugf("Received request for (%s from %s) from %s on %s.\n",
       format_prefix(prefix, plen),
       format_prefix(src_prefix, src_plen),
       format_address(from), ifp->name);
send_update(neigh->ifp, 0, prefix, plen, src_prefix, src_plen);
```
quindi il primo parametro di una send_update e l'interfaccia di comunicazione sui cui mandare l'update
il secondo è un intero che indica l'urgenza (mi sembra di aver letto da qualche parte che >=1 vuol dire "resend" altrimenti è urgente)
il terzo e il quarto sono il prefix,plen destinazione dell'update
mentre src_prefix,src_plen dovrebbero essere la rete di appartenenza dell'originatore dell'update
