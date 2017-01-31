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

struct destination *destinations = NULL;


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
      /*printf("\t<%s,%i>\n",
          format_address(ptr->neigh->address), ptr->contribute);*/
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

struct destination* find_destination(unsigned char *nodeid) {
  if (destinations==NULL)
    return NULL;
  struct destination *ptr=destinations;
  while (ptr!=NULL) {
    if (memcmp(ptr->nodeid, nodeid, 8)==0) {
      return ptr;
    }
    ptr=ptr->next;
  }
  return NULL;
}

//look among routes with src->id==nodeid and get best route.
//copy src->metric and route->nexthop as fields of destination indexed by nodeid
void update_dest_table(unsigned char *nodeid) {
  unsigned short best_metric=INFINITY;
  unsigned char nexthop[16];
  //loop over installed routes
  struct route_stream *routes = route_stream(ROUTE_INSTALLED);
  if(routes) {
    struct source *current_src;
      while(1) {
          struct babel_route *rt = route_stream_next(routes);
          if(rt == NULL) break;
          current_src=rt->src;
          //if route's originator is nodeid...
          if(memcmp(current_src->id, nodeid,8)==0) {
            best_metric = MIN(current_src->metric, best_metric);
            //if this route offers best route to nodeid copy the nexthop
            if(best_metric==current_src->metric) {
              memcpy(nexthop,rt->nexthop,16);
            }
          }
      }
      route_stream_done(routes);
      //update destination with best_metric and preferred nexthop
      //flapping? better to use rt->smoothed_metric??
      struct destination *dest=find_destination(nodeid);
      if (dest) {
        dest->metric=best_metric;
        memcpy(dest->nexthop, nexthop, 16);
        return;
      } else {
        //should allocate destination and add it to the list
        struct destination *link =
          (struct destination*) malloc(sizeof(struct destination));
        memcpy(link->nodeid, nodeid, 8);
        link->metric = best_metric;
       	memcpy(link->nexthop, nexthop, 16);
        link->contributors=NULL;
       	link->next = destinations;
       	destinations = link;
        return;
      }
  }
}

unsigned short node_centrality() {
  struct destination *ptr=destinations;
  unsigned short tot=0;
  unsigned short part=0;
  printf("### Computing centrality\n");
  while(ptr!=NULL) {
    part=total_contribute(ptr->contributors);
    printf("DST %s->contr: %hu\n",format_eui64(ptr->nodeid),part);
    tot+=part;
    ptr=ptr->next;
  }
  printf("### Cent of this node=%hu\n",tot);
  return tot;
}
/*unsigned short node_centrality() {
  unsigned short total = 0;
  unsigned short contr = 0;

  //struct xroute_stream *xroutes;
  struct route_stream *routes;
  printf("### Computing centrality for xroutes\n");
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
  }

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
}*/

/*struct src2contribute* update_src_contributors(struct src2contribute *head,
                        struct source *src, unsigned short contr) {
	struct src2contribute *ptr = head;
	int found = 0;
	while(ptr != NULL) {
      if (memcmp(ptr->srcid, src->id, 8) == 0) {
      	//other routes had same source_id, should merge contributes
        printf("\tKNOWN SRC %s (part=%hu). Adding +%hu (tot=%hu)\n",
            format_eui64(ptr->srcid),ptr->aggregate_contribute,contr,
          ptr->aggregate_contribute + contr);
        ptr->interfaces_counter++;
        ptr->aggregate_contribute = ptr->aggregate_contribute + contr;
        found = 1;
      	break;
      }
      ptr = ptr->next;
   }
   if (!found) {
    //should add at the top of list by allocating new clist-item
    printf("\tNOT KNOWN SRC %s, new src-item startContr=%hu\n", format_eui64(src->id), contr);
    struct src2contribute *fresh =
          (struct src2contribute*) malloc(sizeof(struct src2contribute));
    memcpy(fresh->srcid, src->id, 8);
    fresh->interfaces_counter = 1;
    fresh->aggregate_contribute = contr;
    fresh->next = head;
    head = fresh;
  }
   return head;
}*/

/*unsigned short node_centrality_multiIP() {
  return node_centrality();
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
          sclist = update_src_contributors(sclist,rt->src,part);
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

}*/

/*void printInstalledRoutes() {
  struct route_stream *routes;
  routes = route_stream(ROUTE_INSTALLED);
  if(routes) {
      while(1) {
          struct babel_route *rt = route_stream_next(routes);
          if(rt == NULL) break;
          printf("SRC:%s\tRT:%s ",
        format_eui64(rt->src->id), format_prefix(rt->src->prefix, rt->src->plen));
        int i=0;
        for (i=0; i<16; i++) {
          printf("%u ", rt->src->prefix[i]);
        }
        printf("\n");
      }
      route_stream_done(routes);
  }
}*/
