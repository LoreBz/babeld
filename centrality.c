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
      printf("(%s,%d) ",format_address(ptr->neigh->address),ptr->contribute);
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

struct contribute* remove_contribute(struct contribute *head, struct neighbour *neigh) {
   struct contribute* current = head;
   struct contribute* previous = NULL;
   if(head == NULL)
      return head;
   while(current->neigh != neigh) {
      //if it is last element
      if(current->next == NULL) {
         return head;
      } else {
         //store reference to current link
         previous = current;
         //move to next link
         current = current->next;
      }
   }
   //found a match, update the link
   printf("CENTR; Removing contribute<%s,%hu>\n",
          format_address(current->neigh->address), current->contribute);
   if(current == head) {
      //change first to point to next link
      head = head->next;
   } else {
      //bypass the current link
      previous->next = current->next;
   }
   free(current);//attenzione!!!
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

struct src_contribute* update_src_contributors(struct src_contribute *head,
                        struct source *src, struct contribute *clist) {
	struct src_contribute *ptr = head;
	int found = 0;
	while(ptr != NULL) {
      if (strcmp(format_eui64(ptr->src->id), format_eui64(src->id)) == 0) {
      	//other routes had same source_id, should merge contributes
        printf("MERGING CONTRIBUTES FOR ALREADY KNOWN SRC %s\n", format_eui64(src->id));
        found = 1;
      	break;
      }
      ptr = ptr->next;
   }
   if (!found) {
    //should add at the top of list by allocating, allocate new clist,
    //do not use pointer to route->contributors to avoid messing up...
    printf("NOT KNOWN SRC %s, ALLOCATING srcClist\n", format_eui64(src->id));
    struct src_contribute *fresh = (struct src_contribute*) malloc(sizeof(struct src_contribute));
    fresh->src=src;
    fresh->contributors=NULL;
    fresh->next = head;
    head = fresh;
    ptr = fresh;
  }
  //merge clist into fresh->contributors, iterative update_contributors should do the trick
  //if src was not found ptr is fresh, else is the ptr to item that should be updated
  struct contribute *citem = clist;

  while(citem!=NULL) {
    printf("SRC:%s, neigh:%s, contr:%hu\n", format_eui64(ptr->src->id), format_address(citem->neigh->address), citem->contribute);
    ptr->contributors=update_contributors(ptr->contributors,citem->neigh,citem->contribute);
    citem=citem->next;
  }
  return head;
}

unsigned short node_centrality_multiIP() {
  struct src_contribute *sclist=NULL;
  struct route_stream *routes;
  struct source *src;

  unsigned short total = 0;
  unsigned short contr = 0;

  printf("### Computing MULTI_IP_centrality for learned routes\n");
  routes = route_stream(ROUTE_INSTALLED);
  if(routes) {
      while(1) {
          struct babel_route *rt = route_stream_next(routes);
          if(rt == NULL) break;
          printf("RT:%s SRC:%s\n",format_prefix(rt->src->prefix,rt->src->plen), format_eui64(rt->src->id));
          src=(rt->src);
          sclist = update_src_contributors(sclist,src,rt->contributors);
      }
      route_stream_done(routes);
  }
  //now contributes should be merged per src in sclist, iterate over it to aggregate them
  while(sclist!=NULL) {
    contr = total_contribute(sclist->contributors);
    printf("\t\t<SRC:%s,contr:%hu>\n",
          format_prefix(sclist->src->prefix,sclist->src->plen), contr);
    total += contr;
    //potrei approfeittare per liberare memoria
    sclist=sclist->next;
  }
  printf("### MULTI_IP_Cent of this node=%hu\n",total);
  return total;
}
