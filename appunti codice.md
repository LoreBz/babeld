# COMMENTI SUL CODICE BABEL

#### babeld.c:
- tutte le righe iniziali (sono alla 528 ora), sono dedicate al parsing dei parametri, dei file di configurazione, controllo dei paramateri di inizializzazione ecc. Non dvorei doverci mettere le mani.
- Si fa un uso criminale di goto ovunque
- le xroute sono le rotte esportate (da altri protocolli? da border router?)
- si pianifica il "neighbours check" ogni 5 sec, e l'"interface check" ogni 30
- alla linea 558 comincia il codice di "bootstrap", col commento dell'autore
*Make some noise so that others notice us, and send retractions in case we were restarted recently*
fondamentalmente si mandano hello su tutte le interfacce più un po' di messagi "ad cazzum", guardando bene immagino voglia farsi mandare il dump delle rotte dai vicini però boh
- a linea 583 si entra nel main loop (finisce a 788)
	- per ogni interfaccia si impostano i vari timer (hello, update, flush...)
	- poi si fa roba con kernel e socket che spero non mi serva mai capire davvero!
	- a linea 730 controlla se è l'ora di fare il "check neighbours", in caso affermativo ne pianifica uno fra poco
	- analogamente segue il controllo per decidere se fare il check_interface-
	- seguono nell'ordine i controlli dei vari timer:
	expiry_time [che innesca -> expire_routes() e expire_resend()]
	source_expiry_time [-> expire_sources()]
	- quindi per tutte le interfacce si controlla se è ora di mandare: Hello, update, o fare flush
	- altrime timer vegono controllati per fare "resend" (di pending request forse) e "flush_unicast" (??)
	- prima di uscire dal main loop si controlla se è il caso di fare un dump (si fa se debug è attivo o se è richiesto il dumping)
	- in qualche modo si riesce dentro al loop a controllare se è arrivato un segnale di arresto a babel (ad esempio c'è un controllo del tipo if(exiting) che breaka il main loop); per cui adesso spiego il codice fuori/dopo il main loop
- uscendo quindi occorre:
	- flushare tutte le rotte (linea 795)
	- *Make sure that we expire quickly from our neighbours' association 		caches.* -> send_hello_noupdate(ifp, 10);send_wildcard_retraction(ifp);
- poi si smollano tutte le connessioni a kernel, pidfile ecc e saluti! (siamo arrivati a linea 848)
- seguono le funzioni usate localmente come usage() o le label a cui si rimanda con i goto tipo FAIL e FAILPID


#### message.h
- vengono definiti i tipi dei messaggi con una serie di direttive *#define* che associano un codice int al tipo di messaggio
	- ad esempio sono definiti: (ack_req, ack, hello, ihu ecc..)
- vegono definite importantissimi funzioni/procedure
	- *parse_packet*
	- flush vari
	- send*qualsiasiTipoDiMessaggio*
	- di categoria a parte ci sono la *handle_request* *update_my_seqno*
	- interessante la distinzione tra send di request che non vanno inoltrare rispetto a quelle che possono essere inotrate a cui è aggiunto il prefisso **multihop**
	- le send definite qua dentro sono parecchie:
	```c
	void send_ack(struct neighbour *neigh, unsigned short nonce, unsigned short interval){...}

	void send_hello_noupdate(struct interface *ifp, unsigned interval)
	{...}

	void send_hello(struct interface *ifp){...}

	void send_update(struct interface *ifp, int urgent,
	            const unsigned char *prefix, unsigned char plen,
	            const unsigned char *src_prefix, unsigned char src_plen){...}

	void send_update_resend(struct interface *ifp,
	                   const unsigned char *prefix, unsigned char plen,
	                   const unsigned char *src_prefix, unsigned char src_plen){...}

	void send_wildcard_retraction(struct interface *ifp){...}
	```

#### message.c
- linea 325 inizia il vero parsing dei messaggi
- misteriosa funzione **DO_NTOHS** definita in util.h che serve per fare traduzioni in vari formati a quanto pare N to HS (cosa vorranno dire N e HS?)
- potrebbe essere interessante fare un flow chart delle azioni intraprese quando viene ricevuto un certo messaggio. Ad esempio dal codice si nota:
	- (348) MESSAGE_ACK_REQ -> send_ack(neigh, nonce, interval);
	- (360) MESSAGE_HELLO -> 	update_neighbour(neigh, seqno, interval);
		update_neighbour_metric(neigh, changed);
		schedule_neighbours_check(interval * 15, 0);
		ecc;
	- in generale si caspice più o meno per tutti i tipi di messaggi ricevuti quali sono le reazioni
	- dalla linea (1035) si possono vedere come sono implementate le varie send di messaggi, pertanto può essere utile per quando dovrò decidere di inviare i pacchetti per il protocollo centralità
	- 1305 flushupdates gestisce parecchio le rotte, può diventare importante da capire
	- come sarebbe interessante fare il flow chart delle azioni intraprese quando viene ricevuto un messaggio, parimenti potrebbe essere interessante schematizzare le azioni intraprese durante l'invio dei vari tipi di messaggio

#### net.h
- i metodi dichirati sono *babel_socket, babel_recv, babel_send, tcp_server_socket* per cui net è dedicato alla gestione degli stream di rete direi...

#### net.c
- l'implementazione dei metodi nel relativo .h fa uso solo di librerie standard e fondamentalmente gestisce buffer di ricezione/trasmissione e alza flag su interfacce ecc per segnalare eventi da usare in altre parti di codice con logica più alta

#### util.h
- di vagamente importante contiene le seguenti funzioni
	- roughly(val)->per prendere un valore similie aggiungedo un piccolo random
	-	tutte le funzioni di timeval comparison
	- un po' di formatter vari
	- anche i metodi per confrontare due seqno_plus
	- debugf e kdebugf

##### interface.h

- cotiene diverse strutture dati:
	- **buffered_update, interface_conf, interface**
	- merita di essere mostrata interface
```c
struct interface {
    struct interface *next;  
		/*	e' cosi' che si ottiene la "tabella" delle interfacce,
		in forma di linked list
		  */
    struct interface_conf *conf;
    unsigned int ifindex;
    unsigned short flags;
    unsigned short cost;
    int channel;
    struct timeval hello_timeout;
    struct timeval update_timeout;
    struct timeval flush_timeout;
    struct timeval update_flush_timeout;
    char name[IF_NAMESIZE];
    unsigned char *ipv4;
    int numll;
    unsigned char (*ll)[16];
    int buffered;
    int bufsize;
    /* Relative position of the Hello message in the send buffer, or
       (-1) if there is none. */
    int buffered_hello;
    char have_buffered_id;
    char have_buffered_nh;
    char have_buffered_prefix;
    unsigned char buffered_id[8];
    unsigned char buffered_nh[4];
    unsigned char buffered_prefix[16];
    unsigned char *sendbuf;
    struct buffered_update *buffered_updates;
    int num_buffered_updates;
    int update_bufsize;
    time_t bucket_time;
    unsigned int bucket;
    time_t last_update_time;
    time_t last_specific_update_time;
    unsigned short hello_seqno;
    unsigned hello_interval;
    unsigned update_interval;
    /* A higher value means we forget old RTT samples faster. Must be
       between 1 and 256, inclusive. */
    unsigned int rtt_decay;
    /* Parameters for computing the cost associated to RTT. */
    unsigned int rtt_min;
    unsigned int rtt_max;
    unsigned int max_rtt_penalty;
};
```
- ***ifp*** probabilmente sta per ***interface pointer***
- IF_CONF , FOR_ALL_INTERFACES , if_up() sono definite qui eh!

#### kernel.h
- una entry della routing table
```c
struct kernel_route {
    unsigned char prefix[16];
    int plen;
    unsigned char src_prefix[16];
    int src_plen; /* no source prefix <=> src_plen == 0 */
    int metric;
    unsigned int ifindex;
    int proto;
    unsigned char gw[16];
};
```
## kernel è un vaso di pandora, non ne posso più, codice ovunque, codice difficile, AAAAh

##### neighbour.h

```c
struct neighbour {
    struct neighbour *next;
    /* This is -1 when unknown, so don't make it unsigned */
    int hello_seqno;
    unsigned char address[16];
    unsigned short reach;
    unsigned short txcost;
    struct timeval hello_time;
    struct timeval ihu_time;
    unsigned short hello_interval; /* in centiseconds */
    unsigned short ihu_interval;   /* in centiseconds */
    /* Used for RTT estimation. */
    /* Absolute time (modulo 2^32) at which the Hello was sent,
       according to remote clock. */
    unsigned int hello_send_us;
    struct timeval hello_rtt_receive_time;
    unsigned int rtt;
    struct timeval rtt_time;
    struct interface *ifp;
};

extern struct neighbour *neighs;
```

- a linea (74) parte la procedura di creazione di un vicino
```c
struct neighbour *
find_neighbour(const unsigned char *address, struct interface *ifp)
{...}
```
```c
/* Recompute a neighbour's rxcost.  Return true if anything changed.
   This does not call local_notify_neighbour, see update_neighbour_metric. */
int
update_neighbour(struct neighbour *neigh, int hello, int hello_interval)
{
```
- a linea (230) la famosa
```c
unsigned check_neighbours(){
```
che fondamentalmente controlla se i vicini sono ancora raggiungibili, aggiorna la tabella dei vicini invocando `update_neighbour` su tutti loro, se vede che sono ormai irrangiugibili li flusha, aggiorna il txcost con `reset_txcost` e infine invoca `update_neighbour_metric`

### in neighbour.c si fa anche tutta la gestione dei costi, quindi è importante nel caso si voglia customizzare il costo in base alla centralità

#### route.h

```c
struct babel_route {
    struct source *src;
    unsigned short refmetric;
    unsigned short cost;
    unsigned short add_metric;
    unsigned short seqno;
    struct neighbour *neigh;
    unsigned char nexthop[16];
    time_t time;
    unsigned short hold_time;    /* in seconds */
    unsigned short smoothed_metric; /* for route selection */
    time_t smoothed_metric_time;
    short installed;
    unsigned char channels[DIVERSITY_HOPS];
    struct babel_route *next;
};
```
- concerne:
	- find / flush /

```c
int route_feasible(struct babel_route *route);
int route_old(struct babel_route *route);
int route_expired(struct babel_route *route);
int route_interferes(struct babel_route *route, struct interface *ifp);
int update_feasible(struct source *src,
                    unsigned short seqno, unsigned short refmetric);
```

**routing** ->
```c
struct babel_route *find_best_route(const unsigned char *prefix,
                                    unsigned char plen,
                                    const unsigned char *src_prefix,
                                    unsigned char src_plen,
                                    int feasible, struct neighbour *exclude);
struct babel_route *install_best_route(const unsigned char prefix[16],
                                 unsigned char plen);
```
**altre importantissime procedure per getire/mantenere la tabella di routing**
```c
void retract_neighbour_routes(struct neighbour *neigh);
void send_unfeasible_request(struct neighbour *neigh, int force,
                             unsigned short seqno, unsigned short metric,
                             struct source *src);

void send_triggered_update(struct babel_route *route,
                          	struct source *oldsrc, unsigned oldmetric);
void expire_routes(void);
```

## TABELLE COME SLOT
**sta storia degli slot nelle tabelle vale anche per sources ecc quindi meriterà un approfondimento**

#### source.h
- come da aspettarsi si trovano tutti i metodi per la gestione della source table
```c
struct source {
    unsigned char id[8];
    unsigned char prefix[16];
    unsigned char plen;
    unsigned char src_prefix[16];
    unsigned char src_plen;
    unsigned short seqno;
    unsigned short metric;
    unsigned short route_count;
    time_t time;
};
```
