/**
 *  \file   greedy.c
 *  \brief  Greedy geographic routing
 *  \author Elyes Ben Hamida and Guillaume Chelius
 *  \date   2007
 **/
#include <stdio.h>
#include <kernel/modelutils.h>


/* ************************************************** */
/* ************************************************** */
model_t model =  {
  "Greedy geographic routing",
  "Elyes Ben Hamida and Guillaume Chelius",
  "0.1",
  MODELTYPE_ROUTING
};


/* ************************************************** */
/* ************************************************** */
#define HELLO_PACKET 0
#define DATA_PACKET  1


/* ************************************************** */
/* ************************************************** */
int routing_header_size =
  sizeof(nodeid_t)   + 
  3 * sizeof(double) +
  sizeof(nodeid_t)   +
  3 * sizeof(double) +
  sizeof(int)        +
  sizeof(int);
/* header dst, header dst_pos, header src, header src_pos, header hop, header type */


struct neighbor {
  int id;
  position_t position;
  uint64_t time;
};

struct nodedata {
  list_t *neighbors;
  int overhead;

  uint64_t start;
  uint64_t period;
  uint64_t timeout;
  int hop;     

  int hello_packet_real_size;  

  /* stats */
  int hello_tx;
  int hello_rx;
  int data_tx;
  int data_rx;
  int data_noroute;
  int data_hop;
};


/* ************************************************** */
/* ************************************************** */
int advert_callback(call_t *to, call_t *from, void *args);
void display_neighbors(call_t *to);


/* ************************************************** */
/* ************************************************** */
int init(call_t *to, void *params) {
  return 0;
}

int destroy(call_t *to) {
  return 0;
}


/* ************************************************** */
/* ************************************************** */
int bind(call_t *to, void *params) {
  struct nodedata *nodedata = malloc(sizeof(struct nodedata));
  param_t *param;

  /* default values */
  nodedata->neighbors = list_create();
  nodedata->overhead = -1;
  nodedata->hello_tx = 0;
  nodedata->hello_rx = 0;
  nodedata->data_tx = 0;
  nodedata->data_rx = 0;
  nodedata->data_noroute = 0;
  nodedata->data_hop = 0;
  nodedata->start = 0;
  nodedata->hop = 32;
  nodedata->period = 1000000000;
  nodedata->timeout = 2500000000ull;
  nodedata->hello_packet_real_size = 16;  // as in OLSR v2 RFC3626
 
  /* get params */
  list_init_traverse(params);
  while ((param = (param_t *) list_traverse(params)) != NULL) {
    if (!strcmp(param->key, "start")) {
      if (get_param_time(param->value, &(nodedata->start))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "period")) {
      if (get_param_time(param->value, &(nodedata->period))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "hop")) {
      if (get_param_integer(param->value, &(nodedata->hop))) {
	goto error;
      }
    }
    if (!strcmp(param->key, "timeout")) {
      if (get_param_time(param->value, &(nodedata->timeout))) {
	goto error;
      }
    }
  }
    
  set_node_private_data(to, nodedata);
  return 0;
    
 error:
  free(nodedata);
  return -1;
}

int unbind(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor;
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
    
  /* get mac header overhead */
  nodedata->overhead = GET_HEADER_SIZE(&to0, to);
        
  /*  hello packet */
  if (nodedata->period > 0) {
    call_t from = {-1, -1};
    uint64_t start = get_time() + nodedata->start + get_random_double() * nodedata->period;
    scheduler_add_callback(start, to, &from, advert_callback, NULL);
  }

  return 0;
}

int ioctl(call_t *to, int option, void *in, void **out) {
  return 0;
}

