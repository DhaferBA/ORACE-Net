/**
 *  \file   aodv.c
 *  \brief  AODV routing protocol
 *  \author Elyes Ben Hamida
 *  \date   2015
 **/
#include <stdio.h>
#include <kernel/modelutils.h>
#include "routing_common_types.h"
#include "routing_neighbors_management.h"
#include "routing_rreq_management.h"
#include "routing_routes_management.h"

/* ************************************************** */
/* ************************************************** */
model_t model =  {
  "AODV routing protocol",
  "Elyes Ben Hamida",
  "0.1",
  MODELTYPE_ROUTING
};


/* ************************************************** */
/* ************************************************** */
int init(call_t *to, void *params) {
 struct classdata *classdata = malloc(sizeof(struct classdata));
   
   /* set the default values for global parameters */
   classdata->global_establishment_time = -1;
   
   classdata->global_tx_control_packet = 0;
   classdata->global_rx_control_packet = 0;
   classdata->global_tx_control_packet_bytes = 0;
   classdata->global_rx_control_packet_bytes = 0;
   
   classdata->current_tx_control_packet = 0;
   classdata->current_rx_control_packet = 0;
   classdata->current_tx_control_packet_bytes = 0;
   classdata->current_rx_control_packet_bytes = 0;  
   
    /* Assign the initialized local variables to the node */
    set_class_private_data(to, classdata);
	return 0;
}

int destroy(call_t *to) {
 struct classdata *classdata = get_class_private_data(to);

#ifdef ROUTING_LOG_CLASS_STATS
    if (classdata->global_establishment_time != -1) {
		printf("[NETWORK_GLOBAL_STATS] global_path_establishment_time_ms %f total_tx_nbr %d total_tx_nbr_bytes %d total_rx_nbr %d total_rx_nbr_bytes %d\n", classdata->global_establishment_time, classdata->global_tx_control_packet , classdata->global_tx_control_packet_bytes, classdata->global_rx_control_packet , classdata->global_rx_control_packet_bytes);
	}
#endif
  
	/* destroy/clean the global variables */
	free(classdata);
    return 0;
}


