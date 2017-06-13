/**
 *  \file   routing_rreq_management.c
 *  \brief  AODV RREQ/RREP Management Source to0de File
 *  \author Elyes Ben Hamida (QMIC)
 *  \date   March 2015
 **/
#include <stdio.h>
#include <kernel/modelutils.h>

#include "routing_common_types.h"
#include "routing_neighbors_management.h"
#include "routing_rreq_management.h"
#include "routing_routes_management.h"


/** \brief Callback function for the RREQ packet dissemination in AODV (to be used with the scheduler_add_callback function).
 *  \fn int rreq_propagation_callback(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int rreq_propagation_callback(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
  destination_t *dst = (destination_t*) args;

  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rreq_packet_header), nodedata->rreq_packet_real_size*8);

  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct rreq_packet_header *rreq_header = malloc(sizeof(struct rreq_packet_header));
  field_t *field_rreq_header = field_create(INT, sizeof(struct rreq_packet_header), rreq_header);
  packet_add_field(packet, "rreq_packet_header", field_rreq_header);
  
  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = BROADCAST_ADDR;
  header->type = nodedata->node_type;
  header->packet_type = RREQ_PACKET;

  /* set basic packet header */
  rreq_header->dst = dst->id;
  rreq_header->src = to->object;
  rreq_header->seq = nodedata->rreq_seq++;
  rreq_header->ttl = nodedata->rreq_ttl;
  rreq_header->ttl_max = nodedata->rreq_ttl;
  rreq_header->data_type = nodedata->rreq_data_type;
  rreq_header->position.x = get_node_position(to->object)->x;
  rreq_header->position.y = get_node_position(to->object)->y;
  rreq_header->position.z = get_node_position(to->object)->z;
  
  /* send SRC node RREQ packet */
  TX(&to0, &from0, packet);
  
  /* if it is the first generated RREQ message => update the RREQ startup time variable */
  if (nodedata->first_rreq_startup_time == 0) {
    nodedata->first_rreq_startup_time = get_time();
  }
  
  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);

#ifdef ROUTING_RREQ_GENERATION
  printf("[ROUTING_RREQ_GENERATION] Time %lfs Node %d broadcasts RREQ packet (dst=%d, seq=%d, data_type=%d, TTL=%d)\n", get_time()*0.000000001, to->object, rreq_header->dst, rreq_header->seq, rreq_header->data_type, rreq_header->ttl);
#endif

  return 0;
}

/** \brief Callback function for the periodic generation of RREQ packets in AODV (to be used with the scheduler_add_callback function).
 *  \fn int rreq_periodic_generation_callback(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int rreq_periodic_generation_callback(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  
  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rreq_packet_header), nodedata->rreq_packet_real_size*8);

  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct rreq_packet_header *rreq_header = malloc(sizeof(struct rreq_packet_header));
  field_t *field_rreq_header = field_create(INT, sizeof(struct rreq_packet_header), rreq_header);
  packet_add_field(packet, "rreq_packet_header", field_rreq_header);
  
  struct route_aodv *route;

  /* broadcast a RREQ packet only if destination ID is known and a route to this destination does not exist */
  if (nodedata->sink_id != -1 && (route = route_get_nexthop_to_destination(to, nodedata->sink_id)) == NULL) {

    destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};

    /* set mac header */
    if (SET_HEADER(&to0, to, packet, &destination) == -1) {
      packet_dealloc(packet);
      return -1;
    }
      
    /* set basic packet header */
    header->src = to->object;
    header->dst = BROADCAST_ADDR;
    header->type = nodedata->node_type;
    header->packet_type = RREQ_PACKET;

    /* set basic packet header */
    rreq_header->dst = nodedata->sink_id;
    rreq_header->src = to->object;
    rreq_header->seq = nodedata->rreq_seq++;
    rreq_header->ttl = nodedata->rreq_ttl;
    rreq_header->ttl_max = nodedata->rreq_ttl;
    rreq_header->data_type = nodedata->rreq_data_type;
    rreq_header->position.x = get_node_position(to->object)->x;
    rreq_header->position.y = get_node_position(to->object)->y;
    rreq_header->position.z = get_node_position(to->object)->z;
      
    /* send SRC node RREQ packet */
    TX(&to0, &from0, packet);
      
    /* if it is the first generated RREQ message => update the RREQ startup time variable */
    if (nodedata->first_rreq_startup_time == 0) {
      nodedata->first_rreq_startup_time = get_time();
    }
      
    /* update local stats */
    nodedata->tx_nbr[header->packet_type]++;

    /* update global stats */
    classdata->current_tx_control_packet ++;
    classdata->current_tx_control_packet_bytes += (packet->real_size/8);
      
    #ifdef ROUTING_RREQ_GENERATION
    printf("[ROUTING_RREQ_GENERATION] Time %lfs Node %d broadcasts RREQ packet (dst=%d, seq=%d, data_type=%d, TTL=%d)\n", get_time()*0.000000001, to->object, rreq_header->dst, rreq_header->seq, rreq_header->data_type, rreq_header->ttl);
    #endif
  }
  else {
    packet_dealloc(packet);
    
    #ifdef ROUTING_RREQ_GENERATION
    printf("[ROUTING_RREQ_GENERATION] Time %lfs Node %d RREQ broadcast is canceled !\n", get_time()*0.000000001, to->object);
    #endif
  }

  /* update nbr of transmitted RREQ packets */
  if (nodedata->rreq_nbr > 0) {
    nodedata->rreq_nbr--;
  }
  
  /* schedules next RREQ generation */
  if (nodedata->rreq_nbr > 0 || nodedata->rreq_nbr == -1) {
    nodedata->previous_rreq_slot_time += nodedata->rreq_period;
	
	scheduler_add_callback(nodedata->previous_rreq_slot_time + get_random_double() * nodedata->rreq_period, to, from, rreq_periodic_generation_callback, NULL);
  }

  return 0;
}

