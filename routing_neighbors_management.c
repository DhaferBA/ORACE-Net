/**
 *  \file   routing_neighbors_management.c
 *  \brief  Neighbor Management Source Code File
 *  \author Elyes Ben Hamida (QMIC)
 *  \date   March 2015
 **/
 
#include <stdio.h>
#include <kernel/modelutils.h>

#include "routing_common_types.h"
#include "routing_neighbors_management.h"
#include "routing_routes_management.h"

typedef int bool;
#define true 1
#define false 0

/** \brief Callback function for the periodic hello packet transmission in Directed Diffusion (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_directed_diffusion(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_discovery_callback_directed_diffusion(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  /* get a pointer to the lower-layers modules */
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
  
  /* create a hello packet */
  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct hello_packet_header), nodedata->hello_packet_real_size*8);
  
  /* extract the network and hello headers */
  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct hello_packet_header *hello_header = malloc(sizeof(struct hello_packet_header));
  field_t *field_hello_header = field_create(INT, sizeof(struct hello_packet_header), hello_header);
  packet_add_field(packet, "hello_packet_header", field_hello_header);
  
  /* compute the nexthop node ID for the specified destination */
  struct route *route = route_get_nexthop(to, -1);

  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = BROADCAST_ADDR;
  header->type = nodedata->node_type;
  header->packet_type = HELLO_PACKET;
  
  /* set hello packet header */
  if (nodedata->node_type == SINK_NODE) {
    hello_header->sink_id = to->object;
    hello_header->hop_to_sink = 0;  
  }
  else if (route != NULL) {
    hello_header->sink_id = route->sink_id;
    hello_header->hop_to_sink = route->hop_to_sink;
  }
  else  {
    hello_header->sink_id = -1;
    hello_header->hop_to_sink = -1;
  }
  hello_header->position.x = get_node_position(to->object)->x;
  hello_header->position.y = get_node_position(to->object)->y;
  hello_header->position.z = get_node_position(to->object)->z;
  
  /* send / push hello packet to the immediately lower-layer simulation module (e.g. MAC) */
  TX(&to0, &from0, packet);

  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update global stats */
  classdata->current_tx_control_packet++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);

#ifdef ROUTING_LOG_HELLO
  printf("[ROUTING_LOG_HELLO] Time %lfs node %d has sent a hello packet (%d %d) \n", get_time()*0.000000001, to->object, hello_header->sink_id, hello_header->hop_to_sink);
#endif

  /* update nbr of transmitted hello packets */
  if (nodedata->hello_nbr > 0) {
    nodedata->hello_nbr--;
  }
  
  /* schedules next hello packet transmission */
  if (nodedata->hello_nbr > 0 || nodedata->hello_nbr == -1) {
    nodedata->previous_hello_slot_time += nodedata->hello_period;
    scheduler_add_callback(nodedata->previous_hello_slot_time + get_random_double() * nodedata->hello_period, to, from,neighbor_discovery_callback_directed_diffusion, NULL);
  }
  return 0;
}


/** \brief Callback function for the periodic hello packet transmission in AODV (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_aodv(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_discovery_callback_aodv(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);

 
  /* get a pointer to the lower-layers modules */
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
 
  /* create a hello packet */
  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct hello_packet_header), nodedata->hello_packet_real_size*8);
 
  /* extract hello and network headers */
   struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct hello_packet_header *hello_header = malloc(sizeof(struct hello_packet_header));
  field_t *field_hello_header = field_create(INT, sizeof(struct hello_packet_header), hello_header);
  packet_add_field(packet, "hello_packet_header", field_hello_header);
  
  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = BROADCAST_ADDR;
  header->type = nodedata->node_type;
  header->packet_type = HELLO_PACKET;
  
  /* set hello packet header */
  if (nodedata->node_type == SINK_NODE) {
    hello_header->sink_id = to->object;
    hello_header->hop_to_sink = -1;  
  }
  else  {
    hello_header->sink_id = -1;
    hello_header->hop_to_sink = -1;
  }
  hello_header->position.x = get_node_position(to->object)->x;
  hello_header->position.y = get_node_position(to->object)->y;
  hello_header->position.z = get_node_position(to->object)->z;
  
  /* send / push hello packet to the immediately lower-layer simulation module (e.g. MAC) */
  TX(&to0, &from0, packet);

  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
#ifdef ROUTING_LOG_HELLO
  printf("[ROUTING_LOG_HELLO] Time %lfs node %d has sent a hello packet (%d %d) \n", get_time()*0.000000001, to->object, hello_header->sink_id, hello_header->hop_to_sink);
#endif

  /* update nbr of transmitted hello packets */
  if (nodedata->hello_nbr > 0) {
    nodedata->hello_nbr--;
  }
  
  /* schedules next hello packet transmission */
  if (nodedata->hello_nbr > 0 || nodedata->hello_nbr == -1) {
    nodedata->previous_hello_slot_time += nodedata->hello_period;
    scheduler_add_callback(nodedata->previous_hello_slot_time + get_random_double() * nodedata->hello_period, to, from, neighbor_discovery_callback_aodv, NULL);
  }
  return 0;
}

/*## ORACENET Discovery Callback*/

/** \brief Callback function for the periodic hello packet transmission in AODV (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_aodv(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_discovery_callback_oracenet(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
 
 /* Variables needed to consider the delayed Hello in the PRR calculations */
  uint64_t hello_slot_time = get_time() + nodedata->hello_start;
  uint64_t hello_tx_time = hello_slot_time + get_random_double() * nodedata->hello_period;
 
  /* get a pointer to the lower-layers modules */
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
 
  /* create a hello packet */
  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct hello_packet_header), nodedata->hello_packet_real_size*8);
 
  /* extract hello and network headers */
   struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct hello_packet_header *hello_header = malloc(sizeof(struct hello_packet_header));
  field_t *field_hello_header = field_create(INT, sizeof(struct hello_packet_header), hello_header);
  packet_add_field(packet, "hello_packet_header", field_hello_header);
  
  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = BROADCAST_ADDR;
  header->type = nodedata->node_type;
  header->packet_type = HELLO_PACKET;
  
  /* set hello packet header */
  if (nodedata->node_type == SINK_NODE) {
    hello_header->sink_id = to->object;
    hello_header->hop_to_sink = -1;  
  }
  else  {
    hello_header->sink_id = -1;
    hello_header->hop_to_sink = -1;
  }
  hello_header->position.x = get_node_position(to->object)->x;
  hello_header->position.y = get_node_position(to->object)->y;
  hello_header->position.z = get_node_position(to->object)->z;
  
  /* send / push hello packet to the immediately lower-layer simulation module (e.g. MAC) */
  //if ((hello_tx_time - nodedata->last_tx_time) > nodedata->hello_period){
  //	printf("### #HELLO BDCAST DONE");
  	TX(&to0, &from0, packet);
	/* update local stats */
	  nodedata->tx_nbr[header->packet_type]++;
	  
	  /* update global stats */
	  classdata->current_tx_control_packet ++;
	  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
	  