/* ************************************************** */
/* ************************************************** */
int bind(call_t *to, void *params) {
  struct nodedata *nodedata = malloc(sizeof(struct nodedata));
  param_t *param;
  int i = 0;
  
  /* set the default values for global parameters */
  nodedata->node_type = SENSOR_NODE;
  nodedata->overhead = -1;
  nodedata->neighbors = list_create();
  nodedata->routing_table = list_create();
  nodedata->rreq_table = list_create();
  nodedata->rrep_table = list_create();
  nodedata->path_establishment_delay = -1;
  nodedata->sink_id = -1;
  for (i=0; i<5; i++) {
	nodedata->rx_nbr[i] = 0;
	nodedata->tx_nbr[i] = 0;
  }
  nodedata->data_packet_size = -1;
  
  /* set the default values for the real sizes of packets */
  nodedata->hello_packet_real_size      = 20;
  nodedata->rreq_packet_real_size       = 24;
  nodedata->rrep_packet_real_size       = 20;

  /* set the default values for the hello protocol */
  nodedata->hello_status = STATUS_ON;
  nodedata->hello_nbr = -1;
  nodedata->hello_start = 0;
  nodedata->hello_period = 1000000000ull; /* 1s */
  nodedata->hello_timeout = 3*nodedata->hello_period;  
  
  /* set the default values for the AODV protocol */
  nodedata->rreq_status = STATUS_OFF;        
  nodedata->rreq_nbr = -1;                                         
  nodedata->rreq_start = 0;                                 
  nodedata->rreq_period = 10000000000ull; /* 10s */
  nodedata->rreq_propagation_probability = 1.0;
  nodedata->rreq_propagation_backoff = 1000000000ull; /* 1s */
  nodedata->rrep_propagation_backoff = 200000000ull; /* 200ms */
  nodedata->first_rreq_startup_time = 0;
  nodedata->rreq_seq = 0;
  nodedata->rrep_seq = 0;
  nodedata->rreq_ttl = 9999;
  nodedata->rreq_data_type = -1;

  /* set the default values for the link quality estimator */
  nodedata->rssi_smoothing1_nbr = 1;
  nodedata->rssi_smoothing2_nbr = 8;
  nodedata->lqe_w = 10;
  nodedata->lqe_threshold = 0.8;
  nodedata->rssi_smoothing_factor = 0.9;
  
  /* get params */
  list_init_traverse(params);
  while ((param = (param_t *) list_traverse(params)) != NULL) {
  
   /* reading the parameter related to the node type from the xml file */
    if (!strcmp(param->key, "node_type")) {
      if (get_param_integer(param->value, &(nodedata->node_type))) {
	goto error;
      }
    }
    /* reading the parameter related to the node type from the xml file */
    if (!strcmp(param->key, "sink_id")) {
      if (get_param_integer(param->value, &(nodedata->sink_id))) {
	goto error;
      }
    }
    /* reading the parameter related to the hello protocol from the xml file */
    if (!strcmp(param->key, "hello_status")) {
      if (get_param_integer(param->value, &(nodedata->hello_status))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "hello_nbr")) {
      if (get_param_integer(param->value, &(nodedata->hello_nbr))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "hello_start")) {
      if (get_param_time(param->value, &(nodedata->hello_start))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "hello_period")) {
      if (get_param_time(param->value, &(nodedata->hello_period))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "hello_timeout")) {
      if (get_param_time(param->value, &(nodedata->hello_timeout))) {
	goto error;
      }
    }
    /* reading the parameter related to the RREQ protocol from the xml file */
    if (!strcmp(param->key, "rreq_status")) {
      if (get_param_integer(param->value, &(nodedata->rreq_status))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "rreq_nbr")) {
      if (get_param_integer(param->value, &(nodedata->rreq_nbr))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "rreq_start")) {
      if (get_param_time(param->value, &(nodedata->rreq_start))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "rreq_period")) {
      if (get_param_time(param->value, &(nodedata->rreq_period))) {
	goto error;
      }
    }
    /* reading the parameter related to the link quality estimator from the xml file */
    if (!strcmp(param->key, "rssi_smoothing1_nbr")) {
      if (get_param_integer(param->value, &(nodedata->rssi_smoothing1_nbr))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "rssi_smoothing2_nbr")) {
      if (get_param_integer(param->value, &(nodedata->rssi_smoothing2_nbr))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "lqe_w")) {
      if (get_param_integer(param->value, &(nodedata->lqe_w))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "lqe_threshold")) {
      if (get_param_double_range(param->value, &(nodedata->lqe_threshold), 0.0, 1.0)) {
	goto error;
      }
    }
	
    /* reading the parameter related to the real sizes of packets from the xml file */
    if (!strcmp(param->key, "hello_packet_real_size")) {
      if (get_param_integer(param->value, &(nodedata->hello_packet_real_size))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "rreq_packet_real_size")) {
      if (get_param_integer(param->value, &(nodedata->rreq_packet_real_size))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "rrep_packet_real_size")) {
      if (get_param_integer(param->value, &(nodedata->rrep_packet_real_size))) {
	goto error;
      }
    }
	
  }
    
/* Check the validity of the input parameters */
  if ((nodedata->node_type != SENSOR_NODE) && (nodedata->node_type != SINK_NODE) && (nodedata->node_type != ANCHOR_NODE)) {
    nodedata->node_type = SENSOR_NODE;
  }

  /* Check protocol status */
  nodedata->hello_status = (nodedata->hello_status > 0)? STATUS_ON : STATUS_OFF;	
	
  set_node_private_data(to, nodedata);
  return 0;
    
 error:
  free(nodedata);
  return -1;
}

