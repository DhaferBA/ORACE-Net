
/**
 *  \file   directed_diffusion.c
 *  \brief  Directed Diffusion routing protocol
 *  \author Elyes Ben Hamida
 *  \date   2015
 **/
#include <stdio.h>
#include <kernel/modelutils.h>
#include "routing_common_types.h"
#include "routing_neighbors_management.h"
#include "routing_routes_management.h"
#include "routing_sink_interest_management.h"

/* ************************************************** */
/* ************************************************** */
model_t model =  {
  "Directed Diffusion routing protocol",
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
  nodedata->interest_table = list_create();
  nodedata->path_establishment_delay = -1;
  for (i=0; i<5; i++) {
	nodedata->rx_nbr[i] = 0;
	nodedata->tx_nbr[i] = 0;
  }
  nodedata->data_packet_size = -1;
  
  /* set the default values for the real sizes of packets */
  nodedata->hello_packet_real_size     = 16; // as in OLSR v2 RFC3626
  nodedata->interest_packet_real_size  = 12; // as in OLSR v2 RFC3626
  
  /* set the default values for the hello protocol */
  nodedata->hello_status = STATUS_ON;
  nodedata->hello_nbr = -1;
  nodedata->hello_start = 0;
  nodedata->hello_period = 1000000000ull; /* 1s */
  nodedata->hello_timeout = 3*nodedata->hello_period;

  /* set the default values for the link quality estimator */
  nodedata->rssi_smoothing1_nbr = 1;
  nodedata->rssi_smoothing2_nbr = 8;
  nodedata->lqe_w = 10;
  nodedata->lqe_threshold = 0.8;
  nodedata->rssi_smoothing_factor = 0.9;

  /* set the default values for the SINK interest propagation protocol */
  nodedata->sink_interest_status = STATUS_OFF;
  nodedata->sink_interest_propagation_probability = 1.0;
  nodedata->sink_interest_propagation_backoff = 1000000000ull; /* 1s */
  nodedata->sink_interest_nbr = -1;
  nodedata->sink_interest_seq = 0;
  nodedata->sink_interest_start = 0;
  nodedata->sink_interest_period = 10000000000ull; /* 10s */
  nodedata->sink_interest_ttl = 100;
  nodedata->sink_interest_data_type = -1;
  
  /* get params */
  list_init_traverse(params);
  while ((param = (param_t *) list_traverse(params)) != NULL) {
  
  /* reading the parameter related to the node type from the xml file */
    if (!strcmp(param->key, "node_type")) {
      if (get_param_integer(param->value, &(nodedata->node_type))) {
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
	
    /* reading the parameter related to the interest dissemination protocol from the xml file */
    if (!strcmp(param->key, "sink_interest_status")) {
      if (get_param_integer(param->value, &(nodedata->sink_interest_status))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "sink_interest_propagation_probability")) {
      if (get_param_double_range(param->value, &(nodedata->sink_interest_propagation_probability), 0.0, 1.0)) {
	goto error;
      }
    }
    if (!strcmp(param->key, "sink_interest_propagation_backoff")) {
      if (get_param_time(param->value, &(nodedata->sink_interest_propagation_backoff))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "sink_interest_nbr")) {
      if (get_param_integer(param->value, &(nodedata->sink_interest_nbr))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "sink_interest_start")) {
      if (get_param_time(param->value, &(nodedata->sink_interest_start))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "sink_interest_period")) {
      if (get_param_time(param->value, &(nodedata->sink_interest_period))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "sink_interest_ttl")) {
      if (get_param_integer(param->value, &(nodedata->sink_interest_ttl))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "sink_interest_data_type")) {
      if (get_param_integer(param->value, &(nodedata->sink_interest_data_type))) {
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
    if (!strcmp(param->key, "interest_packet_real_size")) {
      if (get_param_integer(param->value, &(nodedata->interest_packet_real_size))) {
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
  nodedata->sink_interest_status = (nodedata->sink_interest_status > 0)? STATUS_ON : STATUS_OFF;
	
  set_node_private_data(to, nodedata);
  return 0;
    
 error:
  free(nodedata);
  return -1;
}

int unbind(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor;
   
#ifdef ROUTING_LOG_CLASS_STATS
  struct route *route = route_get_nexthop(to, -1);
  int size_hello = (nodedata->hello_packet_real_size != -1) ? nodedata->hello_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct hello_packet_header));
  int size_interest = (nodedata->interest_packet_real_size != -1) ? nodedata->interest_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct sink_interest_packet_header));
  int size_rreq= (nodedata->rreq_packet_real_size != -1) ? nodedata->rreq_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rreq_packet_header));
  int size_rrep = (nodedata->rrep_packet_real_size != -1) ? nodedata->rrep_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rrep_packet_header));
  int tx_total = nodedata->tx_nbr[0] + nodedata->tx_nbr[1] + nodedata->tx_nbr[2] + nodedata->tx_nbr[3] + nodedata->tx_nbr[4];
  int rx_total = nodedata->rx_nbr[0] + nodedata->rx_nbr[1] + nodedata->rx_nbr[2] + nodedata->rx_nbr[3] + nodedata->rx_nbr[4];
  if (route != NULL) {
    printf("[ROUTING_LOG_CLASS_STATS] node %d type %d X %lf Y %lf Z %lf sink_dst %d nexthop_id %d hops %d neighbors %d routes %d tx_nbr %d rx_nbr %d init_path_delay_ms %lf final_path_delay_ms %lf Hello_packet %d %d %d Interest_packet %d %d %d Data_packet %d %d %d RREQ_packet %d %d %d RREP_packet %d %d %d\n", to->object, nodedata->node_type, get_node_position(to->object)->x, get_node_position(to->object)->y, get_node_position(to->object)->z, nodedata->sink_id, route->nexthop_id, route->hop_to_sink , list_getsize(nodedata->neighbors), list_getsize(nodedata->routing_table), tx_total, rx_total, nodedata->path_establishment_delay, route->time*0.000001,
	nodedata->tx_nbr[0], nodedata->rx_nbr[0], size_hello, nodedata->tx_nbr[1], nodedata->rx_nbr[1], size_interest, nodedata->tx_nbr[2], nodedata->rx_nbr[2], nodedata->data_packet_size, nodedata->tx_nbr[3], nodedata->rx_nbr[3], size_rreq, nodedata->tx_nbr[4], nodedata->rx_nbr[4], size_rrep);
  }
  else {
    printf("[ROUTING_LOG_CLASS_STATS] node %d type %d X %lf Y %lf Z %lf sink_dst %d nexthop_id %d hops %d neighbors %d routes %d tx_nbr %d rx_nbr %d init_path_delay_ms %d final_path_delay_ms %d Hello_packet %d %d %d Interest_packet %d %d %d Data_packet %d %d %d RREQ_packet %d %d %d RREP_packet %d %d %d\n", to->object, nodedata->node_type, get_node_position(to->object)->x, get_node_position(to->object)->y, get_node_position(to->object)->z, nodedata->sink_id, -1, -1, list_getsize(nodedata->neighbors), list_getsize(nodedata->routing_table), tx_total, rx_total, -1, -1,
	nodedata->tx_nbr[0], nodedata->rx_nbr[0], size_hello, nodedata->tx_nbr[1], nodedata->rx_nbr[1], size_interest, nodedata->tx_nbr[2], nodedata->rx_nbr[2], nodedata->data_packet_size, nodedata->tx_nbr[3], nodedata->rx_nbr[3], size_rreq, nodedata->tx_nbr[4], nodedata->rx_nbr[4], size_rrep);
  }
