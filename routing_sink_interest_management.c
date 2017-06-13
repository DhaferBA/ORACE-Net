/**
 *  \file   routing_sink_interest_management.c
 *  \brief  Directed Diffusion Interest Dissemination Management Source Code File
 *  \author Elyes Ben Hamida (QMIC)
 *  \date   March 2015
 **/
#include <stdio.h>
#include <kernel/modelutils.h>

#include "routing_common_types.h"
#include "routing_neighbors_management.h"
#include "routing_routes_management.h"
#include "routing_sink_interest_management.h"

/** \brief Callback function for the periodic dissemination of Interest packets in Directed Diffusion (to be used with the scheduler_add_callback function).
 *  \fn int sink_interest_propagation_callback(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int sink_interest_propagation_callback(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};

  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct sink_interest_packet_header), nodedata->interest_packet_real_size*8);
  
  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct sink_interest_packet_header *interest_header = malloc(sizeof(struct sink_interest_packet_header));
  field_t *field_interest_header = field_create(INT, sizeof(struct sink_interest_packet_header), interest_header);
  packet_add_field(packet, "sink_interest_packet_header", field_interest_header);

 
  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = BROADCAST_ADDR;
  header->type = nodedata->node_type;
  header->packet_type = SINK_INTEREST_PACKET;

  /* set basic packet header */
  interest_header->sink_id = to->object;
  interest_header->seq = nodedata->sink_interest_seq++;
  interest_header->ttl = nodedata->sink_interest_ttl;
  interest_header->ttl_max = nodedata->sink_interest_ttl;
  interest_header->data_type = nodedata->sink_interest_data_type;
  interest_header->position.x = get_node_position(to->object)->x;
  interest_header->position.y = get_node_position(to->object)->y;
  interest_header->position.z = get_node_position(to->object)->z;
  interest_header->time         = get_time();
  
  /* send sink interest packet */
  TX(&to0, &from0, packet);
  
  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
#ifdef LOCALG_LOG_INTEREST_PROPAGATION
  printf("[LOCALG_NET_IG] Time %lfs CC node %d propagate interest packet (seq=%d, data_type=%d, TTL=%d, DST=%d)\n", get_time()*0.000000001, to->object, interest_header->seq, interest_header->data_type, interest_header->ttl, destination.id);
#endif

  /* update nbr of transmitted sink interest packets */
  if (nodedata->sink_interest_nbr > 0) {
    nodedata->sink_interest_nbr--;
  }
  
  /* schedules next sink interest packet transmission */
  if (nodedata->sink_interest_nbr > 0 || nodedata->sink_interest_nbr == -1) {
    nodedata->previous_sink_interest_slot_time += nodedata->sink_interest_period;
    scheduler_add_callback(nodedata->previous_sink_interest_slot_time + get_random_double() * nodedata->sink_interest_period, to, from, sink_interest_propagation_callback, NULL);
  }

  return 0;
}

