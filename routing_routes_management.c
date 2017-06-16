/**
 *  \file   routing_route_management.c
 *  \brief  Route Management Header File
 *  \author Dhafer Ben Arbia & Elyes Ben Hamida
 *  \date   March 2015
 **/
#include <stdio.h>
#include <kernel/modelutils.h>

#include "routing_common_types.h"
#include "routing_neighbors_management.h"
#include "routing_rreq_management.h"
#include "routing_routes_management.h"
#include <string.h>

#define V MAX_NEIGHBORS_SIZE

#define IN 99
#define N 30


typedef int bool;
#define true 1
#define false 0
#define false 0

/* #ORACENET# CROSSLAYER PARAMETER */
typedef int header_src_t;

/* STRREV Fct */  

    char *strrev(char *str)
    {
          char *p1, *p2;

          if (! str || ! *str)
                return str;
          for (p1 = str, p2 = str + strlen(str) - 1; p2 > p1; ++p1, --p2)
          {
                *p1 ^= *p2;
                *p2 ^= *p1;
                *p1 ^= *p2;
          }
          return str;
    }


/** \brief Function to forward received data packet towards the destination (Directed Diffusion)
 *  \fn void route_forward_advert_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 **/
void route_forward_interest_packet(call_t *to, call_t *from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  destination_t destination;
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  
  struct route *route = route_get_nexthop(to, header->dst);
  
  /* destroy data packet if no route towards the sink */
  if (route == NULL) {
#ifdef ROUTING_LOG_DATA_FORWARDING  
    printf("[ROUTING_LOG_ADVERT_FORWARDING] node %d => no route towards sink %d : advert packet destroyed\n", to->object, header->dst);
#endif
    packet_dealloc(packet);
    return;
  }
  
  destination.id = BROADCAST_ADDR;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  
  /* set MAC header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return;
  }

  /* send the data packet to the nexthop */ 
  TX(&to0, &from0, packet);
  
  /* update local stats */ 
  nodedata->tx_nbr[header->packet_type]++;

  /* update the global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
#ifdef ROUTING_LOG_DATA_FORWARDING  
  printf("[ROUTING_LOG_ADVERT_FORWARDING] node %d received an advertisement packet from source node %d => forwarding packet towards neighbors \n", to->object, header->src);
#endif
}



/** \brief Function to forward received data packet towards the destination (Directed Diffusion)
 *  \fn void route_forward_data_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 **/
void route_forward_data_packet(call_t *to, call_t *from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);

  destination_t destination;
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  
  struct route *route = route_get_nexthop(to, header->dst);
  
  /* destroy data packet if no route towards the sink */
  if (route == NULL) {
#ifdef ROUTING_LOG_DATA_FORWARDING  
    printf("[ROUTING_LOG_DATA_FORWARDING] node %d => no route towards sink %d : data packet destroyed\n", to->object, header->dst);
#endif
    packet_dealloc(packet);
    return;
  }
  
  header->nexthop = route->nexthop_id;
  header->prevhop = to->object;

  destination.id = route->nexthop_id;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  
  /* set MAC header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return;
  }

  /* send the data packet to the nexthop */ 
  TX(&to0, &from0, packet);

  /* update local stats */ 
  nodedata->tx_nbr[header->packet_type]++;

  /* update the global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
#ifdef ROUTING_LOG_DATA_FORWARDING  
  printf("[ROUTING_LOG_DATA_FORWARDING] node %d received a data packet from source node %d => forwarding packet towards sink %d through nexthop %d\n", to->object, header->src, header->dst, destination.id);
#endif
}


/** \brief Function to forward received data packet towards the destination (ORACENET)
 *  \fn void route_forward_data_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 **/
void route_forward_oracenet_data_packet(call_t *to, call_t *from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);

  destination_t destination;
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};

  printf("ggg Before get nexthop\n");
  struct route *route = route_get_nexthop_to_destination_oracenet(to, header->dst);
  header->nexthop = route->nexthop_id;
  /* Update the LQE Parameters */
  route_update_oracenet_prr(to, packet, header->nexthop);
  
  /* Update Value of the route->E2E_PRR */
  route->E2E_PRR = header->E2E_PRR;

  /* destroy data packet if no route towards the sink */
  if (route == NULL) {
#ifdef ROUTING_LOG_DATA_FORWARDING  
    printf("[ROUTING_LOG_DATA_FORWARDING] node %d => no route towards sink %d : data packet destroyed\n", to->object, header->dst);
#endif
    packet_dealloc(packet);
    return;
  }
  
  header->nexthop = route->nexthop_id;
  header->prevhop = to->object;
  destination.id = route->nexthop_id;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  
  

  /* set MAC header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return;
  }  

  /* send the data packet to the nexthop */ 
  TX(&to0, &from0, packet);
  
  /* update local stats */ 
  nodedata->tx_nbr[header->packet_type]++;

  /* update the global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8); 

}



/** \brief Function to forward received data packet towards the destination (AODV)
 *  \fn void route_forward_data_packet_to_destination(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 **/