#ifdef ROUTING_LOG_HELLO
	  printf("[ROUTING_LOG_HELLO] Time %lfs node %d has sent a hello packet (%d %d) \n", get_time()*0.000000001, to->object, hello_header->sink_id, hello_header->hop_to_sink);
#endif
  //}
  //else{
  //	  printf("### #HELLO BDCAST DELAYED");
   /* If this node will delay its HELLO BCAST, it has to notify all its neighbors by incrementing their delayed hello from this node */
	  
  //        nodedata->delayed_hello++;
#ifdef ROUTING_LOG_HELLO
	  printf("[ROUTING_LOG_HELLO] Time %lfs node %d delayed a hello packet broadcast (%d %d) \n", get_time()*0.000000001, to->object, hello_header->sink_id, hello_header->hop_to_sink);
#endif
 // }
  

  /* update nbr of transmitted hello packets */
  if (nodedata->hello_nbr > 0) {
    nodedata->hello_nbr--;
  }
  
  /* schedules next hello packet transmission */
  if (nodedata->hello_nbr > 0 || nodedata->hello_nbr == -1) {
    nodedata->previous_hello_slot_time += nodedata->hello_period;
    scheduler_add_callback(nodedata->previous_hello_slot_time + get_random_double() * nodedata->hello_period, to, from, neighbor_discovery_callback_aodv, NULL);
  }
  return 0;
}



/** \brief Callback function for the periodic hello packet transmission in AODV (to be used with the scheduler_add_callback function).
 *  \fn int advert_callback_oracenet(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int advert_callback_oracenet(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
 
 
  /* get a pointer to the lower-layers modules */
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
  
  to->object = 0;
  
  /* create a hello packet */
  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct hello_packet_header), nodedata->hello_packet_real_size*8);
 
  /* extract hello and network headers */
   struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct hello_packet_header *hello_header = malloc(sizeof(struct hello_packet_header));
  field_t *field_hello_header = field_create(INT, sizeof(struct hello_packet_header), hello_header);
  packet_add_field(packet, "hello_packet_header", field_hello_header);
  
  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = BROADCAST_ADDR;
  header->type = SINK_NODE;
  header->packet_type = ADVERT_PACKET;
  
  
  /* Starting Advertisement Wave from the sink node */
  
  TX(&to0, &from0, packet);
  #ifdef ROUTING_LOG_HELLO
  printf("[ROUTING_LOG_ADVERT_TX] Time %lfs sink node %d has broadcasted an Advertisement packet \n", get_time()*0.000000001, to->object);
#endif


/* schedules next advert packet transmission */
/*
   if (nodedata->hello_nbr > 0 || nodedata->hello_nbr == -1) {
       nodedata->previous_hello_slot_time += nodedata->hello_period;
       scheduler_add_callback(nodedata->previous_hello_slot_time + get_random_double() * nodedata->hello_period, to, from, advert_callback_oracenet, NULL);
   }
*/

  /* set hello packet header */
  if (nodedata->node_type == SINK_NODE) {
    hello_header->sink_id = to->object;
    hello_header->hop_to_sink = -1; 
    hello_header->hop = 0; 
  

  }
  else  {
    hello_header->sink_id = -1;
    hello_header->hop_to_sink = -1;
  }
  hello_header->position.x = get_node_position(to->object)->x;
  hello_header->position.y = get_node_position(to->object)->y;
  hello_header->position.z = get_node_position(to->object)->z;
  
  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);

  return 0;
}



/** \brief Callback function for the periodic hello packet transmission in OLSRv2 (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_olsrv2(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_discovery_callback_olsrv2(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  struct neighbor *neighbor = NULL;
 
  /* get a pointer to the lower-layers modules */
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
 
  /* create a hello packet */
  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct hello_packet_header), nodedata->hello_packet_real_size*8);
 
  /* extract hello and network headers */
   struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct hello_packet_header *hello_header = malloc(sizeof(struct hello_packet_header));
  field_t *field_hello_header = field_create(INT, sizeof(struct hello_packet_header), hello_header);
  packet_add_field(packet, "hello_packet_header", field_hello_header);
   
  int i = 0;

  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = BROADCAST_ADDR;
  header->type = nodedata->node_type;
  header->packet_type = HELLO_PACKET;

	
	for(i=0; i< MAX_NEIGHBORS_SIZE; i++){
		hello_header->first_hop_neighbors[i] = -1;
	}

i = 0;


 while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
 
 
/* Add neighbors in the HELLO packet Header */
	
  hello_header->first_hop_neighbors[i] = neighbor->id;

  printf("[ROUTING_LOG_HELLO] %d added in the HELLO_OLSRv2 packet header \n", hello_header->first_hop_neighbors[i], to->object);

  i = i + 1;
}

  /* set hello packet header */
  if (nodedata->node_type == SINK_NODE) {
    hello_header->sink_id = to->object;
    hello_header->hop_to_sink = -1;  
  }
  else  {
    hello_header->sink_id = -1;
    hello_header->hop_to_sink = -1;
  }
  hello_header->position.x = get_node_position(to->object)->x;
  hello_header->position.y = get_node_position(to->object)->y;
  hello_header->position.z = get_node_position(to->object)->z;
  
  /* send / push hello packet to the immediately lower-layer simulation module (e.g. MAC) */
  TX(&to0, &from0, packet);

  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);
  