/** \brief Function to forward RREQ  packet towards the destination (AODV)
 *  \fn int rreq_propagation(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int rreq_propagation(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  packet_t *packet = (packet_t *) args;

  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  field_t *field_rreq_header = packet_retrieve_field(packet, "rreq_packet_header");
  struct rreq_packet_header* rreq_header = (struct rreq_packet_header*) field_getValue(field_rreq_header);
  
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
  
  /* update TTL */
  if (rreq_header->ttl > 0) {
    rreq_header->ttl --;
  }

  /* drop RREQ packet if TTL is 0 */
  if (rreq_header->ttl <= 0) {
#ifdef ROUTING_RREQ_GENERATION
    printf("[ROUTING_RREQ_GENERATION] Time %lfs Node %d drop the RREQ packet from SOURCE %d  seq=%d  (TTL=%d) => TTL!\n", get_time()*0.000000001, to->object, rreq_header->src, rreq_header->seq, rreq_header->ttl);
#endif
    packet_dealloc(packet);
    return 0;
  }

  /* drop the SRC node RREQ packet acto0rding to a given probability */
  if (get_random_double_range(0.0, 1.0) > nodedata->rreq_propagation_probability) {
#ifdef ROUTING_RREQ_GENERATION
    printf("[ROUTING_RREQ_GENERATION] Time %lfs Node %d drop the RREQ packet from SRC %d  seq=%d  (TTL=%d) => probability !\n", get_time()*0.000000001, to->object, rreq_header->src, rreq_header->seq, rreq_header->ttl);
#endif
    packet_dealloc(packet);   
    return 0;
  }

  /* broadcast packet only once */
  if (rreq_table_lookup(to, rreq_header->src, rreq_header->dst, rreq_header->data_type, rreq_header->seq) == 0) {
    
    /* set mac header */
    if (SET_HEADER(&to0, to, packet, &destination) == -1) {
      packet_dealloc(packet);
      return 0;
    }
  
    /* update basic packet header */
    header->src = to->object;
    header->type = nodedata->node_type;

    /* send SRC node RREQ packet */
    TX(&to0, &from0, packet);
    
    /* update local stats */
    nodedata->tx_nbr[header->packet_type]++;

    /* update global stats */
    classdata->current_tx_control_packet ++;
    classdata->current_tx_control_packet_bytes += (packet->real_size/8);
      
#ifdef ROUTING_RREQ_GENERATION
    printf("[ROUTING_RREQ_GENERATION] Time %lfs Node %d rebroadcast RREQ packet from SRC %d to DST %d  seq=%d  (TTL=%d) \n", get_time()*0.000000001, to->object, rreq_header->src, rreq_header->dst, rreq_header->seq, rreq_header->ttl);
#endif

    rreq_table_update(to, rreq_header->src, rreq_header->dst, rreq_header->data_type, rreq_header->seq);
  }
  /* Otherwise destroy the received RREQ packet*/
  else {
#ifdef ROUTING_RREQ_GENERATION
    printf("[ROUTING_RREQ_GENERATION] Time %lfs Node %d drop the RREQ packet from SRC %d  towards DST %d  seq=%d  (TTL=%d) => already sent !\n", get_time()*0.000000001, to->object, rreq_header->src, rreq_header->dst, rreq_header->seq, rreq_header->ttl);
#endif
    packet_dealloc(packet);
  }
  
  return 0;
}