void route_forward_data_packet_to_destination(call_t *to, call_t *from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  destination_t destination; 
  
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  
  struct route_aodv *route = route_get_nexthop_to_destination(to, header->end_dst);
  
  /* destroy data packet if no route towards the sink */
  if (route == NULL) {
#ifdef ROUTING_LOG_DATA_FORWARDING 
    printf("[ROUTING_LOG_DATA_FORWARDING] node %d => no route towards DST %d : data packet destroyed...\n", to->object, header->end_dst);
    route_aodv_show(to);
#endif
    packet_dealloc(packet);
    return;
  }
  
  destination.id = route->nexthop_id;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  
  /* set MAC header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return;
  }

  /* Update data packet header before forward */
  //header->hop = header->hop + 1;
  /* send the data packet to the nexthop */ 
  TX(&to0, &from0, packet);
  
  /* update the local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update the global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
#ifdef ROUTING_LOG_DATA_FORWARDING
  printf("[ROUTING_LOG_DATA_FORWARDING] node %d received a data packet from source node %d towards destination %d => forwarding packet to nexthop %d\n", to->object, header->src, header->end_dst, destination.id);
#endif
}


/** \brief Function to forward received RREP packet towards the destination (AODV)
 *  \fn int route_forward_rrep_packet(call_t *to, packet_t* packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_forward_rrep_packet(call_t *to, call_t *from, packet_t* packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  field_t *field_rrep_header = packet_retrieve_field(packet, "rrep_packet_header");
  struct rrep_packet_header* rrep_header = (struct rrep_packet_header*) field_getValue(field_rrep_header);
  
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
    
  struct route_aodv *route = route_get_nexthop_to_destination(to, rrep_header->dst);
  destination_t destination;

  /* destroy data packet if no route towards the sink */
  if (route == NULL) {
#ifdef ROUTING_LOG_RREP_PROPAGATION
    printf("[ROUTING_LOG_RREP_PROPAGATION] Time %lfs node %d => not route towards SRC node %d ! \n", get_time()*0.000000001, to->object, rrep_header->dst);
#endif
    packet_dealloc(packet);
    return -1;
  }
  
  destination.id = route->nexthop_id;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  
  /* set MAC header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* update basic packet header */
  header->src = to->object;
  header->type = nodedata->node_type;
  
#ifdef ROUTING_LOG_RREP_PROPAGATION
  printf("[ROUTING_LOG_RREP_PROPAGATION] Time %lfs node %d forwards RREP towards SRC node %d via nexthop node %d (hop_to_dst=%d) \n", get_time()*0.000000001, to->object, rrep_header->dst, route->nexthop_id, route->hop_to_dst);
#endif

  /* send the data packet to the nexthop */ 
  TX(&to0, &from0, packet);
  
  rrep_table_update(to, rrep_header->src, rrep_header->dst, -1, rrep_header->seq_rreq);

  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;

  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
  return 0;
}

  
/** \brief Function to update the local routing table according to received hello packets
 *  \fn int route_update_from_hello(call_t *to, struct packet_header *header, struct hello_packet_header *hello_header, double lqe)
 *  \param c is a pointer to the called entity
 *  \param header is a pointer to the network packet header
 *  \param hello_header is a pointer to the hello packet header
 *  \param lqe is the estimated link quality indicator
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_hello(call_t *to, struct packet_header *header, struct hello_packet_header *hello_header, double lqe) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route;
  int updated = 0;

  /* Update the routing table of sensors if nexthop node to a sink is available */
  if (nodedata->node_type == SENSOR_NODE && (header->type == SINK_NODE || hello_header->hop_to_sink > 0)) {
    
    /* check if a nexthop to the sink already exist and update related information */
    list_init_traverse(nodedata->routing_table);
    while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
      /* the sink is within reach communication */    
      if (route->sink_id == header->src) {
	route->nexthop_id = header->src;
	route->nexthop_lqe = lqe;
	route->hop_to_sink = 1;
	updated = 1;
	break;
      }
      /* the sink is reacheable through the sensor */
      else if (route->sink_id != header->src && route->sink_id == hello_header->sink_id) {
	/* select the neighbor as a nexthop if it is closest to the sink */
	if ( (hello_header->hop_to_sink + 1) < route->hop_to_sink) {
	  route->nexthop_id = header->src;
	  route->nexthop_lqe = lqe;
	  route->hop_to_sink = hello_header->hop_to_sink + 1;
	  route->time = get_time();
	}
	/* select the neighbor as a nexthop if it has a better LQE */
	else if (hello_header->hop_to_sink == route->hop_to_sink && lqe > route->nexthop_lqe) {
	  route->nexthop_id = header->src;
	  route->nexthop_lqe = lqe;
	  route->hop_to_sink = hello_header->hop_to_sink + 1;
	}
	updated = 1;
	break;
      }
    }

    /* Insert a new route to the sink if it does not exist in the routing table */ 
    if (updated == 0) {
      route = (struct route *) malloc(sizeof(struct route));
      route->sink_id = hello_header->sink_id;
      route->nexthop_id = header->src;
      route->nexthop_lqe = lqe;
      route->time = get_time();
      if (header->type == SINK_NODE) {
	route->hop_to_sink = 1;
      }
      else {
	route->hop_to_sink = hello_header->hop_to_sink + 1;
      }
      list_insert(nodedata->routing_table, (void *) route);
      /* set the path establishment time */
      if (nodedata->path_establishment_delay == -1) {
	nodedata->path_establishment_delay = get_time() * 0.000001;
	route_update_global_stats(to, nodedata->path_establishment_delay);
      }
#ifdef ROUTING_LOG_ROUTES
      printf("[ROUTING_LOG_ROUTES] From hello: Time %lfs at node %d: a new route has been added towards sink %d through sensor %d (%d hops) !\n", get_time()*0.000000001, to->object, route->sink_id, route->nexthop_id, route->hop_to_sink);
#endif

    }
  }

  return 0;
}