#ifdef ROUTING_LOG_HELLO
  printf("[ROUTING_LOG_HELLO] Time %lfs node %d has sent a hello packet (%d %d) \n", get_time()*0.000000001, to->object, hello_header->sink_id, hello_header->hop_to_sink);
#endif

  /* update nbr of transmitted hello packets */
  if (nodedata->hello_nbr > 0) {
    nodedata->hello_nbr--;
  }
  
  /* schedules next hello packet transmission */
  if (nodedata->hello_nbr > 0 || nodedata->hello_nbr == -1) {
    nodedata->previous_hello_slot_time += nodedata->hello_period;
    scheduler_add_callback(nodedata->previous_hello_slot_time + get_random_double() * nodedata->hello_period, to, from, neighbor_discovery_callback_olsrv2, NULL);
  }
  return 0;
}


/** \brief Callback function for the periodic TC packet transmission in OLSRv2 (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_olsrv2(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int tc_broadcast_olsrv2(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  struct neighbor *neighbor = NULL;
 
  /* get a pointer to the lower-layers modules */
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
 
  /* create a TC packet */
  packet_t *packet = packet_create(to, nodedata->overhead + sizeof(struct packet_header)+sizeof(struct tc_packet_header), nodedata->tc_packet_real_size*8);
 
  /* extract tc and network headers */
  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct tc_packet_header *tc_header = malloc(sizeof(struct tc_packet_header));
  field_t *field_tc_header = field_create(INT, sizeof(struct tc_packet_header), tc_header);
  packet_add_field(packet, "tc_packet_header", field_tc_header);
   
  int i = 0;

  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }
  
  /* set basic packet header */
  header->src = to->object;
  header->dst = BROADCAST_ADDR;
  header->type = nodedata->node_type;
  header->packet_type = TC_PACKET;

  tc_header->seq = nodedata->tc_seq++;

  /* initialization of 1st hop and mpr table before receiving the new values from the TC header */	
	for(i=0; i< MAX_NEIGHBORS_SIZE; i++){
		tc_header->first_hop_neighbors[i] = -1;
		
	}
/* Add MPRs in the TC packet Header */
	for(i=0; i< MAX_NEIGHBORS_SIZE; i++){
		tc_header->mpr[i] = nodedata->MPR_set[i];
		
	}

	i = 0;

	

	while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) { 
 
/* Add 1st hop neighbors in the TC packet Header */
	
  tc_header->first_hop_neighbors[i] = neighbor->id;


  printf("[ROUTING_LOG_TC] First hop %d added in the TC_OLSRv2 packet header of %d\n", tc_header->first_hop_neighbors[i], to->object);
  i = i + 1;
}

  
  /* send / push hello packet to the immediately lower-layer simulation module (e.g. MAC) */
  TX(&to0, &from0, packet);

  /* update local stats */
  nodedata->tx_nbr[header->packet_type]++;
  
  /* update global stats */
  classdata->current_tx_control_packet ++;
  classdata->current_tx_control_packet_bytes += (packet->real_size/8);

  /* update nbr of transmitted hello packets */
  if (nodedata->tc_nbr > 0) {
    nodedata->tc_nbr--;
  }
  
  /* schedules next TC packet transmission */
  if (nodedata->tc_nbr > 0 || nodedata->tc_nbr == -1) {
    nodedata->previous_tc_slot_time += nodedata->tc_period;
    scheduler_add_callback(nodedata->previous_tc_slot_time + get_random_double() * nodedata->tc_period, to, from, tc_broadcast_olsrv2, NULL);
  }

  return 0;
}

/** \brief Function to update the local node neighbor table in Directed Diffusion according to a received hello packet.
 *  \fn int neighbor_update(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_update(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  /* extract the network and hello headers */
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  field_t *field_hello_header = packet_retrieve_field(packet, "hello_packet_header");
  struct hello_packet_header* hello_header = (struct hello_packet_header*) field_getValue(field_hello_header);
  
  struct neighbor *neighbor = NULL;
  int current_slot, update = 0;


   /* clear the neighbor table from dead/unavailable neighbors */ 
  list_selective_delete(nodedata->neighbors, neighbor_timeout_management, (void *)to);
 
  /* check if neighbor node already exist and update related information */
  list_init_traverse(nodedata->neighbors);
  
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
  
    /* update the entry information if the neighbor already exist */
    if (neighbor->id == header->src) {
      neighbor->type = header->type;
      neighbor->hop_to_sink = hello_header->hop_to_sink;
      current_slot = floor( (get_time()-nodedata->hello_start) / nodedata->hello_period );
      neighbor->rx_nbr++;
      neighbor->loss_nbr += (current_slot - neighbor->slot - 1);
      neighbor->lqe = (neighbor->rx_nbr*1.0 / ((neighbor->rx_nbr + neighbor->loss_nbr)*1.0));
      neighbor->rxdbm = nodedata->rssi_smoothing_factor * neighbor->rxdbm + (1 - nodedata->rssi_smoothing_factor) * packet->rxdBm;
      neighbor->time = get_time();
      neighbor->slot = current_slot;
      neighbor->position.x = hello_header->position.x;
      neighbor->position.y = hello_header->position.y;
      neighbor->position.z = hello_header->position.z;
      update = 1;
      break;
    }
    
  }
  
  /* if the neighbor does not exist, create a new entry in the local neighborhood table */
  if (update == 0) {
    neighbor = (struct neighbor *) malloc(sizeof(struct neighbor));
    neighbor->id = header->src;
    neighbor->type = header->type;
    neighbor->hop_to_sink = hello_header->hop_to_sink;
    neighbor->slot_init = floor((get_time()-nodedata->hello_start) / nodedata->hello_period );
    neighbor->slot = neighbor->slot_init;
    neighbor->rx_nbr = 1;
    neighbor->loss_nbr = neighbor->slot;
    neighbor->lqe = neighbor->rx_nbr / (neighbor->rx_nbr + neighbor->loss_nbr);
    neighbor->rxdbm = packet->rxdBm;
    neighbor->time = get_time();
    neighbor->position.x = hello_header->position.x;
    neighbor->position.y = hello_header->position.y;
    neighbor->position.z = hello_header->position.z;
    list_insert(nodedata->neighbors, (void *) neighbor);
    
    /* set the path establishment time */
    if (neighbor->type == SINK_NODE && nodedata->path_establishment_delay == -1) {
      nodedata->path_establishment_delay = get_time() * 0.000001;
      route_update_global_stats(to, nodedata->path_establishment_delay);
    }
    
#ifdef ROUTING_LOG_HELLO
    printf("[ROUTING_LOG_HELLO] Time %lfs node %d has discovered the neighbor %d \n", get_time()*0.000000001, to->object, neighbor->id);
#endif
  }

  /* update the route table if the neighbor has a good LQE */ 
  if (neighbor->lqe >= nodedata->lqe_threshold) {
    route_update_from_hello(to, header, hello_header, neighbor->lqe);
  }

  return 0;
}