/** \brief Function to check if RREQ  packet has already been sent (AODV)
 *  \fn int rreq_table_lookup(call_t *to, int src, int dst, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param src is the source ID
 *  \param dst is the destination ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of RREQ
 *  \return 0 if success, -1 otherwise
 **/
int rreq_table_lookup(call_t *to, int src, int dst, int data_type, int seq) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct rreq *rreq;
  list_init_traverse(nodedata->rreq_table);
  while((rreq = (struct rreq *) list_traverse(nodedata->rreq_table)) != NULL) {
    if (rreq->src == src && rreq->dst == dst && rreq->data_type == data_type && rreq->seq >= seq) {
      return 1;
    }
  }
  return 0;
}


/** \brief Function to update the local RREQ table to avoid the transmission of duplicate RREQ (AODV)
 *  \fn void rreq_table_update(call_t *to, int src, int dst, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param src is the source ID
 *  \param dst is the destination ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of RREQ
 **/
void rreq_table_update(call_t *to, int src, int dst, int data_type, int seq) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct rreq *rreq;
  list_init_traverse(nodedata->rreq_table);
  while((rreq = (struct rreq *) list_traverse(nodedata->rreq_table)) != NULL) {
    if (rreq->src == src && rreq->dst == dst && rreq->data_type == data_type && rreq->seq > seq) {
      rreq->seq = seq;
      rreq->time = get_time();
      return;
    }
  }

  rreq = (struct rreq *) malloc(sizeof(struct rreq));
  rreq->dst = dst;
  rreq->src = src;
  rreq->data_type = data_type;
  rreq->seq = seq;
  rreq->time = get_time();
  list_insert(nodedata->rreq_table, (void *)rreq);
}


/** \brief Function for the transmission of RREP packet to the source node (AODV) => to be done by the sink node
 *  \fn int rrep_transmission(call_t *to, struct rreq_packet_header *rreq_header)
 *  \param c is a pointer to the called entity
 *  \param rreq_header is a pointer to the received RREQ packet
 *  \return 0 if success, -1 otherwise
 **/
int rrep_transmission(call_t *to, call_t *from, struct rreq_packet_header *rreq_header) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  array_t *down = get_class_bindings_down(to);
  
  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rrep_packet_header), nodedata->rrep_packet_real_size*8);
  
  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct rrep_packet_header *rrep_header = malloc(sizeof(struct rrep_packet_header));
  field_t *field_rrep_header = field_create(INT, sizeof(struct rrep_packet_header), rrep_header);
  packet_add_field(packet, "rrep_packet_header", field_rrep_header);
  
  struct route_aodv *route = route_get_nexthop_to_destination(to, rreq_header->src);
  
  /* check for the next hop towards the source node for sending the RREP packet */
  if (route == NULL) {
#ifdef ROUTING_RREQ_GENERATION
    printf("[ROUTING_RREQ_GENERATION] Time %lfs DST node %d : no route towards source node %d for sending the RREP \n", get_time()*0.000000001, to->object, rreq_header->src);
    route_aodv_show(to);
	route_show(to);
#endif
    return -1;
  }

  destination_t destination = {route->nexthop_id, {-1, -1, -1}};
  
  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = route->nexthop_id;
  header->type = nodedata->node_type;
  header->packet_type = RREP_PACKET;
  
  /* set RREP packet header */
  rrep_header->dst = rreq_header->src;                 
  rrep_header->src = to->object;                  
  rrep_header->seq = nodedata->rrep_seq;                  
  rrep_header->seq_rreq = rreq_header->seq;                  
  rrep_header->hop_to_dst = rreq_header->ttl_max - (rreq_header->ttl - 1); 
  rrep_header->position.x = get_node_position(to->object)->x;
  rrep_header->position.y = get_node_position(to->object)->y;
  rrep_header->position.z = get_node_position(to->object)->z;
  
  /* send sink interest packet */
  TX(&to0, &from0, packet);
  
#ifdef ROUTING_RREQ_GENERATION
  printf("[ROUTING_RREQ_GENERATION1] Time %lfs node %d sends a RREP packet:  src=%d, dst=%d, nexthop=%d, seq=%d, hop_to_dst=%d, interface=%d/%d \n", get_time()*0.000000001, to->object, rrep_header->src, rrep_header->dst, route->nexthop_id, rrep_header->seq, rrep_header->hop_to_dst, 0, down->size);
#endif

  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;

  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
  rrep_table_update(to, rrep_header->src, rrep_header->dst, -1, rrep_header->seq_rreq);
  
  nodedata->tx_nbr[header->packet_type]++;
  
  nodedata->rrep_seq++;
  
  return 0;
}