/* ## ORACENET ## */
/** \brief Function to update the local routing table according to received hello packets
 *  \fn int route_update_from_oracenet_hello_packet(call_t *to, struct packet_header *header, struct hello_packet_header *hello_header, double lqe)
 *  \param c is a pointer to the called entity
 *  \param header is a pointer to the network packet header
 *  \param hello_header is a pointer to the hello packet header
 *  \param lqe is the estimated link quality indicator
 *  \return 0 if success, -1 otherwise
 **/

int route_update_from_oracenet_hello_packet(call_t *to, struct packet_header *header, struct hello_packet_header *hello_header, double prr) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route;
  int updated = 0;


  if (hello_header->hop_to_sink > 0) {
    /* check if a nexthop to the sink already exist and update related information */
    list_init_traverse(nodedata->routing_table);
    while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
      /* the sink is within reach communication */    
      if ((route->sink_id == header->src) && (route->E2E_PRR <= prr)) {
	route->nexthop_id = header->src;
	route->E2E_PRR = prr;
	printf(" hops and PRR = %lf updated", prr); 
	route->hop_to_sink = 1;
	updated = 1;
	break;
      }
        

    /* Insert a new route to the sink if it does not exist in the routing table */ 
    if (updated == 0) {
      route = (struct route *) malloc(sizeof(struct route));
      route->sink_id = hello_header->sink_id;
      route->nexthop_id = header->src;
      route->E2E_PRR = prr;
      route->time = get_time();
      route->hop_to_sink = 1;
	printf(" hops and PRR = %lf updated", prr); 
      list_insert(nodedata->routing_table, (void *) route);
      /* set the path establishment time */
      if (nodedata->path_establishment_delay == -1) {
	nodedata->path_establishment_delay = get_time() * 0.000001;
	route_update_global_stats(to, nodedata->path_establishment_delay);
      }
     }
    }
#ifdef ROUTING_LOG_ROUTES
      printf("[ROUTING_LOG_ROUTES] From hello: Time %lfs at node %d: a new route has been added towards sink %d through sensor %d (%d hops and PRR : %lf) !\n", get_time()*0.000000001, to->object, route->sink_id, route->nexthop_id, route->hop_to_sink, route->E2E_PRR);
#endif
    
    
  }

  return 0;
}



/** \brief Function to update the local routing table according to received interest packets
 *  \fn  int route_update_from_interest(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_interest(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  field_t *field_interest_header = packet_retrieve_field(packet, "sink_interest_packet_header");
  struct sink_interest_packet_header* interest_header = (struct sink_interest_packet_header*) field_getValue(field_interest_header);
  
  struct route *route;
  int updated = 0;
  int i = 0;
  //printf("BEFORE route_update_from_interest \n");

  /* check if a nexthop to the sink already exist and update related information */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
    /* the sink is within reach communication */    
    if (route->dst == interest_header->sink_id && (route->hop_to_dst > nodedata->sink_interest_ttl - (interest_header->ttl - 1))) {
      route->nexthop_id = header->src;
      route->hop_to_dst = interest_header->ttl_max - (interest_header->ttl - 1);


      updated = 1;
#ifdef ROUTING_LOG_ROUTES
      printf("[ROUTING_LOG_ROUTES] From interest: Time %lfs at node %d: the route to sink %d has been updated (new nexthop: %d, %d hops) !\n", get_time()*0.000000001, to->object, route->dst, route->nexthop_id, route->hop_to_dst);
#endif
      break;
    }
    else if (route->dst == interest_header->sink_id && (route->hop_to_dst <= nodedata->sink_interest_ttl - (interest_header->ttl - 1))) {
      updated = 1;
      break;
    }
  }

  if (updated == 0) {
    route = (struct route *) malloc(sizeof(struct route));
    route->dst = interest_header->sink_id;
    route->nexthop_id = header->src;
    route->nexthop_lqe = 1.0;
    route->time = get_time();
    route->hop_to_dst = interest_header->ttl_max - (interest_header->ttl - 1);
    list_insert(nodedata->routing_table, (void *) route); 
    /* set the path establishment time */
    if (nodedata->path_establishment_delay == -1) {
      nodedata->path_establishment_delay = get_time() * 0.000001;
      route_update_global_stats(to, nodedata->path_establishment_delay);
    }

#ifdef ROUTING_LOG_ROUTES
    printf("[ROUTING_LOG_ROUTES] From interest: Time %lfs at node %d: a new route has been added towards sink %d through sensor %d (%d hops) !\n", get_time()*0.000000001, to->object, route->dst, route->nexthop_id, route->hop_to_dst);
#endif
  }

  //printf("AFTER route_update_from_interest \n");

  return 0;
}