/* ************************************************** */
/* ************************************************** */
struct neighbor* get_nexthop(call_t *to, position_t *dst, int dst_id) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL, *n_hop = NULL;
  uint64_t clock = get_time();
  double dist = distance(get_node_position(to->object), dst);
  double d = dist;
  
  /* parse neighbors */
  list_init_traverse(nodedata->neighbors);    
  while ((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {        
    if ((nodedata->timeout > 0)
	&& (clock - neighbor->time) >= nodedata->timeout ) {
      continue;
    }
        
    /* choose next hop */
    if ((d = distance(&(neighbor->position), dst)) < dist) {
      dist = d;
      n_hop = neighbor;
    }
	
	/* stop in case the exact destination was found */
	if (neighbor->id == dst_id) {
		n_hop = neighbor;
		break;
	}
  }    

  return n_hop;
}


void display_neighbors(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL;

  /* parse neighbors */
  list_init_traverse(nodedata->neighbors);    
  printf("[NEIGHBORS] Node %d : ", to->object);
  while ((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {        
	printf("%d (%.3f), ", neighbor->id, distance(&(neighbor->position), get_node_position(to->object)));
  }    
  printf("\n");
}

void add_neighbor(call_t *to, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL;

  /* check wether neighbor already exists */
  list_init_traverse(nodedata->neighbors);      
  while ((neighbor = (struct neighbor *) list_traverse(nodedata->neighbors)) != NULL) {      
    if (neighbor->id == *((int*)packet_retrieve_field_value_ptr(packet,"routing_header_src"))) {
      neighbor->position.x = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_src_pos_x"));
      neighbor->position.y = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_src_pos_y"));
      neighbor->position.z = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_src_pos_z"));
      neighbor->time = get_time();
      return;
    }
  }  

  neighbor = (struct neighbor *) malloc(sizeof(struct neighbor));
  neighbor->id = *((int*)packet_retrieve_field_value_ptr(packet,"routing_header_src"));
  neighbor->position.x = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_src_pos_x"));
  neighbor->position.y = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_src_pos_y"));
  neighbor->position.z = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_src_pos_z"));
  neighbor->time = get_time();
  list_insert(nodedata->neighbors, (void *) neighbor);
  return;
}


/* ************************************************** */
/* ************************************************** */
int set_header(call_t *to, call_t *from, packet_t *packet, destination_t *dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *n_hop = get_nexthop(to, &(dst->position), dst->id);
  destination_t destination;    
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  int next_hop;

  /* if no route, return -1 */
  if (dst->id != BROADCAST_ADDR && n_hop == NULL) {
    nodedata->data_noroute++;
    return -1;
  }
  else if (dst->id == BROADCAST_ADDR) {
    next_hop = BROADCAST_ADDR;
  }
  else {
    next_hop = n_hop->id;
  }
  
  /* set routing header */
  nodeid_t *routing_header_dst = malloc(sizeof(nodeid_t));
  *routing_header_dst = dst->id;
  field_t *field_routing_header_dst = field_create(INT, 8 * sizeof(nodeid_t), routing_header_dst);
  packet_add_field(packet, "routing_header_dst", field_routing_header_dst);
  //---------
  double *routing_header_dst_pos_x = malloc(sizeof(double));
  *routing_header_dst_pos_x = dst->position.x;
  field_t *field_routing_header_dst_pos_x = field_create(DBLE, sizeof(double), routing_header_dst_pos_x);
  packet_add_field(packet, "routing_header_dst_pos_x", field_routing_header_dst_pos_x);
  //---------
  double *routing_header_dst_pos_y = malloc(sizeof(double));
  *routing_header_dst_pos_y = dst->position.y;
  field_t *field_routing_header_dst_pos_y = field_create(DBLE, sizeof(double), routing_header_dst_pos_y);
  packet_add_field(packet, "routing_header_dst_pos_y", field_routing_header_dst_pos_y);
  //---------
  double *routing_header_dst_pos_z = malloc(sizeof(double));
  *routing_header_dst_pos_z = dst->position.z;
  field_t *field_routing_header_dst_pos_z = field_create(DBLE, sizeof(double), routing_header_dst_pos_z);
  packet_add_field(packet, "routing_header_dst_pos_z", field_routing_header_dst_pos_z);
  //---------
  nodeid_t *routing_header_src = malloc(sizeof(nodeid_t));
  *routing_header_src = to->object;
  field_t *field_routing_header_src = field_create(INT, sizeof(nodeid_t), routing_header_src);
  packet_add_field(packet, "routing_header_src", field_routing_header_src);
  //---------
  double *routing_header_src_pos_x = malloc(sizeof(double));
  *routing_header_src_pos_x = get_node_position(to->object)->x;
  field_t *field_routing_header_src_pos_x = field_create(DBLE, sizeof(double), routing_header_src_pos_x);
  packet_add_field(packet, "routing_header_src_pos_x", field_routing_header_src_pos_x);
  //---------
  double *routing_header_src_pos_y = malloc(sizeof(double));
  *routing_header_src_pos_y = get_node_position(to->object)->y;
  field_t *field_routing_header_src_pos_y = field_create(DBLE, sizeof(double), routing_header_src_pos_y);
  packet_add_field(packet, "routing_header_src_pos_y", field_routing_header_src_pos_y);
  //---------
  double *routing_header_src_pos_z = malloc(sizeof(double));
  *routing_header_src_pos_z = get_node_position(to->object)->z;
  field_t *field_routing_header_src_pos_z = field_create(DBLE, sizeof(double), routing_header_src_pos_z);
  packet_add_field(packet, "routing_header_src_pos_z", field_routing_header_src_pos_z);
  //---------
  int *routing_header_type = malloc(sizeof(int));
  *routing_header_type = DATA_PACKET;
  field_t *field_routing_header_type = field_create(INT, sizeof(int), routing_header_type);
  packet_add_field(packet, "routing_header_type", field_routing_header_type);
  //---------
  int *routing_header_hop = malloc(sizeof(int));
  *routing_header_hop = nodedata->hop;
  field_t *field_routing_header_hop = field_create(INT, sizeof(int), routing_header_hop);
  packet_add_field(packet, "routing_header_hop", field_routing_header_hop);
  
  
  /* Set mac header */
  destination.id = next_hop;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  return SET_HEADER(&to0, to, packet, &destination);
}

int get_header_size(call_t *to, call_t *from) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  if (nodedata->overhead == -1) {
    call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
    nodedata->overhead = GET_HEADER_SIZE(&to0, to);
  }

  return (nodedata->overhead + routing_header_size);
}

int get_header_real_size(call_t *to, call_t *from) {
  struct nodedata *nodedata = get_node_private_data(to);
  
  if (nodedata->overhead == -1) {
    call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
    nodedata->overhead = GET_HEADER_REAL_SIZE(&to0, to);
  }
  
  return nodedata->overhead + routing_header_size;
}


/* ************************************************** */
/* ************************************************** */
int neighbor_timeout(void *data, void *arg) {
  struct neighbor *neighbor = (struct neighbor *) data;
  call_t *to = (call_t *) arg;
  struct nodedata *nodedata = get_node_private_data(to);
  if ((get_time() - neighbor->time) >= nodedata->timeout) {
    return 1;
  }
  return 0;
}

int advert_callback(call_t *to, call_t *from, void *args) {
  struct nodedata *nodedata = get_node_private_data(to);
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  destination_t destination = {BROADCAST_ADDR, {-1, -1, -1}};
  packet_t *packet = packet_create(to, nodedata->overhead + routing_header_size, nodedata->hello_packet_real_size*8);

  /* set mac header */
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return -1;
  }

  /* set routing header */
  nodeid_t *routing_header_dst = malloc(sizeof(nodeid_t));
  *routing_header_dst = BROADCAST_ADDR;
  field_t *field_routing_header_dst = field_create(INT, sizeof(nodeid_t), routing_header_dst);
  packet_add_field(packet, "routing_header_dst", field_routing_header_dst);
  //---------
  double *routing_header_dst_pos_x = malloc(sizeof(double));
  *routing_header_dst_pos_x = -1.0;
  field_t *field_routing_header_dst_pos_x = field_create(DBLE, sizeof(double), routing_header_dst_pos_x);
  packet_add_field(packet, "routing_header_dst_pos_x", field_routing_header_dst_pos_x);
  //---------
  double *routing_header_dst_pos_y = malloc(sizeof(double));
  *routing_header_dst_pos_y = -1.0;
  field_t *field_routing_header_dst_pos_y = field_create(DBLE, sizeof(double), routing_header_dst_pos_y);
  packet_add_field(packet, "routing_header_dst_pos_y", field_routing_header_dst_pos_y);
  //---------
  double *routing_header_dst_pos_z = malloc(sizeof(double));
  *routing_header_dst_pos_z = -1.0;
  field_t *field_routing_header_dst_pos_z = field_create(DBLE, sizeof(double), routing_header_dst_pos_z);
  packet_add_field(packet, "routing_header_dst_pos_z", field_routing_header_dst_pos_z);
  //---------
  nodeid_t *routing_header_src = malloc(sizeof(nodeid_t));
  *routing_header_src = to->object;
  field_t *field_routing_header_src = field_create(INT, sizeof(nodeid_t), routing_header_src);
  packet_add_field(packet, "routing_header_src", field_routing_header_src);
  //---------
  double *routing_header_src_pos_x = malloc(sizeof(double));
  *routing_header_src_pos_x = get_node_position(to->object)->x;
  field_t *field_routing_header_src_pos_x = field_create(DBLE, sizeof(double), routing_header_src_pos_x);
  packet_add_field(packet, "routing_header_src_pos_x", field_routing_header_src_pos_x);
  //---------*((int*)packet_retrieve_field_value_ptr(packet,"routing_header_src"))
  double *routing_header_src_pos_y = malloc(sizeof(double));
  *routing_header_src_pos_y = get_node_position(to->object)->y;
  field_t *field_routing_header_src_pos_y = field_create(DBLE, sizeof(double), routing_header_src_pos_y);
  packet_add_field(packet, "routing_header_src_pos_y", field_routing_header_src_pos_y);
  //---------
  double *routing_header_src_pos_z = malloc(sizeof(double));
  *routing_header_src_pos_z = get_node_position(to->object)->z;
  field_t *field_routing_header_src_pos_z = field_create(DBLE, sizeof(double), routing_header_src_pos_z);
  packet_add_field(packet, "routing_header_src_pos_z", field_routing_header_src_pos_z);
  //---------
  int *routing_header_type = malloc(sizeof(int));
  *routing_header_type = HELLO_PACKET;
  field_t *field_routing_header_type = field_create(INT, sizeof(int), routing_header_type);
  packet_add_field(packet, "routing_header_type", field_routing_header_type);
  //---------
  int *routing_header_hop = malloc(sizeof(int));
  *routing_header_hop = 1;
  field_t *field_routing_header_hop = field_create(INT, sizeof(int), routing_header_hop);
  packet_add_field(packet, "routing_header_hop", field_routing_header_hop);

  /* send hello */
  TX(&to0, to, packet);
  nodedata->hello_tx++;

  /* check neighbors timeout  */
  list_selective_delete(nodedata->neighbors, neighbor_timeout, (void *) to);

  /* schedules hello */
  scheduler_add_callback(get_time() + nodedata->period, to, from, advert_callback, NULL);

  return 0;
}


/* ************************************************** */
/* ************************************************** */
void tx(call_t *to, call_t* from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
    
  nodedata->data_tx++;
  TX(&to0, &from0, packet);
}


/* ************************************************** */
/* ************************************************** */
void forward(call_t *to, call_t *from, packet_t *packet) {  
  struct nodedata *nodedata = get_node_private_data(to);
  call_t to0   = {get_class_bindings_down(to)->elts[0], to->object};
  call_t from0 = {to->class, to->object};
  position_t dst_pos;
  
  dst_pos.x = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_dst_pos_x"));
  dst_pos.y = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_dst_pos_y"));
  dst_pos.z = *((double*)packet_retrieve_field_value_ptr(packet,"routing_header_dst_pos_z"));
  
  int dst = *((int*)packet_retrieve_field_value_ptr(packet,"routing_header_dst"));

  
  struct neighbor *n_hop = get_nexthop(to, &dst_pos, dst);
  destination_t destination;    

  /* delivers packet to application layer */
  if (n_hop == NULL) {
    /*  array_t *up = get_class_bindings_up(to);
    int i = up->size;
        
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
    */
    packet_dealloc(packet);
    //display_neighbors(to);
	
    return;
    }
    
  /* update hop count */
  (*((int*)packet_retrieve_field_value_ptr(packet,"routing_header_hop")))--;
  if (*((int*)packet_retrieve_field_value_ptr(packet,"routing_header_hop")) == 0) {
    nodedata->data_hop++;
    packet_dealloc(packet);
    return;
  }
    

  /* set mac header */
  destination.id = n_hop->id;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  if (SET_HEADER(&to0, &from0, packet, &destination) == -1) {
    packet_dealloc(packet);
    return;
  }
    
  /* forwarding packet */
  nodedata->data_tx++;
  TX(&to0, &from0, packet);
}


/* ************************************************** */
/* ************************************************** */
void rx(call_t *to, call_t *from, packet_t *packet) {
  struct nodedata *nodedata = get_node_private_data(to);
  array_t *up = get_class_bindings_up(to);
  int i = up->size;

  switch(*((int*)packet_retrieve_field_value_ptr(packet,"routing_header_type"))) {
  case HELLO_PACKET:         
    nodedata->hello_rx++;
    add_neighbor(to, packet);
    packet_dealloc(packet);
    break;

  case DATA_PACKET : 
    nodedata->data_rx++;
    int dst = *((int*)packet_retrieve_field_value_ptr(packet,"routing_header_dst"));
    if ( (dst != BROADCAST_ADDR) && (dst != to->object) ) {
      forward(to, from, packet);
      return;
    }
       int hops = nodedata->hop - (*((int*)packet_retrieve_field_value_ptr(packet,"routing_header_hop"))) + 1;

//#ifdef ROUTING_LOG_DATA_RX
			  printf("[ROUTING_LOG_DATA_RX]  DST %d has received a data packet from source node %d hops nbr = %d - forwarding to upper layer \n", to->object, *((int*)packet_retrieve_field_value_ptr(packet,"routing_header_src")), hops);
//#endif


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
    break;

  default :
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