int unbind(call_t *to) {
 struct nodedata *nodedata = get_node_private_data(to);
 struct neighbor *neighbor = NULL;
 struct route_aodv *route = NULL;
 
#ifdef ROUTING_LOG_NODE_STATS
  int size_hello = (nodedata->hello_packet_real_size != -1) ? nodedata->hello_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct hello_packet_header));
  int size_interest = (nodedata->interest_packet_real_size != -1) ? nodedata->interest_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct sink_interest_packet_header));
  int size_rreq= (nodedata->rreq_packet_real_size != -1) ? nodedata->rreq_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rreq_packet_header));
  int size_rrep = (nodedata->rrep_packet_real_size != -1) ? nodedata->rrep_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rrep_packet_header));
  int tx_total = nodedata->tx_nbr[0] + nodedata->tx_nbr[1] + nodedata->tx_nbr[2] + nodedata->tx_nbr[3] + nodedata->tx_nbr[4];
  int rx_total = nodedata->rx_nbr[0] + nodedata->rx_nbr[1] + nodedata->rx_nbr[2] + nodedata->rx_nbr[3] + nodedata->rx_nbr[4];
  route = route_get_nexthop_to_destination(to, nodedata->sink_id);

  if (route != NULL) {
    printf("[NETWORK_STATS] node %d type %d X %lf Y %lf Z %lf sink_dst %d nexthop_id %d hops %d neighbors %d routes %d tx_nbr %d rx_nbr %d init_path_delay_ms %lf final_path_delay_ms %lf Hello_packet %d %d %d Interest_packet %d %d %d Data_packet %d %d %d RREQ_packet %d %d %d RREP_packet %d %d %d\n", to->object, nodedata->node_type, get_node_position(to->object)->x, get_node_position(to->object)->y, get_node_position(to->object)->z, nodedata->sink_id, route->nexthop_id, route->hop_to_dst , list_getsize(nodedata->neighbors), list_getsize(nodedata->routing_table), tx_total, rx_total, nodedata->path_establishment_delay, route->time*0.000001,
	nodedata->tx_nbr[0], nodedata->rx_nbr[0], size_hello, nodedata->tx_nbr[1], nodedata->rx_nbr[1], size_interest, nodedata->tx_nbr[2], nodedata->rx_nbr[2], nodedata->data_packet_size, nodedata->tx_nbr[3], nodedata->rx_nbr[3], size_rreq, nodedata->tx_nbr[4], nodedata->rx_nbr[4], size_rrep);
  }
  else {
    printf("[NETWORK_STATS] node %d type %d X %lf Y %lf Z %lf sink_dst %d nexthop_id %d hops %d neighbors %d routes %d tx_nbr %d rx_nbr %d init_path_delay_ms %d final_path_delay_ms %d Hello_packet %d %d %d Interest_packet %d %d %d Data_packet %d %d %d RREQ_packet %d %d %d RREP_packet %d %d %d\n", to->object, nodedata->node_type, get_node_position(to->object)->x, get_node_position(to->object)->y, get_node_position(to->object)->z, nodedata->sink_id, -1, -1, list_getsize(nodedata->neighbors), list_getsize(nodedata->routing_table), tx_total, rx_total, -1, -1,
	nodedata->tx_nbr[0], nodedata->rx_nbr[0], size_hello, nodedata->tx_nbr[1], nodedata->rx_nbr[1], size_interest, nodedata->tx_nbr[2], nodedata->rx_nbr[2], nodedata->data_packet_size, nodedata->tx_nbr[3], nodedata->rx_nbr[3], size_rreq, nodedata->tx_nbr[4], nodedata->rx_nbr[4], size_rrep);
  }
#endif

  /* free list of neighbors */
  while ((neighbor = (struct neighbor *) list_pop(nodedata->neighbors)) != NULL) {
	free(neighbor);
  }
  list_destroy(nodedata->neighbors);    


  /* Check for the nexthop towards a particular destination */
  list_init_traverse(nodedata->routing_table);
  while((route = (struct route_aodv *) list_traverse(nodedata->routing_table)) != NULL) {
    if (route->dst == 0) {
      printf("[NETWORK_GRAPHVIZ] %d -> %d [style=bold]; \n", to->object, route->nexthop_id);
      free(route);
    }
  }

  free(nodedata);
  return 0;
}


/* ************************************************** */
/* ************************************************** */
int bootstrap(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from = {-1, -1};
  
  /* get mac header overhead */
  nodedata->overhead = GET_HEADER_SIZE(&to0, to);
        
  /* start the neighbor discovery protocol */
  if (nodedata->hello_status == STATUS_ON && (nodedata->hello_nbr > 0 || nodedata->hello_nbr == -1)) {
    uint64_t hello_slot_time = get_time() + nodedata->hello_start;
    uint64_t hello_tx_time = hello_slot_time + get_random_double() * nodedata->hello_period;
    nodedata->previous_hello_slot_time = hello_slot_time; 
	scheduler_add_callback(hello_tx_time, to, &from, neighbor_discovery_callback_aodv, NULL);
  }

   /* start the periodic RREQ generation */
  if (nodedata->rreq_status == STATUS_ON && (nodedata->rreq_nbr > 0 || nodedata->rreq_nbr == -1)) {
    uint64_t rreq_slot_time = get_time() + nodedata->rreq_start;
    uint64_t rreq_tx_time = rreq_slot_time + get_random_double() * nodedata->rreq_period;
    nodedata->previous_rreq_slot_time = rreq_slot_time;
	scheduler_add_callback(rreq_tx_time, to, &from, rreq_periodic_generation_callback, NULL);
  }

  return 0;
}