/** \brief Function to update the local routing table according to received RREQ packets
 *  \fn  int route_update_from_rreq(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_rreq(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  field_t *field_rreq_header = packet_retrieve_field(packet, "rreq_packet_header");
  struct rreq_packet_header* rreq_header = (struct rreq_packet_header*) field_getValue(field_rreq_header);
  
  struct route_aodv *route;
  int updated = 0;

  if (to->object == rreq_header->src){
    return 0;
  }
  
  /* check if a nexthop to the dst already exist and update related information */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route_aodv *) list_traverse(nodedata->routing_table)) != NULL) {
  
    /* the dst is present in routing table */    
    if (route->dst == rreq_header->src && rreq_header->seq >= route->seq_rreq && (route->hop_to_dst > (rreq_header->ttl_max - (rreq_header->ttl - 1))) ) {
      route->nexthop_id = header->src;
      route->hop_to_dst = rreq_header->ttl_max - (rreq_header->ttl - 1);
      updated = 1;
#ifdef ROUTING_LOG_ROUTES
      //      printf("[ROUTING_LOG_ROUTES] From rreq: Time %lfs at node %d: the route to DST %d has been updated (new nexthop: %d, %d hops) !\n", get_time()*0.000000001, to->object, route->dst, route->nexthop_id, route->hop_to_dst);
#endif
      break;
    }
    else if (route->dst == rreq_header->src) {
      updated = 1;
      break;
    }
  }

  if (updated == 0) {
    route = (struct route_aodv *) malloc(sizeof(struct route_aodv));
    route->dst = rreq_header->src;
    route->nexthop_id = header->src;
    route->nexthop_lqe = 1.0;
    route->time = get_time();
    route->seq_rreq = rreq_header->seq;
    route->seq_rrep = -1;
    route->hop_to_dst = rreq_header->ttl_max - (rreq_header->ttl - 1);
    list_insert(nodedata->routing_table, (void *) route); 

    #ifdef ROUTING_LOG_ROUTES
    printf("[ROUTING_LOG_ROUTES] From rreq: Time %lfs at node %d: a new route has been added towards DST %d through sensor %d (%d hops) !\n", get_time()*0.000000001, to->object, route->dst, route->nexthop_id, route->hop_to_dst);
#endif

  }

  return 0;
}

/** \brief Function to update the local routing table according to received RREP packets
 *  \fn  int route_update_from_rrep(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_rrep(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  field_t *field_rrep_header = packet_retrieve_field(packet, "rrep_packet_header");
  struct rrep_packet_header* rrep_header = (struct rrep_packet_header*) field_getValue(field_rrep_header);
  
  struct route_aodv *route;
  int updated = 0;

  if (to->object == rrep_header->src){
    return 0;
  }
   
  /* check if a nexthop to the sink already exist and update related information */
  list_init_traverse(nodedata->routing_table);
  
  while((route = (struct route_aodv *) list_traverse(nodedata->routing_table)) != NULL) {
  
    /* the dst is present in routing table */    
    if (route->dst == rrep_header->src && rrep_header->seq >= route->seq_rreq && route->hop_to_dst > rrep_header->hop_to_dst ) {
      route->nexthop_id = header->src;
      route->hop_to_dst = rrep_header->hop_to_dst;
      updated = 1;
#ifdef ROUTING_LOG_ROUTES
      //      printf("[ROUTING_LOG_ROUTES] From rrep: Time %lfs at node %d: the route to DST %d has been updated (new nexthop: %d, %d hops) !\n", get_time()*0.000000001, to->object, route->dst, route->nexthop_id, route->hop_to_dst);
#endif
      break;
    }
    else if (route->dst == rrep_header->src){
      updated = 1;
      break;
    }
  }

  if (updated == 0) {
    route = (struct route_aodv *) malloc(sizeof(struct route_aodv));
    route->dst = rrep_header->src;
    route->nexthop_id = header->src;
    route->nexthop_lqe = 1.0;
    route->time = get_time();
    route->seq_rreq = rrep_header->seq;
    route->seq_rrep = -1;
    route->hop_to_dst = rrep_header->hop_to_dst;
    list_insert(nodedata->routing_table, (void *) route); 
    
#ifdef ROUTING_LOG_ROUTES
    printf("[ROUTING_LOG_ROUTES] From rrep: Time %lfs at node %d: a new route has been added towards DST %d through sensor %d (%d hops) !\n", get_time()*0.000000001, to->object, route->dst, route->nexthop_id, route->hop_to_dst);
#endif
  }

  return 0;
}


/** \brief Function to forward received data packet towards the destination (OLSRv2)
 *  \fn void route_forward_data_packet_to_destination(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 **/
