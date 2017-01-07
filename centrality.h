

struct contribute {
    struct neighbour *neigh;
    unsigned short contribute;
    struct contribute *next;
};

struct src_contribute {
  struct source *src;
  struct contribute *contributors;
  struct src_contribute *next;
};

unsigned short total_contribute(struct contribute *head);
struct contribute *update_contributors(struct contribute *head,
                        struct neighbour *neigh, unsigned short contribute);
unsigned short node_centrality();
struct contribute* remove_contribute(struct contribute *head, struct neighbour *neigh);
void printList(struct contribute *head);
unsigned short node_centrality_multiIP();