/** ## ORACENET  Neighbor Update **/

int oracenet_neighbor_update(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  

  /* extract the network and hello headers */
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  field_t *field_hello_header = packet_retrieve_field(packet, "hello_packet_header");
  struct hello_packet_header* hello_header = (struct hello_packet_header*) field_getValue(field_hello_header);
  
  struct neighbor *neighbor = NULL;
  int current_slot, update = 0;
  double current_prr;
  double time_to_test;
  
 
  /* Value of the Current nodedata->prr */
   while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
  
    /* update the entry information if the neighbor already exist, needed to update the route from Hello packets*/
    if ((neighbor->id == header->src) && (get_time() > nodedata->hello_start)) {
	current_prr = neighbor->prr;
    } 
    else{
	current_prr = 0.000000;
    }
   }

   /* clear the neighbor table from dead/unavailable neighbors */ 
  list_selective_delete(nodedata->neighbors, neighbor_timeout_management, (void *)to);
 
  /* check if neighbor node already exist and update related information */
  list_init_traverse(nodedata->neighbors);
  
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
  
    /* update the entry information if the neighbor already exist */
    if (neighbor->id == header->src) {
      neighbor->type = header->type;
      neighbor->hop_to_sink = hello_header->hop_to_sink;
      current_slot = floor((get_time() - nodedata->hello_start) / (int)(nodedata->hello_period) );
      neighbor->rx_nbr++;
      nodedata->expected_hello = floor((get_time() - nodedata->hello_start) / nodedata->hello_period) + 1;
      neighbor->prr = (double)(neighbor->rx_nbr)/ (nodedata->expected_hello - nodedata->delayed_hello);
      neighbor->rxdbm = nodedata->rssi_smoothing_factor * neighbor->rxdbm + (1 - nodedata->rssi_smoothing_factor) * packet->rxdBm;
      neighbor->time = get_time();
      neighbor->slot = current_slot;
      neighbor->position.x = hello_header->position.x;
      neighbor->position.y = hello_header->position.y;
      neighbor->position.z = hello_header->position.z;
      update = 1;
      break;
    }
    
  }
  
  /* if the neighbor does not exist, create a new entry in the local neighborhood table */
  if (update == 0) {
    neighbor = (struct neighbor *) malloc(sizeof(struct neighbor));
    neighbor->id = header->src;
    neighbor->type = header->type;
    neighbor->hop_to_sink = hello_header->hop_to_sink;
    neighbor->slot_init = floor((get_time()-nodedata->hello_start) / (int)(nodedata->hello_period) ); 
    neighbor->slot = neighbor->slot_init;
    neighbor->rx_nbr = 1;
    nodedata->expected_hello = floor((get_time() - nodedata->hello_start) / nodedata->hello_period) + 1;
    neighbor->prr = (double)(neighbor->rx_nbr)/ (nodedata->expected_hello - nodedata->delayed_hello);
    neighbor->rxdbm = packet->rxdBm;
    neighbor->time = get_time();
    neighbor->position.x = hello_header->position.x;
    neighbor->position.y = hello_header->position.y;
    neighbor->position.z = hello_header->position.z;
    list_insert(nodedata->neighbors, (void *) neighbor);
    
    /* set the path establishment time */
    if (neighbor->type == SINK_NODE && nodedata->path_establishment_delay == -1) {
      nodedata->path_establishment_delay = get_time() * 0.000001;
      route_update_global_stats(to, nodedata->path_establishment_delay);
    }
    
#ifdef ROUTING_LOG_HELLO
    printf("[ROUTING_LOG_HELLO] Time %lfs node %d has discovered the neighbor %d with PRR %lf \n", get_time()*0.000000001, to->object, neighbor->id, neighbor->prr);
#endif
  }
  /* Update the route table if the neighbor has new better PRR */
   if (neighbor->prr > 0.000000) {
	route_update_oracenet_prr_from_hello(to, packet, header->src); 
   }

  return 0;
}

/* ## ORACENET Neighbor Update from MAC layer */