void route_forward_olsrv2_data_packet_to_destination(call_t *to, call_t *from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  destination_t destination; 
  
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  

  //printf(" #### HERE IN ROUTE FORWARD \n");

  /* destroy data packet if no route towards the sink */
  if (header->olsr_path[header->olsr_path_index] == -1) {
#ifdef ROUTING_LOG_DATA_FORWARDING 
    printf("[ROUTING_LOG_DATA_FORWARDING] node %d => no route towards DST %d : data packet destroyed...\n", to->object, header->end_dst);
#endif
    packet_dealloc(packet);
    return;
  }
  
  //printf(" ##### %d", header->olsr_path[header->olsr_path_index]);
  
  destination.id = header->olsr_path[header->olsr_path_index];
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  
  /* set MAC header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return;
  }

  /* send the data packet to the nexthop */ 
  TX(&to0, &from0, packet);
  
  /* update the local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update the global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
#ifdef ROUTING_LOG_DATA_FORWARDING
  printf("[ROUTING_LOG_DATA_FORWARDING] node %d received a data packet from source node %d towards destination %d => forwarding packet to nexthop %d\n", to->object, header->src, header->end_dst, destination.id);
#endif
}



/** \brief Function to compute the nexthop towards the closest SINK node (Directed Diffusion)
 *  \fn   struct route* route_get_nexthop(call_t *to, int sink_id)
 *  \param c is a pointer to the called entity
 *  \param sink_id is the sink ID
 *  \return NULL if failure, otherwise the destination information of nexthop node
 **/
struct route* route_get_nexthop(call_t *to, int sink_id) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route = NULL;
  int hop_to_sink = 9999;
  struct route *nexthop = NULL;

  /* Check for the nexthop towards a particular sink */
  if (sink_id >= 0) {
    list_init_traverse(nodedata->routing_table);
    while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
      if (route->sink_id == sink_id) {
	return route;
      }
    }
  }
  /* Check the nexthop to the closest sink*/
  else {
    list_init_traverse(nodedata->routing_table);
    while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
      if (route->hop_to_sink < hop_to_sink) {
	nexthop = route;
	hop_to_sink = route->hop_to_sink;
      }
    }
    return nexthop;
  }

  return NULL;
}


struct route* oracenet_route_get_nexthop(call_t *to, int dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route = NULL;

  /* Check for the nexthop towards a particular destination */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
    if (route->dst == dst) {
      return route;
    }
  }

  return NULL;
}


 
/** \brief Function to compute the nexthop towards a given destination (AODV)
 *  \fn   struct route_aodv* route_get_nexthop_to_destination(call_t *to, int dst)
 *  \param c is a pointer to the called entity
 *  \param dst is the ID of the destination
 *  \return NULL if failure, otherwise the destination information of nexthop node
 **/
struct route_aodv* route_get_nexthop_to_destination(call_t *to, int dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route_aodv *route = NULL;

  /* Check for the nexthop towards a particular destination */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route_aodv *) list_traverse(nodedata->routing_table)) != NULL) {
    if (route->dst == dst) {
      return route;
    }
  }

  return NULL;
}

/** \brief Function to compute the nexthop towards a given destination (ORACENET)
 *  \fn   struct route* route_get_nexthop_to_destination_oracenet(call_t *to, int dst)
 *  \param c is a pointer to the called entity
 *  \param dst is the ID of the destination
 *  \return NULL if failure, otherwise the destination information of nexthop node
 **/
struct route* route_get_nexthop_to_destination_oracenet(call_t *to, int dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route = NULL;
  struct neighbor *neighbor = NULL;


  /* intermediate local variables */
  int routes_count;
  routes_count = 0;
  int route_nexthop_id_selected;
  double var_prr;			
  var_prr = 0;

  /* checking if we have many routes to the dst */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
    	if (route->dst == dst) {
    		routes_count++;
	}
  }
     

  /* If we have only one route to the dst, we return it */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
    	if ((route->dst == dst) && (routes_count == 1) && (route->E2E_PRR <= 1.000000)) {
	//printf("ONLY ONE Route to dst = %d with E2E_PRR= %lf is selected \n  ", route->dst, route->E2E_PRR);
    		return route;
	}else if ((route->dst == dst) && (routes_count == 1) && (route->E2E_PRR > 1.000000)){
		route->E2E_PRR = 1.000000;
		return route;
	}
  } 
 
  /* If we have many routes to the dst, we choose the route with Best E2E_PRR */

  /* Get the first Value of the Nexthop_LQE and ID */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
    if ((route->dst == dst) && (routes_count > 1) && (route->E2E_PRR <= 1)) {
        var_prr = route->E2E_PRR;
	route_nexthop_id_selected = route->nexthop_id;
	//printf("Route to dst = %d with E2E_PRR= %lf is selected \n  ", route->dst, route->E2E_PRR);
	break;
    }
  }
     
  /* Check if we have another route with better PRR */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
    if ((route->dst == dst) && (routes_count > 1)) {
		if ((route->E2E_PRR > var_prr) && (route->E2E_PRR <= 1)){
			route_nexthop_id_selected = route->nexthop_id;
			var_prr = route->E2E_PRR;			/* Here we found new route with better prr than the saved one */
		//	printf("Other Route to dst = %d with E2E_PRR= %lf is selected \n  ", route->dst, route->E2E_PRR);
		}	  
    }
  }
  /* We return the best route with the best E2E_PRR */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
    if ((route->dst == dst) && (route->nexthop_id == route_nexthop_id_selected)) {
	    return route;
    }
  }
  return NULL;
}


/** \brief Function to compute the nexthop towards a given destination (OLSRv2)
 *  \fn   struct route_olsrv2* route_get_nexthop_to_destination(call_t *to, int dst)
 *  \param c is a pointer to the called entity
 *  \param dst is the ID of the destination
 *  \return NULL if failure, otherwise the destination information of nexthop node
 **/