#endif

  while ((neighbor = (struct neighbor *) list_pop(nodedata->neighbors)) != NULL) {
    free(neighbor);
  }
  
  list_destroy(nodedata->neighbors);    
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

  /* start the sink interest propagation protocol */
  if (nodedata->sink_interest_status == STATUS_ON && nodedata->node_type == SINK_NODE && (nodedata->sink_interest_nbr > 0 || nodedata->sink_interest_nbr == -1)) {
    uint64_t sink_interest_slot_time = get_time() + nodedata->sink_interest_start;
    uint64_t sink_interest_tx_time = sink_interest_slot_time + get_random_double() * nodedata->sink_interest_period;
    nodedata->previous_sink_interest_slot_time = sink_interest_slot_time; 
    scheduler_add_callback(sink_interest_tx_time, to, &from, sink_interest_propagation_callback, NULL);
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

  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
  
  struct route *route = NULL;
  
  /* check for a route to the closest sink */
  if (dst->id == BROADCAST_ADDR) {
    route = route_get_nexthop(to, -1);
  }
  /* check for a route to a particular sink */
  else {
    route = route_get_nexthop(to, dst->id);
  }

  /* if no route, return -1 */
  if (route == NULL) {
    printf("[LOCALG_NET_SETHEADER] node %d no route to destination %d\n", to->object, dst->id);
    return -1;
  }
  
  /* set routing header */
  header->src = to->object;
  header->dst = dst->id;
  header->type = nodedata->node_type;
  header->packet_type = DATA_PACKET;
  header->hop = 0;
  
  /* Set mac header */
  destination.id = route->nexthop_id;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  
  /* Return the hop number to the upper layer */
  dst->position.x = route->hop_to_sink;
  
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
  printf("[LOCALG_NET_DATA_TX] time=%fs   source node %d send a data packet towards sink %d\n", get_time()*0.000000001, header->src, header->dst);
#endif

}