int oracenet_neighbor_crosslayer_update(call_t *to, packet_t *packet, int prevhop) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  /* extract the network and hello headers */
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  struct neighbor *neighbor = NULL;
  int current_slot, update = 0;
 
  /* clear the neighbor table from dead/unavailable neighbors */ 
  list_selective_delete(nodedata->neighbors, neighbor_timeout_management, (void *)to);
 
  /* check if neighbor node already exist and update related information */
  list_init_traverse(nodedata->neighbors);
  
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
  
    /* update the entry information if the neighbor already exist */
    if (neighbor->id == header->src) {
      neighbor->type = header->type;
      neighbor->hop_to_sink = 1;
      current_slot = floor((get_time() - nodedata->hello_start) / (int)(nodedata->hello_period) );
      neighbor->rx_nbr++;
      nodedata->expected_hello = floor((get_time() - nodedata->hello_start) / nodedata->hello_period) + 1;
      neighbor->prr = (double)(neighbor->rx_nbr)/ (nodedata->expected_hello - nodedata->delayed_hello);
      if (neighbor->prr > 1.000000){
	neighbor->prr = 1.000000;
      }
      neighbor->rxdbm = nodedata->rssi_smoothing_factor * neighbor->rxdbm + (1 - nodedata->rssi_smoothing_factor) * packet->rxdBm;
      neighbor->time = get_time();
      neighbor->slot = current_slot;
      update = 1;
      break;
    }
    
  }
  
  /* if the neighbor does not exist, create a new entry in the local neighborhood table */
  if (update == 0) {
    neighbor = (struct neighbor *) malloc(sizeof(struct neighbor));
    neighbor->id = header->src;
    neighbor->type = header->type;
    neighbor->hop_to_sink = 1;
    neighbor->slot_init = floor((get_time()-nodedata->hello_start) / (int)(nodedata->hello_period) ); 
    neighbor->slot = neighbor->slot_init;
    neighbor->rx_nbr = 1;
    nodedata->expected_hello = floor((get_time() - nodedata->hello_start) / nodedata->hello_period) + 1;
    neighbor->prr = (double)(neighbor->rx_nbr)/ (nodedata->expected_hello - nodedata->delayed_hello);
    if (neighbor->prr > 1.000000){
	neighbor->prr = 1.000000;
    }
    neighbor->rxdbm = packet->rxdBm;
    neighbor->time = get_time();
    list_insert(nodedata->neighbors, (void *) neighbor);
    
    /* set the path establishment time */
    if (neighbor->type == SINK_NODE && nodedata->path_establishment_delay == -1) {
      nodedata->path_establishment_delay = get_time() * 0.000001;
      route_update_global_stats(to, nodedata->path_establishment_delay);
    }
    
#ifdef ROUTING_LOG_HELLO
    printf("[ROUTING_LOG_CROSS_LAYER] Time %lfs node %d has discovered the neighbor %d with PRR %lf \n", get_time()*0.000000001, to->object, neighbor->id, neighbor->prr);
#endif
  }
  return 0;
}





/** \brief Function to update the local node neighbor table in Directed Diffusion according to a received hello packet.
 *  \fn int neighbor_update_from_oracenet_data_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_update_from_oracenet_data_packet(call_t *to, packet_t *packet, int prevhop) {
struct nodedata *nodedata = get_node_private_data(to);
  
  /* extract the network and hello headers */
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  struct neighbor *neighbor = NULL;
  int current_slot, update = 0;
 
  /* clear the neighbor table from dead/unavailable neighbors */ 
  list_selective_delete(nodedata->neighbors, neighbor_timeout_management, (void *)to);
 
  /* check if neighbor node already exist and update related information */
  list_init_traverse(nodedata->neighbors);
  
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
  
    /* update the entry information if the neighbor already exist */
    if (neighbor->id == header->src) {
      neighbor->type = header->type;
      neighbor->hop_to_sink = 1;
      current_slot = floor((get_time() - nodedata->hello_start) / (int)(nodedata->hello_period) );
      neighbor->rx_nbr++;
      nodedata->expected_hello = floor((get_time() - nodedata->hello_start) / nodedata->hello_period) + 1;
      neighbor->prr = (double)(neighbor->rx_nbr)/ (nodedata->expected_hello - nodedata->delayed_hello);
      if (neighbor->prr > 1.000000){
	neighbor->prr = 1.000000;
      }
      neighbor->rxdbm = nodedata->rssi_smoothing_factor * neighbor->rxdbm + (1 - nodedata->rssi_smoothing_factor) * packet->rxdBm;
      neighbor->time = get_time();
      neighbor->slot = current_slot;
      update = 1;
      break;
    }
    
  }
  
  /* if the neighbor does not exist, create a new entry in the local neighborhood table */
  if (update == 0) {
    neighbor = (struct neighbor *) malloc(sizeof(struct neighbor));
    neighbor->id = header->src;
    neighbor->type = header->type;
    neighbor->hop_to_sink = 1;
    neighbor->slot_init = floor((get_time()-nodedata->hello_start) / (int)(nodedata->hello_period) ); 
    neighbor->slot = neighbor->slot_init;
    neighbor->rx_nbr = 1;
    nodedata->expected_hello = floor((get_time() - nodedata->hello_start) / nodedata->hello_period) + 1;
    neighbor->prr = (double)(neighbor->rx_nbr)/ (nodedata->expected_hello - nodedata->delayed_hello);
    if (neighbor->prr > 1.000000){
	neighbor->prr = 1.000000;
    }
    neighbor->rxdbm = packet->rxdBm;
    neighbor->time = get_time();
    list_insert(nodedata->neighbors, (void *) neighbor);
    
    /* set the path establishment time */
    if (neighbor->type == SINK_NODE && nodedata->path_establishment_delay == -1) {
      nodedata->path_establishment_delay = get_time() * 0.000001;
      route_update_global_stats(to, nodedata->path_establishment_delay);
    }
    
#ifdef ROUTING_LOG_HELLO
    printf("[ROUTING_LOG_DATA] Time %lfs node %d has discovered the neighbor %d with PRR %lf \n", get_time()*0.000000001, to->object, neighbor->id, neighbor->prr);
#endif
  }
  return 0;
}