struct route_olsrv2* route_get_nexthop_to_destination_olsrv2(call_t *to, int dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route_olsrv2 *route = NULL;

  /* Check for the nexthop towards a particular destination */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route_olsrv2 *) list_traverse(nodedata->routing_table)) != NULL) {
    if (route->dst == dst) {
      //printf("#### %d \n", route->dst);
      return route;
    }
  }

  return NULL;
}


/** \brief Function to list the routing table on the standard output (AODV)
 *  \fn     void route_aodv_show(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void route_aodv_show(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route_aodv *route = NULL;
  
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route_aodv *) list_traverse(nodedata->routing_table)) != NULL) {
    printf("   => Route to dst=%d is nexthop=%d with hop_nbr=%d \n", route->dst, route->nexthop_id, route->hop_to_dst);
  }

}

/** \brief Function to list the routing table on the standard output (Directed Diffusion)
 *  \fn    void route_show(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void route_show(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route = NULL;

  printf("Routing table for node %d : \n ", to->object);
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
    printf("   => Route to dst=%d is nexthop=%d with hop_nbr=%d \n", route->dst, route->nexthop_id, route->hop_to_dst);
  }

}

/** \brief Function to list the routing table on the standard output (Directed Diffusion)
 *  \fn    void route_show(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void route_show_oracenet(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route = NULL;

  printf("Routing table for node %d : \n ", to->object);
  list_init_traverse(nodedata->routing_table);
 	 while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
		if (route->dst < MAX_NEIGHBORS_SIZE)
    		printf("   => Route to dst=%d is nexthop=%d with E2E_PRR=%lf \n", route->dst, route->nexthop_id, route->E2E_PRR);
  	 }
}

/** \brief Function to list the routing table on the standard output (Directed Diffusion)
 *  \fn    void route_show(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void route_remove_oracenet(call_t *to, int nexthop) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route = NULL;


  list_init_traverse(nodedata->routing_table);
 	 while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
		if ((route->nexthop_id == nexthop) && (route->dst < MAX_NEIGHBORS_SIZE)){
			list_delete(nodedata->routing_table, (void *) route);
		}
		
  	 }
}


/** \brief Function to update the global entity stats informations (global path establishment time, etc.)
 *  \fn     void route_update_global_stats(call_t *to)
 *  \param c is a pointer to the called entity
 *  \param delay is the path establishment time
 **/
void route_update_global_stats(call_t *to, double path_delay) {
  struct classdata *classdata = get_class_private_data(to);

  classdata->global_tx_control_packet = classdata->current_tx_control_packet;
  classdata->global_rx_control_packet = classdata->current_rx_control_packet;
  classdata->global_tx_control_packet_bytes = classdata->current_tx_control_packet_bytes;
  classdata->global_rx_control_packet_bytes = classdata->current_rx_control_packet_bytes ;
  
  if (path_delay > classdata->global_establishment_time) {
    classdata->global_establishment_time = path_delay;
  }
   
}


/** \brief Function to Calculate Shortest Path to the destination using Dijkstra algorithm (OLSRv2)
 *  \return the shortest route from the src to the 'i' given destination
 **/

// A utility function to find the vertex with minimum distance value, from
// the set of vertices not yet included in shortest path tree

int minDistance(int dist[], int sptSet[])
{
  // Initialize min value
  int min = INT_MAX, min_index;
  int v = 0;
  for (v = 0; v < MAX_NEIGHBORS_SIZE; v++)
    if (sptSet[v] == false && dist[v] <= min)
      min = dist[v], min_index = v;
 
  return min_index;
}


// A utility function to print the constructed distance array
int printSolution(int dist[], int n)
{
   int i = 0;
   printf("Vertex   Distance from Source\n");
   for (i = 0; i < MAX_NEIGHBORS_SIZE; i++)
      printf("%d \t\t %d\n", i, dist[i]);
}


void printPath(struct nodedata *nodedata, int dest, int prev[]){
   
  if (prev[dest] != -1){ //If we are not at the source node
    printPath(nodedata, prev[dest], prev);
  }
  nodedata->olsr_path[nodedata->olsr_path_index] = dest;
  nodedata->olsr_path_index++;
}