int ioctl(call_t *to, int option, void *in, void **out) {
  return 0;
}


/* ************************************************** */
/* ************************************************** */
int set_header(call_t *to, call_t *from, packet_t *packet, destination_t *dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  destination_t destination;    
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  struct route_aodv *route = NULL;
  
  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
    
  nodedata->sink_id = dst->id;

  /* check for a route to the destination */
  if (dst->id != BROADCAST_ADDR) {
	  /* if no route, return -1 */
	  if ( (route = route_get_nexthop_to_destination(to, dst->id)) == NULL) {
#ifdef ROUTING_DEBUG	  
		printf("[ROUTING_DATA_SETHEADER] node %d no route to destination %d ==> broadcast RREQ...\n", to->object, dst->id);
#endif
		rreq_propagation_callback(to, from, (void *)dst);
		return -1;
	  }
	  destination.id = route->nexthop_id;
	  header->dst = route->nexthop_id;
	  header->end_dst = dst->id;
  }
  else {
	  destination.id = BROADCAST_ADDR;
	  header->dst = BROADCAST_ADDR;
  }
#ifdef ROUTING_DEBUG	  
  printf("[ROUTING_DATA_SETHEADER] node %d route to destination %d ==> nexthop = %d...\n", to->object, dst->id, destination.id);
#endif  

  /* set packet header */
  header->src = to->object;
  header->type = nodedata->node_type;
  header->packet_type = DATA_PACKET;
  header->hop = 0;

  /* Set mac header */
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;	

  /* Return the hop number to the upper layer */
  if (route != NULL) {
	dst->position.x = route->hop_to_dst;
  }
  
  return SET_HEADER(&to0, to, packet, &destination);
}


int get_header_size(call_t *to, call_t *from) {
  struct nodedata *nodedata = get_node_private_data(to);
  if (nodedata->overhead == -1) {
	call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
	/* get mac header overhead */
	nodedata->overhead = GET_HEADER_SIZE(&to0, to);
  }
  return nodedata->overhead + sizeof(struct packet_header);
}

int get_header_real_size(call_t *to, call_t *from) {
  struct nodedata *nodedata = get_node_private_data(to);
  if (nodedata->overhead == -1) {
    call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
	/* get mac header overhead */
	nodedata->overhead = GET_HEADER_REAL_SIZE(&to0, to);
  }
  return nodedata->overhead + sizeof(struct packet_header);
}

/* ************************************************** */
/* ************************************************** */
void tx(call_t *to, call_t* from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  
  TX(&to0, &from0, packet);

  nodedata->tx_nbr[header->packet_type]++;
  
  /* update the size of data packet */
  nodedata->data_packet_size = packet->real_size / 8;

  #ifdef ROUTING_LOG_DATA_TX
  printf("[ROUTING_LOG_DATA_TX] time=%fs   source node %d send a data packet to DST %d via nexthop %d (packet type=%d) \n", get_time()*0.000000001, header->src, header->end_dst, header->dst, header->packet_type);
#endif
}