/* ************************************************** */
/* ************************************************** */

void rx(call_t *to, call_t *from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct classdata *classdata = get_class_private_data(to);
  
  field_t *field_header = packet_retrieve_field(packet, "packet_header");
  struct packet_header* header = (struct packet_header*) field_getValue(field_header);
  
  array_t *up = get_class_bindings_up(to);
  int i = up->size;
  
  /* update local stats */
  nodedata->rx_nbr[header->packet_type]++;
  
  /* update the global stats */
  classdata->current_rx_control_packet ++;
  classdata->current_rx_control_packet_bytes += (packet->real_size/8);

#ifdef ROUTING_LOG_RX
  printf("[ROUTING_LOG_RX] Time %lfs Node %d has received a packet of type %d from node %d duration %.3f (ms) T_b %.3f (us) real size %d (bytes)\n", get_time()*0.000000001, to->object, header->packet_type, header->src, packet->duration*1e-006, packet->Tb*0.001, packet->real_size/8);
#endif

  /* By default anchor nodes does not process any received packet */
  if (nodedata->node_type == ANCHOR_NODE) {
    packet_dealloc(packet);
    return;
  }
    
  switch(header->packet_type) {
  case HELLO_PACKET:         
    /* update local neighbor table */
    neighbor_update(to, packet);
    /* destroy hello packet */
    packet_dealloc(packet);
    break;

  case SINK_INTEREST_PACKET:
    /* update routing table of sensor nodes ans schedule packet retransmission using a random backoff period */
    if (nodedata->node_type == SENSOR_NODE) {
      route_update_from_interest(to, packet);
      scheduler_add_callback(get_time() + get_random_double() * nodedata->sink_interest_propagation_backoff, to, from, sink_interest_propagation, (void *)(packet));
    } 
    /* sink and anchor will destroy received packet */
    else {
      packet_dealloc(packet);
    }
    break;

  case DATA_PACKET:
    /* check if data packet has arrived to destination and forwards to upper layers */

    header->hop++;

    if (nodedata->node_type == SINK_NODE && (header->dst == -1 || header->dst == to->object)) {
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
      route_forward_data_packet(to, from, packet);
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