// Funtion that implements Dijkstra's single source shortest path algorithm
// for a graph represented using adjacency matrix representation
void dijkstra(call_t *to, int src, int dst)
{
  struct nodedata *nodedata = get_node_private_data(to);
  
  int dist[MAX_NEIGHBORS_SIZE];     // The output array.  dist[i] will hold the shortest
  // distance from src to i
  int prev[MAX_NEIGHBORS_SIZE];

  int sptSet[MAX_NEIGHBORS_SIZE]; // sptSet[i] will true if vertex i is included in shortest
  // path tree or shortest distance from src to i is finalized
  int i = 0;
  // Initialize all distances as INFINITE and stpSet[] as false
  for (i = 0; i < MAX_NEIGHBORS_SIZE; i++)
    dist[i] = INT_MAX, sptSet[i] = false, prev[i] = -1, nodedata->olsr_path[i] = -1;
 
  nodedata->olsr_path_index = 0;

  // Distance of source vertex from itself is always 0
  dist[src] = 0;
  int count = 0;
  int v = 0;
  // Find shortest path for all vertices
  for (count = 0; count < MAX_NEIGHBORS_SIZE-1; count++)
    {
      // Pick the minimum distance vertex from the set of vertices not
      // yet processed. u is always equal to src in first iteration.
      int u = minDistance(dist, sptSet);
 
      // Mark the picked vertex as processed
      sptSet[u] = true;
 
      // Update dist value of the adjacent vertices of the picked vertex.
      for (v = 0; v < MAX_NEIGHBORS_SIZE; v++)
 
	// Update dist[v] only if is not in sptSet, there is an edge from 
	// u to v, and total weight of path from src to  v through u is 
	// smaller than current value of dist[v]
	if (!sptSet[v] && nodedata->topology_matrix[u][v] && dist[u] != INT_MAX 
	    && dist[u]+nodedata->topology_matrix[u][v] < dist[v]) {
	  dist[v] = dist[u] + nodedata->topology_matrix[u][v];
	  prev[v] = u;
	}
    }
 
  // print the constructed distance array
  //printSolution(dist, MAX_NEIGHBORS_SIZE);

  // print detailed paths
  printPath(nodedata, dst, prev);

  int j = 0;
  if (dist[dst] == INT_MAX){
  	printf("[DIJKSTRA_LOG] Node %d - No route to destination %d - dist = %f\n", to->object, dst,dist[dst]);
	 for (i = 0; i < MAX_NEIGHBORS_SIZE; i++)
    		nodedata->olsr_path[i] = -1, nodedata->olsr_path_index = 0;
  }
  else {
  	printf("[DIJKSTRA_LOG] Node %d - Path from %d to %d = ", to->object, src, dst);
  	for (j = 0; j < nodedata->olsr_path_index; j++) {
    		printf("%d -> ", nodedata->olsr_path[j]);
  	}
  	printf("\n");
  }
}
 


/** \brief Function to forward received TC_PACKET packet towards the destination (OLSRv2)
 *  \fn int route_forward_tc_packet(call_t *to, packet_t* packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_forward_tc_packet(call_t *to, call_t *from, packet_t* packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  field_t *field_tc_header = packet_retrieve_field(packet, "tc_packet_header");
  struct tc_packet_header* tc_header = (struct tc_packet_header*) field_getValue(field_tc_header);
  int i = 0;

  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};

  //printf("Node %d broadcasts a received TC packet from src node %d with seq = %d \n", to->object, header->src, tc_header->seq);

  /* Add MPRs in the TC packet Header */
  for(i=0; i< MAX_NEIGHBORS_SIZE; i++){
	tc_header->mpr[i] = nodedata->MPR_set[i];		
  }

  /* send the data packet to the nexthop */ 
  TX(&to0, &from0, packet);
  
  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;

  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
  return 0;
}
 
/** \brief Function to update the local routing table according to received RREP packets
 *  \fn  int route_update_from_data(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_data_packet(call_t *to, packet_t *packet, int last_src) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);

 
  struct route *route;
  int updated = 0;

  if (to->object == header->src){
#ifdef ROUTING_LOG_ROUTES
    printf("[ROUTING_LOG_ROUTES_ORACENET] Packet received by the same sender [condition 1]\n"); 
#endif
    return 0;
  }
   
  /* check if a nexthop to the sink already exist and update related information */
  list_init_traverse(nodedata->routing_table);
  
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {


    /* the dst is present in routing table */    
    if (route->end_dst == header->src && route->hop_to_sink > header->hop_to_dst )  {
      route->nexthop_id = last_src;
     
      route->hop_to_dst = header->hop;
      updated = 1;
#ifdef ROUTING_LOG_ROUTES
      printf("[ROUTING_LOG_ROUTES] From DATA Packet: Time %lfs at node %d: the route to DST %d has been updated (new nexthop: %d, %d hops) !\n", get_time()*0.000000001, to->object, route->end_dst, route->nexthop_id, route->hop_to_dst);
#endif
      break;
    }
    else if (route->end_dst == header->src){
      updated = 1;
      break;
    }
  }

  if (updated == 0) {
    route = (struct route *) malloc(sizeof(struct route));
    route->end_dst = header->src;
    route->nexthop_id = last_src;
    route->nexthop_lqe = header->E2E_PRR;
    route->time = get_time();
    route->seq = header->seq;
    route->hop_to_dst = header->hop;
    list_insert(nodedata->routing_table, (void *) route); 
    
#ifdef ROUTING_LOG_ROUTES
      printf("[ROUTING_LOG_ROUTES] From DATA Packet: Time %lfs at node %d: the route to DST %d has been updated (new nexthop: %d, %d hops) !\n", get_time()*0.000000001, to->object, route->end_dst, route->nexthop_id, route->hop_to_dst);
#endif
  }

  return 0;
}