/** \brief Function to update the local node neighbor table in AODV according to a received AODV hello packet.
 *  \fn int neighbor_update_from_aodv_hello(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_update_from_aodv_hello(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);

  /* extract the network and hello headers */
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  field_t *field_hello_header = packet_retrieve_field(packet, "hello_packet_header");
  struct hello_packet_header* hello_header = (struct hello_packet_header*) field_getValue(field_hello_header);
  
  struct neighbor *neighbor = NULL;
  int current_slot, update = 0;

  /* clear the neighbor table from dead/unavailable neighbors */ 
  list_selective_delete(nodedata->neighbors, neighbor_timeout_management, (void *)to);
 
  /* check if neighbor node already exist and update related information */
  list_init_traverse(nodedata->neighbors);

  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
  
    /* update the entry information if the neighbor already exist */
    if (neighbor->id == header->src) {
      neighbor->type = header->type;
      neighbor->hop_to_sink = hello_header->hop_to_sink;
      current_slot = floor( (get_time()-nodedata->hello_start) / nodedata->hello_period );
      neighbor->rx_nbr++;
      neighbor->loss_nbr += (current_slot - neighbor->slot - 1);
      neighbor->lqe = (neighbor->rx_nbr*1.0 / ((neighbor->rx_nbr + neighbor->loss_nbr)*1.0));
      neighbor->rxdbm = nodedata->rssi_smoothing_factor * neighbor->rxdbm + (1 - nodedata->rssi_smoothing_factor) * packet->rxdBm;
      neighbor->time = get_time();
      neighbor->slot = current_slot;
      neighbor->position.x = hello_header->position.x;
      neighbor->position.y = hello_header->position.y;
      neighbor->position.z = hello_header->position.z;
      update = 1;
      break;
    }
    
  }
  
  /* if the neighbor does not exist, create a new entry in the local neighborhood table */
  if (update == 0) {
    neighbor = (struct neighbor *) malloc(sizeof(struct neighbor));
    neighbor->id = header->src;
    neighbor->type = header->type;
    neighbor->hop_to_sink = hello_header->hop_to_sink;
    neighbor->slot_init = floor( (get_time()-nodedata->hello_start) / nodedata->hello_period );
    neighbor->slot = neighbor->slot_init;
    neighbor->rx_nbr = 1;
    neighbor->loss_nbr = neighbor->slot; // = 0
    neighbor->lqe = neighbor->rx_nbr / (neighbor->rx_nbr + neighbor->loss_nbr);
    neighbor->rxdbm = packet->rxdBm;
    neighbor->time = get_time();
    neighbor->position.x = hello_header->position.x;
    neighbor->position.y = hello_header->position.y;
    neighbor->position.z = hello_header->position.z;
    list_insert(nodedata->neighbors, (void *) neighbor);
#ifdef ROUTING_LOG_HELLO
    printf("[ROUTING_LOG_HELLO] Time %lfs node %d has discovered the neighbor %d \n", get_time()*0.000000001, to->object, neighbor->id);
#endif
  }

  return 0;
}



/** \brief Function to update the local node neighbor table in OLSRv2 according to a received OLSRv2 hello packet.
 *  \fn int neighbor_update_from_olsrv2_hello(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise

 *  \ Updated by Dhafer BEN ARBIA in 9-5-2015
 **/

int neighbor_update_from_olsrv2_hello(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);

  /* extract the network and hello headers */
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  field_t *field_hello_header = packet_retrieve_field(packet, "hello_packet_header");
  struct hello_packet_header* hello_header = (struct hello_packet_header*) field_getValue(field_hello_header);
  
  struct neighbor *neighbor = NULL;
  int current_slot, update = 0;
  int Max[MAX_NEIGHBORS_SIZE];
 

  int i = 0;
  int j = 0;
 

for (j=0; j<MAX_NEIGHBORS_SIZE; j++){
		nodedata->neighbors_2hops[header->src][j] = -1;
	}

	
  /* update the neighbor 2 hop table */ 
  list_init_traverse(nodedata->neighbors);
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
      if ((get_time() - neighbor->time) >= nodedata->hello_timeout) {
       	  /* remove neighbor from the 2 hop matrix */
	  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {							
	   	nodedata->neighbors_2hops[neighbor->id][i] = -1; 
	  }

	  nodedata->neighbors_2hops_nbr[neighbor->id][0] = 0;
      	  nodedata->neighbors_2hops_nbr[neighbor->id][1] = 0;
     }
  }

  /* clear the neighbor table from dead/unavailable neighbors */ 
  list_selective_delete(nodedata->neighbors, neighbor_timeout_management, (void *)to);


  /* check if neighbor node already exist and update related information */
  list_init_traverse(nodedata->neighbors);
 

  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
  
 
    /* update the entry information if the neighbor already exist */
    if (neighbor->id == header->src) {
      neighbor->type = header->type;
      neighbor->hop_to_sink = hello_header->hop_to_sink;
      current_slot = floor( (get_time()-nodedata->hello_start) / nodedata->hello_period );
      neighbor->rx_nbr++;
      neighbor->loss_nbr += (current_slot - neighbor->slot - 1);
      neighbor->lqe = (neighbor->rx_nbr*1.0 / ((neighbor->rx_nbr + neighbor->loss_nbr)*1.0));
      neighbor->rxdbm = nodedata->rssi_smoothing_factor * neighbor->rxdbm + (1 - nodedata->rssi_smoothing_factor) * packet->rxdBm;
      neighbor->time = get_time();
      neighbor->slot = current_slot;
      neighbor->position.x = hello_header->position.x;
      neighbor->position.y = hello_header->position.y;
      neighbor->position.z = hello_header->position.z;
      neighbor->neighbors_2hop_nbr = 0;      
      
      update = 1;
      

      /* Copy the 2nd hop neighbor from HELLO header into the neighbors_2hops Matrix */	
	i = 0;
      while (hello_header->first_hop_neighbors[i] !=-1){
		
      	nodedata->neighbors_2hops[neighbor->id][i] = hello_header->first_hop_neighbors[i];

	neighbor->neighbors_2hop_nbr++;

        i = i + 1;
      }

      nodedata->neighbors_2hops_nbr[neighbor->id][0]=i;
      nodedata->neighbors_2hops_nbr[neighbor->id][1]=0;
      }
  
  }
  
  /* if the neighbor does not exist, create a new entry in the local neighborhood table */
  if (update == 0) {
    neighbor = (struct neighbor *) malloc(sizeof(struct neighbor));
    neighbor->id = header->src;
    neighbor->type = header->type;
    neighbor->hop_to_sink = hello_header->hop_to_sink;
    neighbor->slot_init = floor( (get_time()-nodedata->hello_start) / nodedata->hello_period );
    neighbor->slot = neighbor->slot_init;
    neighbor->rx_nbr = 1;
    neighbor->loss_nbr = neighbor->slot; 
    neighbor->lqe = neighbor->rx_nbr / (neighbor->rx_nbr + neighbor->loss_nbr);
    neighbor->rxdbm = packet->rxdBm;
    neighbor->time = get_time();
    neighbor->position.x = hello_header->position.x;
    neighbor->position.y = hello_header->position.y;
    neighbor->position.z = hello_header->position.z;
    neighbor->neighbors_2hop_nbr = 0;
    
	
   /* Copy the 2nd hop neighbor from teh HELLO header into the neighbors_2hops Matrix */	
	i = 0;
      while (hello_header->first_hop_neighbors[i] !=-1){		
      	nodedata->neighbors_2hops[neighbor->id][i] = hello_header->first_hop_neighbors[i];
    	neighbor->neighbors_2hop_nbr++;
        i = i + 1;
      }

      nodedata->neighbors_2hops_nbr[neighbor->id][0]=i;
      nodedata->neighbors_2hops_nbr[neighbor->id][1]=0;

    list_insert(nodedata->neighbors, (void *) neighbor);
#ifdef ROUTING_LOG_HELLO
    printf("[ROUTING_LOG_HELLO] Time %lfs node %d has discovered the neighbor %d which has %d two hop neighbors \n", get_time()*0.000000001, to->object, neighbor->id, neighbor->neighbors_2hop_nbr);
#endif


  return 0;
  
}

}