/** \brief Function for the transmission of RREP packet to the source node (AODV) => to be done by intermediate sensor node
 *  \fn int rrep_transmission_from_sensor(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the function argument
 *  \return 0 if success, -1 otherwise
 **/
int rrep_transmission_from_sensor(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  packet_t *packet_old = (packet_t *) args;
  
  field_t *field_rreq_packet_header = packet_retrieve_field(packet_old, "rreq_packet_header");
  struct rreq_packet_header* rreq_header = (struct rreq_packet_header*) field_getValue(field_rreq_packet_header);
  
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};

  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rrep_packet_header), nodedata->rrep_packet_real_size*8);
  
  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct rrep_packet_header *rrep_header = malloc(sizeof(struct rrep_packet_header));
  field_t *field_rrep_header = field_create(INT, sizeof(struct rrep_packet_header), rrep_header);
  packet_add_field(packet, "rrep_packet_header", field_rrep_header);
  
  struct route_aodv *route = route_get_nexthop_to_destination(to, rreq_header->src);
  struct route_aodv *route2 = route_get_nexthop_to_destination(to, rreq_header->dst);
 
  /* check for the next hop towards the source node for sending the RREP packet */
  if (route == NULL) {
#ifdef ROUTING_RREQ_GENERATION
    printf("[ROUTING_RREQ_GENERATION] Time %lfs SENSOR node %d : no route towards source node %d for sending the RREP \n", get_time()*0.000000001, to->object, rreq_header->src);
    route_aodv_show(to);
    packet_dealloc(packet_old);
#endif
    return -1;
  }

  destination_t destination = {route->nexthop_id, {-1, -1, -1}};
  
  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = route->nexthop_id;
  header->type = nodedata->node_type;
  header->packet_type = RREP_PACKET;
  
  /* set RREP packet header */
  rrep_header->dst = rreq_header->src;                 
  rrep_header->src = rreq_header->dst;                  
  rrep_header->seq = rreq_header->seq;                  
  rrep_header->seq_rreq = rreq_header->seq;                  
  rrep_header->hop_to_dst = route2->hop_to_dst+route->hop_to_dst; 
  rrep_header->position.x = get_node_position(to->object)->x;
  rrep_header->position.y = get_node_position(to->object)->y;
  rrep_header->position.z = get_node_position(to->object)->z;
  
  /* send sink interest packet */
  TX(&to0, &from0, packet);
  
  rrep_table_update(to, rrep_header->src, rrep_header->dst, -1, rrep_header->seq_rreq);
  
  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;

  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
  packet_dealloc(packet_old);

#ifdef ROUTING_RREQ_GENERATION
  printf("[ROUTING_RREQ_GENERATION2] Time %lfs SENSOR node %d sends a RREP packet to SRC node %d via node %d (seq=%d, hop_to_dst=%d) \n", get_time()*0.000000001, rrep_header->src, rrep_header->dst, route->nexthop_id, rrep_header->seq, rrep_header->hop_to_dst);
#endif
  
  return 0;
}


/** \brief Function to check if RREP packet has already been sent (AODV)
 *  \fn int rrep_table_lookup(call_t *to, int src, int dst, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param src is the source ID
 *  \param dst is the destination ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of RREQ
 *  \return 0 if success, -1 otherwise
 **/
int rrep_table_lookup(call_t *to, int src, int dst, int data_type, int seq) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct rrep *rrep;
  list_init_traverse(nodedata->rrep_table);
  while((rrep = (struct rrep *) list_traverse(nodedata->rrep_table)) != NULL) {
    if (rrep->src == src && rrep->dst == dst && rrep->data_type == data_type && rrep->seq >= seq) {
      return 1;
    }
  }
  return 0;
}


/** \brief Function to update the local RREP table to avoid the transmission of duplicate RREP (AODV)
 *  \fn void rrep_table_update(call_t *to, int src, int dst, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param src is the source ID
 *  \param dst is the destination ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of RREQ
 **/
void rrep_table_update(call_t *to, int src, int dst, int data_type, int seq) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct rrep *rrep;
  list_init_traverse(nodedata->rrep_table);
  while((rrep = (struct rrep *) list_traverse(nodedata->rrep_table)) != NULL) {
    if (rrep->src == src && rrep->dst == dst && rrep->data_type == data_type && rrep->seq > seq) {
      rrep->seq = seq;
      rrep->time = get_time();
      return;
    }
  }

  rrep = (struct rrep *) malloc(sizeof(struct rrep));
  rrep->dst = dst;
  rrep->src = src;
  rrep->data_type = data_type;
  rrep->seq = seq;
  rrep->time = get_time();
  list_insert(nodedata->rrep_table, (void *)rrep);
}

  