/** \brief Function to update the local routing table according to received interest packets
 *  \fn  int route_update_from_oracenet_data_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_oracenet_data_packet(call_t *to, packet_t *packet, int last_src) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  struct route *route;
  int updated = 0;

  if (to->object == header->src){
    return 0;
  }
   
  /* check if a nexthop to the sink already exist and update related information */
  list_init_traverse(nodedata->routing_table);
  
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
  
    /* the dst is present in routing table */    
    if (route->dst == header->src && route->hop_to_dst < MAX_NEIGHBORS_SIZE) { 
      route->nexthop_id = header->prevhop;
      route->hop_to_dst = header->hop;
      route->E2E_PRR = header->E2E_PRR;
      if (route->E2E_PRR > 1.000000) {
	route->E2E_PRR = 1.000000;
      }
      updated = 1;
#ifdef ROUTING_LOG_ROUTES
      printf("[ROUTING_LOG_ROUTES] From Data Packet: Time %lfs at node %d: the route to DST %d has been updated (new nexthop: %d, %d hops, PRR: %lf) !\n", get_time()*0.000000001, to->object, route->dst, route->nexthop_id, route->hop_to_dst, route->E2E_PRR);
#endif
      break;
    }
    else if (route->dst == header->src){
      updated = 1;
      break;
    }
  }

  if (updated == 0) {
    route = (struct route *) malloc(sizeof(struct route));
    route->dst = header->src;
    route->nexthop_id = header->prevhop;
    route->E2E_PRR = header->E2E_PRR;
    if (route->E2E_PRR > 1.000000) {
      route->E2E_PRR = 1.000000;
    }
    route->time = get_time();
    route->seq = header->seq;
    route->hop_to_dst = header->hop;
    list_insert(nodedata->routing_table, (void *) route); 
    
#ifdef ROUTING_LOG_ROUTES
    printf("[ROUTING_LOG_ROUTES] From Data Packet: Time %lfs at node %d: a new route has been added towards DST %d through sensor %d (%d hops) !\n", get_time()*0.000000001, to->object, route->dst, route->nexthop_id, route->hop_to_dst);
#endif
  }

  return 0;
}

/* #ORACENET# GET LQE PAREMETERS */

void route_get_oracenet_lqe(call_t *to, packet_t *packet, int nexthop) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  struct neighbor *neighbor;


  /* set LQE Parameters */
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {  /* find the right nexthop */
	if (neighbor->id == header->nexthop){
		
		header->E2E_PRR = neighbor->lqe;
			//printf("&& get lqe neighbor->id %d :  neighbor->lqe = %f\n",neighbor->id , neighbor->lqe);
		if ((header->E2E_PRR > 1) || (header->E2E_PRR = 0)){
			header->E2E_PRR = 1;
			header->E2E_ReTx = 0;
			//printf("&& get lqe header->E2E_PRR= %f and ReTx = %f",header->E2E_PRR, header->E2E_ReTx);
		}else{
			header->E2E_ReTx = 1/header->E2E_PRR;			
			//printf("&& get lqe header->E2E_PRR= %f and ReTx = %f",header->E2E_PRR, header->E2E_ReTx);
		}

	}
  }
  }
   
/* ON FORWARD -> UPDATE LQE FUNCTION */


void route_update_oracenet_prr(call_t *to, packet_t *packet, int nexthop) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  struct neighbor *neighbor;

  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {  /* find the right nexthop */
	
	if(neighbor->id == nexthop){
		if((neighbor->prr > 0.000000) && (neighbor->prr <= 1.000000)){
			header->E2E_PRR *= neighbor->prr;
		}
		break;
	}
  }
}


void route_update_oracenet_prr_from_adv(call_t *to, packet_t *packet, int prevhop) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  struct neighbor *neighbor;
  struct route *route;

  /* Update the header E2E_PRR from ADV */
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {  /* find the right nexthop */
	
	if(neighbor->id == prevhop){
		if((neighbor->prr > 0.000000) && (neighbor->prr <= 1.000000)){
			header->E2E_PRR *= neighbor->prr;
			break;			 
		}
		
	}
  }

    list_init_traverse(nodedata->routing_table);
  /* Route update with the new caclcluated E2E_PRR */
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
  
    /* the dst is present in routing table */    
    if ((route->dst == header->src) && (header->E2E_PRR > route->E2E_PRR)){
		route->E2E_PRR = header->E2E_PRR;

    }
  }
}


void route_update_oracenet_prr_from_hello(call_t *to, packet_t *packet, int prevhop) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  struct neighbor *neighbor;
  struct route *route;

  /* Update the header E2E_PRR from HELLO */
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {  /* find the right nexthop */
	
	if(neighbor->id == prevhop){
		if((neighbor->prr > 0.000000) && (neighbor->prr <= 1.000000)){
			header->E2E_PRR *= neighbor->prr;
			break;			 
		}
		
	}
  }

    list_init_traverse(nodedata->routing_table);
  /* Route update with the new caclcluated E2E_PRR */
  while((route = (struct route *) list_traverse(nodedata->routing_table)) != NULL) {
  
    /* the dst is present in routing table */    
    if ((route->dst == header->src) && (header->E2E_PRR >= route->E2E_PRR)){
		route->E2E_PRR = header->E2E_PRR;
		printf("New route updated from Hello packet with better PRR, DST = %d, PRR = %lf\n\n\n ", route->dst, route->E2E_PRR);

    }else if ((route->dst == header->src) && (header->E2E_PRR > 1.00000)){
	header->E2E_PRR = 1.000000;
	route->E2E_PRR = header->E2E_PRR;
     }
  }
}