/** \brief Function to update the local node neighbor table in OLSRv2 according to a received OLSRv2 TC packet.
 *  \fn int neighbor_update_from_olsrv2_TC(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise

 *  \ Updated by Dhafer BEN ARBIA in 24-5-2015
 **/

int route_update_from_tc(call_t *to, packet_t *packet) {

struct nodedata *nodedata = get_node_private_data(to);

  /* extract the network and hello headers */
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  field_t *field_tc_header = packet_retrieve_field(packet, "tc_packet_header");
  struct tc_packet_header* tc_header = (struct tc_packet_header*) field_getValue(field_tc_header);
  int i = 0;

  printf("[ROUTING_LOG_TC] Node %d received TC PACKET from neighbor %d \n ", to->object, header->src);

  // init connectivity matrix
  for (i=0; i<MAX_NEIGHBORS_SIZE; i++){
		nodedata->topology_matrix[header->src][i] = 0;
  }

  // update connectivity matrix
  for (i=0; i<MAX_NEIGHBORS_SIZE; i++){
       if (tc_header->first_hop_neighbors[i] >=0) {
		nodedata->topology_matrix[header->src][tc_header->first_hop_neighbors[i]] = 1;
       }
  }

  return 0;
  
}



/** \brief Function to update the local node neighbor table in AODV according to a received AODV based packet (RREQ, RREP, etc.).
 *  \fn int neighbor_update_from_aodv_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_update_from_aodv_packet(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  /* extract the network and hello headers */
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  struct neighbor *neighbor = NULL;

  /* clear the neighbor table from dead/unavailable neighbors */ 
  list_selective_delete(nodedata->neighbors, neighbor_timeout_management, (void *)to);
 
  /* check if neighbor node already exist and update related information */
  list_init_traverse(nodedata->neighbors);
  
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
 
    /* update the entry information if the neighbor already exist */
    if (neighbor->id == header->src) {
      neighbor->type = header->type;
      neighbor->rxdbm = nodedata->rssi_smoothing_factor * neighbor->rxdbm + (1 - nodedata->rssi_smoothing_factor) * packet->rxdBm;
      neighbor->time = get_time();
      break;
    }
    
  }
  
  return 0;
}


/** \brief Function to remove from the local node neighbor table outdated neighbor entries.
 *  \fn int neighbor_timeout_management(void *data, void *arg)
 *  \param data is a pointer to the neighbor entry
 *  \param arg is a pointer to the function arguments
 *  \return 1 if the entry is no longuer valid, 0 otherwise
 **/
int neighbor_timeout_management(void *data, void *arg) {
  struct neighbor *neighbor = (struct neighbor *) data;
  
  /* extract the function argument */
  call_t *to = (call_t *) arg;
  struct nodedata * nodedata = get_node_private_data(to);

  /* check if the neighbor entry is still valid */
  if ((get_time() - neighbor->time) >= nodedata->hello_timeout) {
	route_remove_oracenet(to, neighbor->id);			// # ORACENET :remove the route depending on the removde neighbor
    return 1;
  }
  
  return 0;
}


/** \brief Function to list on the standard output the local neighbor table.
 *  \fn  void neighbor_show(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void neighbor_show(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL;
 
  list_init_traverse(nodedata->neighbors);

  printf("\n A TC packet at Node %d with MPRs : \n", to->object);
	display_mpr(to);

  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
   // printf("   => Neighbor=%d   type=%d   hop_to_sink=%d  LQE=%lf  \n", neighbor->id, neighbor->type, neighbor->hop_to_sink, neighbor->lqe);
   // printf("\n A TC packet from Node %d with MPRs : \n", header->src);
	//display_mpr(to);
  }

}
/* UPDATED By Dhafer BEN ARBIA, 30 April 2015 */

void neighbor_list(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL;
  int i=0;
  list_init_traverse(nodedata->neighbors);
  
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {
    
  }

}



/* UPDATED By Dhafer BEN ARBIA, 30 April 2015 */
/* This Function show the 1st and 2nd Hop Neighbors for OLSRv2 Protocol */

void neighbor_parse(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL;
  int i = 0;
  
  int Max[MAX_NEIGHBORS_SIZE];

  list_init_traverse(nodedata->neighbors);
  
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL && to->object != neighbor->id){
  
  printf("Node %d  => 1st Hop Neighbor=%d, 2hops neighs = (", to->object, neighbor->id);
  
	/* Show 2nd neighbors of the current node */
	for(i=0; i<MAX_NEIGHBORS_SIZE; i++){
  		if(nodedata->neighbors_2hops[neighbor->id][i] == -1) break;
         	printf("%d, ", nodedata->neighbors_2hops[neighbor->id][i]);

	}
    printf(" )\n ");

    i = i + 1;
  }
}

