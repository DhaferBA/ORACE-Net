/**
 /* OLSRv2 
 /*
**/
#include <stdio.h>
#include <kernel/modelutils.h>
#include "routing_common_types.h"
#include "routing_neighbors_management.h"
#include "routing_rreq_management.h"
#include "routing_routes_management.h"

#define ROUTING_DEBUG 1

/* ************************************************** */
/* ************************************************** */
model_t model =  {
  "OLSRv2 routing protocol",
  "Elyes Ben Hamida & Dhafer Ben Arbia",
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
  int i = 0, j = 0;
  
  /* set the default values for global parameters */
  nodedata->node_type = SENSOR_NODE;
  nodedata->overhead = -1;
  nodedata->neighbors = list_create();

  nodedata->routing_table = list_create();
  nodedata->path_establishment_delay = -1;
  for (i=0; i<5; i++) {
	nodedata->rx_nbr[i] = 0;
	nodedata->tx_nbr[i] = 0;
  }
  nodedata->data_packet_size = -1;
  
  /* set the default values for the real sizes of packets */
  nodedata->hello_packet_real_size  = 16;
  nodedata->tc_packet_real_size     = 12;
  
  for(i = 0; i < MAX_NEIGHBORS_SIZE; i++) {	
	for(j = 0; j < MAX_NEIGHBORS_SIZE; j++){
		nodedata->neighbors_2hops[i][j] = -1;
	}
  }

  /* set the default values for the hello protocol */
  nodedata->hello_status = STATUS_ON;
  nodedata->hello_nbr = -1;
  nodedata->hello_start = 0;
  nodedata->hello_period = 1000000000ull; /* 1s */
  nodedata->hello_timeout = 3*nodedata->hello_period;  

  /* set the default values for the TC Packet */
  /* UPDATED by Dhafer 01-05-2015 */
  nodedata->tc_status = STATUS_ON;
  nodedata->tc_nbr = -1;
  nodedata->tc_start = 0;
  nodedata->tc_period = 1000000000ull; /* 1s */
  nodedata->tc_timeout = 3*nodedata->tc_period; 
  nodedata->tc_seq = 0;
  for(j = 0; j < MAX_NEIGHBORS_SIZE; j++){
		nodedata->tc_cache[j] = -1;
}

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

 /* reading the parameter related to the TC packet from the xml file */
    if (!strcmp(param->key, "tc_status")) {
      if (get_param_integer(param->value, &(nodedata->tc_status))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "tc_nbr")) {
      if (get_param_integer(param->value, &(nodedata->tc_nbr))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "tc_start")) {
      if (get_param_time(param->value, &(nodedata->tc_start))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "tc_period")) {
      if (get_param_time(param->value, &(nodedata->tc_period))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "tc_timeout")) {
      if (get_param_time(param->value, &(nodedata->tc_timeout))) {
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
	
  }
    
/* Check the validity of the input parameters */
  if ((nodedata->node_type != SENSOR_NODE) && (nodedata->node_type != SINK_NODE) && (nodedata->node_type != ANCHOR_NODE)) {
    nodedata->node_type = SENSOR_NODE;
  }

  /* Check protocol status */
  nodedata->hello_status = (nodedata->hello_status > 0)? STATUS_ON : STATUS_OFF;	
	
  set_node_private_data(to, nodedata);
  
   printf("[BIND] node %d %fs %fs \n", to->object, nodedata->hello_start*0.000000001, nodedata->hello_period*0.000000001);

  return 0;
    
 error:
  free(nodedata);
  return -1;
}

int unbind(call_t *to) {
 struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor;
  
#ifdef ROUTING_LOG_NODE_STATS
  int size_hello = (nodedata->hello_packet_real_size != -1) ? nodedata->hello_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct hello_packet_header));
  int size_interest = (nodedata->interest_packet_real_size != -1) ? nodedata->interest_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct sink_interest_packet_header));
  //int size_rreq= (nodedata->rreq_packet_real_size != -1) ? nodedata->rreq_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rreq_packet_header));
  //int size_rrep = (nodedata->rrep_packet_real_size != -1) ? nodedata->rrep_packet_real_size : (int)(nodedata->overhead + sizeof(struct packet_header)+sizeof(struct rrep_packet_header));
  int tx_total = nodedata->tx_nbr[0] + nodedata->tx_nbr[1] + nodedata->tx_nbr[2] + nodedata->tx_nbr[3] + nodedata->tx_nbr[4];
  int rx_total = nodedata->rx_nbr[0] + nodedata->rx_nbr[1] + nodedata->rx_nbr[2] + nodedata->rx_nbr[3] + nodedata->rx_nbr[4];
  struct route_aodv *route = route_get_nexthop_to_destination(to, nodedata->sink_id);

  if (route != NULL) {
    printf("[NETWORK_STATS] node %d type %d X %lf Y %lf Z %lf sink_dst %d nexthop_id %d hops %d neighbors %d routes %d tx_nbr %d rx_nbr %d init_path_delay_ms %lf final_path_delay_ms %lf Hello_packet %d %d %d Interest_packet %d %d %d Data_packet %d %d %d \n", to->object, nodedata->node_type, get_node_position(to->object)->x, get_node_position(to->object)->y, get_node_position(to->object)->z, nodedata->sink_id, route->nexthop_id, route->hop_to_dst , list_getsize(nodedata->neighbors), list_getsize(nodedata->routing_table), tx_total, rx_total, nodedata->path_establishment_delay, route->time*0.000001, nodedata->tx_nbr[0], nodedata->rx_nbr[0], size_hello, nodedata->tx_nbr[1], nodedata->rx_nbr[1], size_interest, nodedata->tx_nbr[2], nodedata->rx_nbr[2], nodedata->data_packet_size, nodedata->tx_nbr[3], nodedata->rx_nbr[3]);
  }
  else {
    printf("[NETWORK_STATS] node %d type %d X %lf Y %lf Z %lf sink_dst %d nexthop_id %d hops %d neighbors %d routes %d tx_nbr %d rx_nbr %d init_path_delay_ms %d final_path_delay_ms %d Hello_packet %d %d %d Interest_packet %d %d %d Data_packet %d %d %d \n", to->object, nodedata->node_type, get_node_position(to->object)->x, get_node_position(to->object)->y, get_node_position(to->object)->z, nodedata->sink_id, -1, -1, list_getsize(nodedata->neighbors), list_getsize(nodedata->routing_table), tx_total, rx_total, -1, -1,
	nodedata->tx_nbr[0], nodedata->rx_nbr[0], size_hello, nodedata->tx_nbr[1], nodedata->rx_nbr[1], size_interest, nodedata->tx_nbr[2], nodedata->rx_nbr[2], nodedata->data_packet_size, nodedata->tx_nbr[3], nodedata->rx_nbr[3]);
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
	scheduler_add_callback(hello_tx_time, to, &from, neighbor_discovery_callback_olsrv2, NULL);
  }

  /* Broadcast the TC Packet */
  if (nodedata->tc_status == STATUS_ON && (nodedata->tc_nbr > 0 || nodedata->tc_nbr == -1)) {
    uint64_t tc_slot_time = get_time() + nodedata->tc_start;
    uint64_t tc_tx_time = tc_slot_time + get_random_double() * nodedata->tc_period;
    nodedata->previous_tc_slot_time = tc_slot_time; 
	scheduler_add_callback(tc_tx_time, to, &from, tc_broadcast_olsrv2, NULL);
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
  struct route_olsrv2 *route = NULL;
  
  struct packet_header *header = malloc(sizeof(struct packet_header));
  field_t *field_packet_header = field_create(INT, sizeof(struct packet_header), header);
  packet_add_field(packet, "packet_header", field_packet_header);
    
  int i = 0;

  if (dst->id == to->object) {
#ifdef ROUTING_DEBUG	  
		printf("[ROUTING_DATA_SETHEADER] node %d cannot send a packet to himself\n", to->object);
#endif
		return -1;
  }

  /* Running Dijkstra to get shortest path with dijkstra */
  dijkstra(to, to->object, dst->id); 


  /* check for a route to the destination */
  if (dst->id != BROADCAST_ADDR) {

	  if (nodedata->olsr_path[0] == -1) {
#ifdef ROUTING_DEBUG	  
		printf("[ROUTING_DATA_SETHEADER] node %d no route to destination %d ==> Packet discarded...\n", to->object, dst->id);
#endif
		return -1;
	  }
	  destination.id = nodedata->olsr_path[1];
	  header->dst = nodedata->olsr_path[1];
	  header->end_dst = dst->id;

	  // add full route and index for the next nexthop node
	  header->olsr_path_index = 1;
	  for (i = 0; i < MAX_NEIGHBORS_SIZE ; i++) {
	  	header->olsr_path[i] = nodedata->olsr_path[i];	  
          }
#ifdef ROUTING_DEBUG
	  printf("[ROUTING_DATA_SETHEADER] time=%fs  Node %d sending packet to Node %d through nexthop %d added to Header \n\n", get_time()*0.000000001,to->object, header->end_dst, header->dst);
#endif
  }
  else {
	  destination.id = BROADCAST_ADDR;
	  header->dst = BROADCAST_ADDR;
  }

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
  

  struct tc_packet_header *tc_header;

  array_t *up = get_class_bindings_up(to);
  int i = up->size;
  
  /* update local stats */
  nodedata->rx_nbr[header->packet_type]++;
  
  /* update the global stats */
  classdata->current_rx_control_packet ++;
  classdata->current_rx_control_packet_bytes += (packet->real_size/8);

#ifdef ROUTING_LOG_RX
  printf("[ROUTING_LOG_RX] Time %lfs N %d has received a packet of type %d from node %d duration %.3f (ms) T_b %.3f (us) real size %d (bytes)\n", get_time()*0.000000001, to->object, header->packet_type, header->src, packet->duration*1e-006, packet->Tb*0.001, packet->real_size/8);
#endif
  

  /* By default anchor nodes does not process any received packet */
  if (nodedata->node_type == ANCHOR_NODE) {
    packet_dealloc(packet);
    return;
  }
  
  /* Update neighbor table from other packets than hello packets */
  if (nodedata->hello_status == STATUS_ON && header->packet_type != HELLO_PACKET) {
  }
  
  /* process the received packet  */
  
  int Max[MAX_NEIGHBORS_SIZE];

 
  switch(header->packet_type) {
  case HELLO_PACKET:         
			/* update local neighbor table */
			neighbor_update_from_olsrv2_hello(to, packet);		
			
			/* Get the Unique entire 2hops neighbors*/
			get_all_2hop_neighbors(to, Max);
			//display_table(Max);

			/* Browse and display the 1st and 2hops neighbors*/    			
			//neighbor_parse(to);     /* displays the current node 1st and 2nd hop connections */
			//graphiz_format(to);     /* displays the output into the graphiz format */

			/* Select the MPR nodes: 1- sorting 1st hop ngbrs with max 2hops ngbrs
						 2- Use a table to cover all 2nd hops ngbrs
						 3- If all 2 hops ngbrs covered stop looking for MPRs*/
						
			mpr_selection(to);	 /* Selects the MPRs */
			//display_mpr(to);	 /* Displays the selected MPRs */
					 
 			
  			/* destroy hello packet */
			packet_dealloc(packet);
			break;

/* UPDATED by Dhafer BEN ARBIA 01-05-2015 */
/* TC packet management 		  */

 case TC_PACKET:         
			/* update topolgy from TC packet */
			route_update_from_tc(to, packet);
			//display_topology(to);
			
			field_t *field_tc_header = packet_retrieve_field(packet, "tc_packet_header");
			struct tc_packet_header* tc_header = (struct tc_packet_header*) field_getValue(field_tc_header);

			
			/* When TC_Packet received no transmit to upper layers */

   			/* forward the TC packet if the current node is an MPR of the sender */
			
			int forward = 0;

			for (i=0; i<MAX_NEIGHBORS_SIZE; i++){
				if (tc_header->mpr[i] == to->object && tc_header->seq > nodedata->tc_cache[header->src] ){

					/* forward the TC packet because I'm MPR */
				 	route_forward_tc_packet(to, from, packet);
					forward = 1;

 					nodedata->tc_cache[header->src] = tc_header->seq;

					break;				
				}
				
			}

			/* destroy hello packet */
			if (forward == 0) {
				packet_dealloc(packet);
			}
			break;
			
			

  case DATA_PACKET:
			/* check if data packet has arrived to destination and forwards to upper layers */
			header->hop++;

			/* Incrementing the hop count variable */

			
			
			if (header->end_dst == to->object) {

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

                         //   - update packet header dst field based on nexthop_index++
				header->olsr_path_index++;				
				header->dst = header->olsr_path[header->olsr_path_index];
				
			 // route_forward_olsrv2_data_packet_to_destination(to, from, packet);
  				route_forward_olsrv2_data_packet_to_destination(to, from, packet);

                         //   - TXsame packet
				//TX(&to, &from, packet);
                         	printf("[ROUTING_LOG_DATA_RX] DATA PACKET has been routed to the final destination : %d=> through next hop :  %d \n", header->end_dst, header->dst);
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
