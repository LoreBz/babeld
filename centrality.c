#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "babeld.h"
#include "kernel.h"
#include "util.h"
#include "neighbour.h"
#include "interface.h"
#include "route.h"
#include "xroute.h"
#include "source.h"
#include "centrality.h"


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
      total = total + (ptr->contribute);
      printf("\t<%s,%i> \n",
          format_address(ptr->neigh->address), ptr->contribute);
      ptr = ptr->next;
   }
//   printf("CENTR; Total contribute: %i\n", total);
   return total;
}

struct contribute *update_contributors(struct contribute *head,
                        struct neighbour *neigh, unsigned short contribute) {
	struct contribute *ptr = head;
	int found = 0;
	while(ptr != NULL) {
      if (ptr->neigh == neigh) {
      	ptr->contribute = contribute;
      	printf("CENTR; Item for neigh:%s updated with value %i\n",
        format_address(neigh->address), contribute);
        //printf("CENTR; Updating contribute\n");
        found = 1;
      	break;
      }
      ptr = ptr->next;
   }
   if (!found) {
   	printf("CENTR; Adding new element <%s,%i>\n",
    format_address(neigh->address), contribute);
    //printf("CENTR; Adding element\n");
    struct contribute *link = (struct contribute*) malloc(sizeof(struct contribute));
   	link->neigh = neigh;
   	link->contribute = contribute;
   	link->next = head;
   	head = link;
   }
   return head;
}

unsigned short node_centrality() {
  unsigned short total = 0;
  unsigned short contr = 0;

  //struct xroute_stream *xroutes;
  struct route_stream *routes;
  /*printf("### Computing centrality for xroutes\n");
  xroutes = xroute_stream();
  if(xroutes) {
      while(1) {
          struct xroute *xrt = xroute_stream_next(xroutes);
          if(xrt == NULL) break;
          contr = total_contribute(xrt->contributors);
          printf("\t\t<Xroute:%s,contr:%hu>\n",
                format_prefix(xrt->prefix,xrt->plen), contr);
          total += contr;
      }
      xroute_stream_done(xroutes);
  }*/

  printf("### Computing centrality for learned routes\n");
  routes = route_stream(ROUTE_INSTALLED);
  if(routes) {
      while(1) {
          struct babel_route *rt = route_stream_next(routes);
          if(rt == NULL) break;
          contr = total_contribute(rt->contributors);
          printf("\t\t<Route:%s,contr:%hu>\n",
                format_prefix(rt->src->prefix,rt->src->plen), contr);
          total += contr;
      }
      route_stream_done(routes);
  }
  printf("### Cent of this node=%hu\n",total);
  return total;
}