/** \brief Function to propagate received Interest packets (Directed Diffusion)
 *  \fn int sink_interest_propagation(call_t *to, void *args);
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int sink_interest_propagation(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  packet_t *packet = (packet_t *) args;
  
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  field_t *field_interest_header = packet_retrieve_field(packet, "sink_interest_packet_header");
  struct sink_interest_packet_header* interest_header = (struct sink_interest_packet_header*) field_getValue(field_interest_header);
  
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};

  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
  
  /* update TTL */
  if (interest_header->ttl > 0) {
    interest_header->ttl --;
  }

  /* drop sink interest packet if TTL is 0 */
  if (interest_header->ttl <= 0) {
#ifdef LOCALG_LOG_INTEREST_PROPAGATION
    printf("[LOCALG_NET_IG] Time %lfs Node %d drop the interest packet from SINK %d  seq=%d  (TTL=%d) => TTL!\n", get_time()*0.000000001, to->object, interest_header->sink_id, interest_header->seq, interest_header->ttl);
#endif
    packet_dealloc(packet);
    return 0;
  }

  /* drop the sink interest packet according to a given probability */
  if (get_random_double_range(0.0, 1.0) > nodedata->sink_interest_propagation_probability) {
#ifdef LOCALG_LOG_INTEREST_PROPAGATION
    printf("[LOCALG_NET_IG] Time %lfs Node %d drop the interest packet from SINK %d  seq=%d  (TTL=%d) => probability !\n", get_time()*0.000000001, to->object, interest_header->sink_id, interest_header->seq, interest_header->ttl);
#endif
    packet_dealloc(packet);   
    return 0;
  }

  /* broadcast packet only once */
  if (sink_interest_table_lookup(to, interest_header->sink_id, interest_header->data_type, interest_header->seq) == 0) {
    
    /* set mac header */
    if (SET_HEADER(&to0, to, packet, &destination) == -1) {
      packet_dealloc(packet);
      return 0;
    }
  
    /* update basic packet header */
    header->src = to->object;
    header->type = nodedata->node_type;

    /* send sink interest packet */
    TX(&to0, &from0, packet);
    
    /* update local stats */
    nodedata->tx_nbr[header->packet_type]++;
    
    /* update global stats */
    classdata->current_tx_control_packet ++;
    classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
#ifdef LOCALG_LOG_INTEREST_PROPAGATION
    printf("[LOCALG_NET_IG] Time %lfs Node %d broadcast interest packet from SINK %d  seq=%d  (TTL=%d) \n", get_time()*0.000000001, to->object, interest_header->sink_id, interest_header->seq, interest_header->ttl);
#endif

    sink_interest_table_update(to, interest_header->sink_id, interest_header->data_type, interest_header->seq);
  }
  /* Otherwise destroy the received interest packet*/
  else {
#ifdef LOCALG_LOG_INTEREST_PROPAGATION
    printf("[LOCALG_NET_IG] Time %lfs Node %d drop the interest packet from SINK %d  seq=%d  (TTL=%d) => already sent !\n", get_time()*0.000000001, to->object, interest_header->sink_id, interest_header->seq, interest_header->ttl);
#endif
    packet_dealloc(packet);
  }
  
  return 0;
}


/** \brief Function to check if Interest packet has already been sent (Directed Diffusion)
 *  \fn int sink_interest_table_lookup(call_t *to, int sink_id, int data_type, int seq);
 *  \param c is a pointer to the called entity
 *  \param sink_id is the sink ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of Interest
 *  \return 0 if success, -1 otherwise
 **/
int sink_interest_table_lookup(call_t *to, int sink_id, int data_type, int seq) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct interest *interest;

  list_init_traverse(nodedata->interest_table);
  while((interest = (struct interest *) list_traverse(nodedata->interest_table)) != NULL) {
    if (interest->sink_id == sink_id && interest->data_type == data_type && interest->seq >= seq) {
      return 1;
    }
  }
  
  return 0;
}


/** \brief Function to update the local Interest table to avoid the transmission of duplicate Interest (Directed Diffusion)
 *  \fn void sink_interest_table_update(call_t *to, int sink_id, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param sink_id is the sink ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of Interest
 **/
void sink_interest_table_update(call_t *to, int sink_id, int data_type, int seq) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct interest *interest;

  list_init_traverse(nodedata->interest_table);
  while((interest = (struct interest *) list_traverse(nodedata->interest_table)) != NULL) {
    if (interest->sink_id == sink_id && interest->data_type == data_type && interest->seq > seq) {
      interest->seq = seq;
      interest->time = get_time();
      return;
    }
  }

  interest = (struct interest *) malloc(sizeof(struct interest));
  interest->sink_id = sink_id;
  interest->data_type = data_type;
  interest->seq = seq;
  interest->time = get_time();
  list_insert(nodedata->interest_table, (void *)interest);
}



