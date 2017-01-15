#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "babeld.h"
#include "kernel.h"
#include "util.h"
#include "neighbour.h"
#include "interface.h"
#include "route.h"
//#include "xroute.h"
#include "source.h"
#include "centrality.h"


void printList(struct contribute *head) {
   struct contribute *ptr = head;
   printf("[ ");
   while(ptr != NULL) {
      printf("(%s,%d) ",format_address(ptr->neigh->address),ptr->contribute);
      ptr = ptr->next;
   }
   printf(" ]\n");
}
/*
Aggregates contributes of all neighbour.
NB: each route entry has a contributors list, i.e. for each known prefix
 there is a list of neighbours that pass through this node in order to
 reach that prefix
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
   return total;
}

struct contribute *update_contributors(struct contribute *head,
                        struct neighbour *neigh, unsigned short contribute) {
	struct contribute *ptr = head;
	int found = 0;
	while(ptr != NULL) {
      if (ptr->neigh == neigh) {
      	ptr->contribute = contribute;
      	/*printf("CENTR; Item for neigh:%s updated with value %i\n",
        format_address(neigh->address), contribute);*/
        found = 1;
      	break;
      }
      ptr = ptr->next;
   }
   if (!found) {
   	/*printf("CENTR; Adding new element <%s,%i>\n",
    format_address(neigh->address), contribute);*/
    struct contribute *link =
      (struct contribute*) malloc(sizeof(struct contribute));
   	link->neigh = neigh;
   	link->contribute = contribute;
   	link->next = head;
   	head = link;
   }
   return head;
}

struct contribute* remove_contribute(struct contribute *head,
                        struct neighbour *neigh) {
   struct contribute* current = head;
   struct contribute* previous = NULL;
   if(head == NULL)
      return head;
   while(current->neigh != neigh) {
      if(current->next == NULL) {
         return head;
      } else {
         previous = current;
         current = current->next;
      }
   }
   //found a match, remove it!
   /*printf("CENTR; Removing contribute<%s,%hu>\n",
          format_address(current->neigh->address), current->contribute);*/
   if(current == head) {
      head = head->next;
   } else {
      previous->next = current->next;
   }
   free(current);
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

struct src2contribute* update_src_contributors(struct src2contribute *head,
                        unsigned char *srcid, unsigned short contr) {
	struct src2contribute *ptr = head;
	int found = 0;
	while(ptr != NULL) {
      if (memcmp(ptr->srcid, srcid, 8) == 0) {
      	//other routes had same source_id, should merge contributes
        printf("ALREADY KNOWN SRC %s (part=%hu). Now adding %hu\n",
            format_eui64(ptr->srcid),ptr->aggregate_contribute,contr);
        ptr->interfaces_counter++;
        ptr->aggregate_contribute = ptr->aggregate_contribute + contr;
        found = 1;
      	break;
      }
      ptr = ptr->next;
   }
   if (!found) {
    /*should add at the top of list by allocating, allocate new clist,
    do not use pointer to route->contributors to avoid messing up...*/
    printf("NOT KNOWN SRC %s, ALLOCATING srcClist\n", format_eui64(srcid));
    struct src2contribute *fresh =
          (struct src2contribute*) malloc(sizeof(struct src2contribute));
    memcpy(fresh->srcid, srcid, 8);
    fresh->interfaces_counter = 1;
    fresh->aggregate_contribute = contr;
    fresh->next = head;
    head = fresh;
  }
   return head;
}

unsigned short node_centrality_multiIP() {
  struct route_stream *routes;
  struct src2contribute *sclist=NULL;
  unsigned short part=0;
  printf("### Computing MULTI_IP_centrality for learned routes\n");
  routes = route_stream(ROUTE_INSTALLED);
  if(routes) {
      while(1) {
          struct babel_route *rt = route_stream_next(routes);
          if(rt == NULL) break;
          printf("SRC:%s RT:%s\n", format_eui64(rt->src->id),
              format_prefix(rt->src->prefix,rt->src->plen));
          part=total_contribute(rt->contributors);
          sclist = update_src_contributors(sclist,rt->src->id,part);
      }
      route_stream_done(routes);
  }
  //now aggregate all src contributes
  struct src2contribute *ptr = sclist;
  struct src2contribute *prev = NULL;
  unsigned short total = 0;
  unsigned short contr = 0;

  while(ptr!=NULL) {
    contr = (ptr->aggregate_contribute)/(ptr->interfaces_counter);
    printf("\t\t<SRC:%s,contr:%hu (=%hu/%hu)>\n", format_eui64(ptr->srcid), contr,
          ptr->aggregate_contribute, ptr->interfaces_counter);
    total += contr;
    //potrei approfittare per liberare memoria
    prev=ptr;
    ptr=ptr->next;
    free(prev);//prev no more needed, free memory ;)
  }
  printf("### MULTI_IP_Cent of this node=%hu\n",total);
  return total;

}