/* this funtcion is used to format the output in order to use it by the Graph tool Graphiz-DOT */
void graphiz_format(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL;
  int i = 0;
  
  int Max[MAX_NEIGHBORS_SIZE];

  list_init_traverse(nodedata->neighbors);
  
  while((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL && to->object != neighbor->id){
  
  printf("%d -> %d;\n", to->object, neighbor->id);
  
	/* Show 2nd neighbors of the current node */
  	for(i=0; i<MAX_NEIGHBORS_SIZE; i++){
  		if(nodedata->neighbors_2hops[neighbor->id][i] == -1) break;
         	printf("%d -> %d;\n", neighbor->id, nodedata->neighbors_2hops[neighbor->id][i]);

	}
   // printf(" )\n ");

    i = i + 1;
  }
}



/* UPDATED By Dhafer BEN ARBIA, 30 April 2015 */
/* This Function Gets all unique 2nd hop neighbors of each Node*/
/* It takes the Neighbors_Matrix as Input and A Second hop table as Output */


void get_all_2hop_neighbors(call_t *to, int T[MAX_NEIGHBORS_SIZE]) 

{
  struct nodedata *nodedata = get_node_private_data(to);
  int i = 0, j = 0, k = 0;

  /* initialization of the output table to "-1" */
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {							
   	T[i] = -1; 
  }


  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {
     for(j = 0; j < MAX_NEIGHBORS_SIZE; j++) {

		if (nodedata->neighbors_2hops[i][j] == -1) {
			break;
                }
	        /* insert 2 hop neighbor ID in output table */
                else {
			
		  for(k = 1; k < MAX_NEIGHBORS_SIZE; k++) {
   		  	if (T[k] == nodedata->neighbors_2hops[i][j]) {
							
				break;
				
                	}
		        else if (T[k] == -1) {
				
				T[k] = nodedata->neighbors_2hops[i][j];
				
				break;
                        }

                   }

                }
			
	  }

  }
 
}

/* This Function diplays a table content */
void display_table(int T[MAX_NEIGHBORS_SIZE]) {
 int i = 0;

  printf("T = ");
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {	
	if (T[i] != -1) {						
   	printf("%d, ", T[i]); 
  }
  	}
   printf("\n");
}


/* This Function diplays a Matrix content */
void display_matrix(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);

 int i = 0;
 int j = 0;
 int k = 0;

  printf("M = ");
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {
        k = 0;
	for(j = 0; j < MAX_NEIGHBORS_SIZE; j++){
		if (nodedata->neighbors_2hops[i][j] != -1) {						
   			printf("%d, ", nodedata->neighbors_2hops[i][j]); 
                        k ++;
  		} else { break;}
	}
   if (k>0) printf("\n");
  }
   printf("\n");
}

/* This Function diplays the Topology Matrix content */
void display_topology(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);

 int i = 0;
 int j = 0;
 int k = 0;

  printf("TOPOLOGY MATRIX OF NODE %d = \n", to->object);
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {
        k = 0;
	for(j = 0; j < MAX_NEIGHBORS_SIZE; j++){
		if (nodedata->topology_matrix[i][j] != -1) {						
   			printf("%d, ", nodedata->topology_matrix[i][j]); 
                        k ++;
  		} else { break;}
	}
   if (k>0) printf("\n");
  }
   printf("\n");
}



/* This function displays the contnt of the MPR Set from the Nodedata structure */

void display_mpr(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);

 int j = 0;

	printf("\n ------> MPRs of %d are  = ( ", to->object);
			for(j=0; j<MAX_NEIGHBORS_SIZE; j++){
				
				if (nodedata->MPR_set[j] != -1) {
                       			printf("%d ,", nodedata->MPR_set[j]);
				}
			}
			printf(" )");
	printf(" \n");
}

/* UPDATED By Dhafer BEN ARBIA, 30 April 2015 */
/* Check if element exists between the Matrix and the output table */

bool exist (int x, int T[MAX_NEIGHBORS_SIZE]) {
  int i = 0;
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {							
   	if (T[i] == x) return true; 
  }
  return false;
}

bool is_table_empty (int T[MAX_NEIGHBORS_SIZE]) {
  int i = 0;
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {							
   	if (T[i] != -1) return false; 
  }
  return true;
}

/*This function initilize the 2 hop neighbors matrix */
void init_2hop_table(call_t *to) {
  int i = 0;
  struct nodedata *nodedata = get_node_private_data(to);
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {							
	nodedata->neighbors_2hops_nbr[i][1] = 0;
  }
}

/* This function selects which node has the greatest 2hop neighbors */
int get_max_2hop(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  int max = -1, max_id = -1, i,j = 0;
  int T[MAX_NEIGHBORS_SIZE];
  
 // get_all_2hop_neighbors(to, T);
 
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {	
      if (nodedata->neighbors_2hops_nbr[i][1] == 0 && nodedata->neighbors_2hops_nbr[i][0] > max) {
		
	
	   		max = nodedata->neighbors_2hops_nbr[i][0];
           		max_id = i;
       }						
  }

  if (max_id != -1) {
    nodedata->neighbors_2hops_nbr[max_id][1] = 1;
  }

   return max_id;
}

/* This function selects the Multi-Point Relays starting from the 2hop neighbors Matrix and the 2 hops neighbors table */

void mpr_selection(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  int T[MAX_NEIGHBORS_SIZE];
  

  int max_id;
  int i,j,k = 0;
  
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {
	nodedata->MPR_set[i] = -1;
  }

  i = 0; 	
  // to init MPR table and list in nodedata
  init_2hop_table(to);
  get_all_2hop_neighbors(to, T);

  while(is_table_empty (T) == false) {

	
	
	   max_id = get_max_2hop(to); // to store this ID in a list of table of MPR inside nodedata

	for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {
	        for(j = 0; j < MAX_NEIGHBORS_SIZE; j++) {
		  	 if (T[j] == nodedata->neighbors_2hops[max_id][i] && nodedata->neighbors_2hops[max_id][i] != -1){ 
			

			if(exist(nodedata->neighbors_2hops[max_id][i],T) == true && nodedata->neighbors_2hops[max_id][i] != -1){

			     
  /* We get MPRs collected and push them into MPR_set, this condition will avoid duplicated MPR setting*/

				if(exist(max_id,nodedata->MPR_set) == false) {
					nodedata->MPR_set[k] = max_id;
					k++;
				}
			}
		       	T[j] = -1;
				  		    	
			
			}
			
		}
	
        }
	
  }
}



 


