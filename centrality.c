#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <net/if.h>

#include "centrality.h"
#include "interface.h"
#include "neighbour.h"

/*
Aggregates contributes of all neighbour.
NB: each route entry has a contributors list, i.e. for each known prefix
 there is a list of neighbours that pass through this node in order to
 reach the prefix
*/
unsigned short total_contribute(struct contribute *head) {
	struct contribute *ptr = head;
	unsigned short total = 0;
	while(ptr != NULL) {
      total=total + ptr->contribute;
      ptr = ptr->next;
   }
   printf("CENTR; Total contribute: %i\n", total);
   return total;
}

struct contribute *update_contributors(struct contribute *head,
                        struct neighbour *neigh, unsigned short contribute) {
	struct contribute *ptr = head;
	int found = 0;
	while(ptr != NULL) {
      if (ptr->neigh == neigh) {
      	ptr->contribute = contribute;
      	printf("CENTR; Item for neigh on %s updated with value %i\n", neigh->ifp->name, contribute);
        //printf("CENTR; Updating contribute\n");
        found = 1;
      	break;
      }
      ptr = ptr->next;
   }
   if (!found) {
   	printf("CENTR; Adding new element <%s,%i>\n", neigh->ifp->name, contribute);
    //printf("CENTR; Adding element\n");
    struct contribute *link = (struct contribute*) malloc(sizeof(struct contribute));
   	link->neigh = neigh;
   	link->contribute = contribute;
   	link->next = head;
   	head = link;
   }
   return head;
}

void printList(struct contribute *head) {
   struct contribute *ptr = head;
   printf("[ ");
   while(ptr != NULL) {
      printf("(%s,%d) ",ptr->neigh->ifp->name,ptr->contribute);
      //printf("(%d) ",ptr->contribute);
      ptr = ptr->next;
   }
   printf(" ]\n");
}
