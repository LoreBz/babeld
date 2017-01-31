

struct contribute {
    struct neighbour *neigh;
    unsigned short contribute;
    struct contribute *next;
};

struct destination {
  unsigned char nodeid[8];
  unsigned short metric;
  unsigned char nexthop[16];
  struct contribute *contributors;
  struct destination *next;
};

/*struct src2contribute {
  unsigned char srcid[8];
  unsigned short interfaces_counter;
  unsigned short aggregate_contribute;
  struct src2contribute *next;
};*/
extern struct destination *destinations;

unsigned short total_contribute(struct contribute *head);
struct contribute *update_contributors(struct contribute *head,
                        struct neighbour *neigh, unsigned short contribute);
struct contribute* remove_contribute(struct contribute *head, struct neighbour *neigh);
struct destination* find_destination(unsigned char *nodeid);
void update_dest_table(unsigned char *nodeid);
void printList(struct contribute *head);
unsigned short node_centrality();
/*unsigned short node_centrality_multiIP();
void printInstalledRoutes();*/