/* ************************************************** */
/* ************************************************** */
void rx(call_t *to, call_t *from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  field_t *field_rreq_packet_header, *field_rrep_header;
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  struct rreq_packet_header *rreq_header;
  struct rrep_packet_header *rrep_header;
  struct route_aodv* route;

  array_t *up = get_class_bindings_up(to);
  int i = up->size;
  
  route = NULL;
  
  /* update local stats */
  nodedata->rx_nbr[header->packet_type]++;
  
  /* update the global stats */
  classdata->current_rx_control_packet ++;
  classdata->current_rx_control_packet_bytes += (packet->real_size/8);

#ifdef ROUTING_LOG_RX
  printf("[ROUTING_LOG_RX] Time %lfs Node %d (neighbors = %d) has received a packet of type %d from node %d towards destination %d duration %.3f (ms) T_b %.3f (us) real size %d (bytes)\n", get_time()*0.000000001, to->object, list_getsize(nodedata->neighbors), header->packet_type, header->src, header->dst, packet->duration*1e-006, packet->Tb*0.001, packet->real_size/8);
#endif

  /* By default anchor nodes does not process any received packet */
  if (nodedata->node_type == ANCHOR_NODE) {
    packet_dealloc(packet);
    return;
  }
  
  /* Update neighbor table from other packets than hello packets */
  if (nodedata->hello_status == STATUS_ON && header->packet_type != HELLO_PACKET) {
	//neighbor_update_from_aodv_packet(c, packet);
  }
  
  /* process the received packet  */
  switch(header->packet_type) {
    case HELLO_PACKET:         
			/* update local neighbor table */
			neighbor_update_from_aodv_hello(to, packet);
			/* destroy hello packet */
			packet_dealloc(packet);
			break;

	case RREQ_PACKET: 
			/* extract RREQ packet header */
			field_rreq_packet_header = packet_retrieve_field(packet, "rreq_packet_header");
			rreq_header = (struct rreq_packet_header*) field_getValue(field_rreq_packet_header);
  
			/* update routing table according to received RREQ */
			route_update_from_rreq(to, packet);

			//printf("[ROUTING_LOG_RX] Node %d received RREQ from %d towards %d \n", to->object, rreq_header->src, rreq_header->dst);
			
			/* check if RREQ arrived to destination */
			if (rreq_header->dst == to->object) {

				/* sends RREP only if it is the first received RREQ message */
				if (rreq_table_lookup(to, rreq_header->src, rreq_header->dst, rreq_header->data_type, rreq_header->seq) == 0) {
				#ifdef ROUTING_LOG_RX
					printf("[ROUTING_LOG_RX_RREQ] Time %lfs DST %d has received an RREQ packet from SRC %d (via node %d, ttl=%d, hop=%d, src=%d, seq=%d) => send an RREP packet \n", get_time()*0.000000001, to->object, rreq_header->src, header->src, rreq_header->ttl, rreq_header->ttl_max - (rreq_header->ttl - 1), rreq_header->src, rreq_header->seq);
				#endif
					
			
					rrep_transmission(to, from, rreq_header);
			
				}
				else {
				  //	  printf("[ROUTING_LOG_RX_RREQ] Time %lfs DST %d has received an RREQ packet from SRC %d (via node %d, ttl=%d, hop=%d, src=%d, seq=%d) => RREP packet already sent !\n", get_time()*0.000000001, to->object, rreq_header->src, header->src, rreq_header->ttl, rreq_header->ttl_max - (rreq_header->ttl - 1), rreq_header->src, rreq_header->seq);
				}
				/* update RREQ table to avoid propagating a same RREQ */
				rreq_table_update(to, rreq_header->src, rreq_header->dst, rreq_header->data_type, rreq_header->seq);
				
				//TODO BUG - packet_dealloc(packet);
			}
			
			// /* send a RREP if a path to the DST is already known */
			else if ( (route = route_get_nexthop_to_destination(to, rreq_header->dst)) != NULL && rreq_table_lookup(to, rreq_header->src, rreq_header->dst, rreq_header->data_type, rreq_header->seq) == 0
						&& rrep_table_lookup(to, rreq_header->dst, rreq_header->src, -1, rreq_header->seq) == 0) {

					/* send a RREP to the SRC */
					printf("==================3\n");					
					scheduler_add_callback(get_time() + get_random_double() * nodedata->rrep_propagation_backoff, to, from, rrep_transmission_from_sensor, (void *)(packet));
					printf("==================4\n");
					
					/* TODO: SEND RREP to DST*/
			  
					/* update RREQ table to avoid propagating a same RREQ */
					rreq_table_update(to, rreq_header->src, rreq_header->dst, rreq_header->data_type, rreq_header->seq);
			}			
			
			/* propagate the RREQ if node is a sensor and it has not already sent the RREQ */
			else if (nodedata->node_type == SENSOR_NODE && rreq_table_lookup(to, rreq_header->src, rreq_header->dst, rreq_header->data_type, rreq_header->seq) == 0) {
		#ifdef ROUTING_LOG_RX
				printf("[ROUTING_LOG_RX_RREQ] Time %lfs Node %d has received an RREQ packet from node %d => propagate RREQ packet towards node %d \n", get_time()*0.000000001, to->object, header->src, rreq_header->dst);
		#endif
				 scheduler_add_callback(get_time() + get_random_double() * nodedata->rreq_propagation_backoff, to, from, rreq_propagation, (void *)(packet));
			} 

			/* else destroy the received packet */
			else {
		#ifdef ROUTING_LOG_RX
				printf("[ROUTING_LOG_RX_RREQ] Time %lfs Node %d destroys an RREQ packet from node %d towards node %d \n", get_time()*0.000000001, to->object, header->src, rreq_header->dst);
		#endif
				/* update RREQ table to avoid propagating a same RREQ */
				rreq_table_update(to, rreq_header->src, rreq_header->dst, rreq_header->data_type, rreq_header->seq);
				packet_dealloc(packet);
			}
			break;
			
	case RREP_PACKET: 
			field_rrep_header = packet_retrieve_field(packet, "rrep_packet_header");
			rrep_header = (struct rrep_packet_header*) field_getValue(field_rrep_header);
  
			printf("[ROUTING_LOG_RX_RREP] Time %lfs node %d received a RREP packet:  src=%d, dst=%d, nexthop=%d, seq=%d, hop_to_dst=%d \n", get_time()*0.000000001, to->object, rrep_header->src, rrep_header->dst, header->dst, rrep_header->seq, rrep_header->hop_to_dst);
  
			/* update routing table according to received RREP */
			route_update_from_rrep(to, packet);
			
			/* update the local sink id */
			nodedata->sink_id = rrep_header->src;

			/* compute the initial path establishment time */
			if (nodedata->path_establishment_delay == -1) {
				nodedata->path_establishment_delay = (get_time()-nodedata->first_rreq_startup_time) * 0.000001;
				route_update_global_stats(to, nodedata->path_establishment_delay);
				//printf("[#####] node %d has discovered the SINK %d and the delay is = %fs\n", to->object, rrep_header->src, nodedata->path_establishment_delay);
			}
			
			/* check if RREP packet has arrived to destination */
			
			printf("[ROUTING_LOG_RX_RREP] %d == %d / src = %d / seq = %d ?? \n", rrep_header->dst, to->object,  rrep_header->src, rrep_header->seq);
			
			if (rrep_header->dst == to->object) {
		
		#ifdef ROUTING_LOG_RX
				printf("[ROUTING_LOG_RX_RREP] Time %lfs Node %d has received an RREP packet from node %d (via node=%d, hop_to_dst=%d) => update route table \n", get_time()*0.000000001, to->object, rrep_header->src, header->src, rrep_header->hop_to_dst);
		#endif
				
				/* Destroy the received packet */
				packet_dealloc(packet);
			}
			/* else forward RREP towards the source node */
			else {
				printf("[ROUTING_LOG_RX_RREP] Time %lfs Node %d has received an RREP packet from node %d (via node=%d, hop_to_dst=%d) => FORWARD (rrep_header->dst = %d)\n", get_time()*0.000000001, to->object, rrep_header->src, header->src, rrep_header->hop_to_dst, rrep_header->dst);
				
				route_forward_rrep_packet(to, from, packet);
			}
			break;

  case DATA_PACKET:
  
			header->hop++;
  
			/* check if data packet has arrived to destination and forwards to upper layers */
			if (header->end_dst  == to->object) {
#ifdef ROUTING_LOG_DATA_RX
			  printf("[ROUTING_LOG_DATA_RX]  DST %d has received a data packet from source node %d hops nbr = %d - forwarding to upper layer \n", to->object, header->src, header->hop);
#endif
				while (i--) {
				  call_t to_up = {up->elts[i], to->object};
				  packet_t *packet_up;
						
				  if (i > 0) {
					packet_up = packet_clone(packet);         
				  } else {
					packet_up = packet;
				  }
				  RX(&to_up, to, packet_up);
				}   
			 
			}
			/* forward the data packet to the closest/particular sink */
			else {
			  route_forward_data_packet_to_destination(to, from, packet);
			}
			break;
    
  default : 
		#ifdef ROUTING_LOG_DATA_RX
			printf("[ROUTING_LOG_DATA_RX] Time %lfs Node %d has received an unknown packet type (%d) !\n", get_time()*0.000000001, to->object, header->packet_type);
		#endif
			packet_dealloc(packet);
			break;       
  }
  
  return;
}


/* ************************************************** */
/* ************************************************** */
routing_methods_t methods = {rx, 
                             tx, 
                             set_header, 
                             get_header_size,
                             get_header_real_size};
