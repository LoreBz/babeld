

struct contribute {
    struct neighbour *neigh;
    unsigned short contribute;
    struct contribute *next;
};

unsigned short total_contribute(struct contribute *contributors);
