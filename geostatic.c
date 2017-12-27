/**
 *  \file   geostatic.c
 *  \brief  Static geographic routing
 *  \author Romain Kuntz
 *  \date   01/2010
 **/

#include <stdio.h>
#include <kernel/modelutils.h>

#define UNUSED __attribute__((unused))

/* ************************************************** */
/* ************************************************** */
model_t model =  {
  "Static geographic routing",
  "Romain Kuntz",
  "0.1",
  MODELTYPE_ROUTING
};

struct neighbor {
  nodeid_t id;
  position_t position;
  uint64_t time;
};

struct nodedata {
  int overhead;
  int hop;       
  int random_nexthop;
  int random_counter;
  double range;

  list_t *neighbors;
  nodeid_t curr_dst;
  struct neighbor* curr_nexthop;
};


/* ************************************************** */
/* ************************************************** */
int routing_header_size = sizeof(nodeid_t) + sizeof(nodeid_t) + sizeof(int);
/* header dst, header src, header hop */


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
/* Find all the neighbors (i.e. nodes in range) of the current node */
int find_neighbors(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL;    
  nodeid_t i;
  double dist = 0;
  int nb_neigh = 0;

  /* Parse all the nodes in the simulation: and find the ones 
   * that are in range of that node
   */
  for(i = 0; i < get_node_count(); i++) {
    /* Do not include myself */
    if (i == to->object) {
      continue;
    }

    dist = distance(get_node_position(to->object), get_node_position(i));
    if (dist <= nodedata->range) {
      /* Add the node in the list of neighbors */
      neighbor = (struct neighbor *) malloc(sizeof(struct neighbor));
      neighbor->id = i;
      neighbor->position.x = get_node_position(i)->x;
      neighbor->position.y = get_node_position(i)->y;
      neighbor->position.z = get_node_position(i)->z;
      neighbor->time = get_time();
      //PRINT_ROUTING("Node %d is added to neighbor list of node %d\n",
      //              neighbor->id, c->node);    
      list_insert(nodedata->neighbors, (void *) neighbor);
      nb_neigh++;
    }
  }

  return nb_neigh;
}

/* Get the best next hop for a specific destination */
struct neighbor* get_nexthop(call_t *to, nodeid_t dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *neighbor = NULL, *n_hop = NULL;
  double dist = distance(get_node_position(to->object), get_node_position(dst));
  double d = dist;

  if (nodedata->curr_dst != dst 
      || nodedata->curr_nexthop == NULL || (!is_node_alive(nodedata->curr_nexthop->id))) {
    /* If the destination is different from the last one,
     * or if the current next hop is dead, reinit the 
     * random counters (to force the selection of a 
     * new next_hop) */
    nodedata->random_counter = nodedata->random_nexthop;
  }

  if (nodedata->random_nexthop == 0) {
    /* We keep the current next hop if 
     * - next hop is not randomized
     * - the next hop is is still alive
     * - the destination is the same 
     */
    if (nodedata->curr_nexthop != NULL
	&& nodedata->curr_dst == dst
	&& is_node_alive(nodedata->curr_nexthop->id)) {
      return nodedata->curr_nexthop;
    }
        
    /* Parse neighbors */
    list_init_traverse(nodedata->neighbors);    
    while ((neighbor = (struct neighbor *) 
	    list_traverse(nodedata->neighbors)) != NULL) {        
      /* Choose next hop (the one the nearest from the final dst) 
       * and verify if it is still alive */
      if ((d = distance(&(neighbor->position), get_node_position(dst))) < dist
	  && is_node_alive(neighbor->id)) {
	dist = d;
	n_hop = neighbor;
      }
    }
  } else if (nodedata->random_counter == nodedata->random_nexthop) {
    list_t *next_hops = list_create();
    int nh = 0;
    double nexthop_dst = 0;

    /* Random geographic routing : we choose randomly among 
     * the neighbors that are nearer from the destination 
     * than the current node.
     */
    list_init_traverse(nodedata->neighbors);    
    while ((neighbor = (struct neighbor *) 
	    list_traverse(nodedata->neighbors)) != NULL) {        
      /* If the neighbor happens to be the final destination, 
       * then we just choose it as the next hop */
      if (neighbor->id == dst) {
	n_hop = neighbor;
	goto out;
      }

      /* Store the neighbors that are nearer from the destination 
       * and that are still alive */
      nexthop_dst = distance(&(neighbor->position), get_node_position(dst));
      if (nexthop_dst < dist && is_node_alive(neighbor->id)) {
	list_insert(next_hops, (void *) neighbor);
      }
    }
    /* Choose next hop randomly among the list */
    nh = list_getsize(next_hops);
    if (nh > 0) {
      int rnd = get_random_integer_range(1, nh);
      while (rnd--) {
	neighbor = (struct neighbor *)list_pop(next_hops);
      }
      n_hop = neighbor;
    }
    list_destroy(next_hops);
  } else /* nodedata->random_counter != nodedata->random_nexthop */ {
    /* Keep the current next hop */
    n_hop = nodedata->curr_nexthop;
  }

 out:
  nodedata->random_counter--;
  if (nodedata->random_counter <= 0) {
    nodedata->random_counter = nodedata->random_nexthop;    
  }

  /* Save the current next hop and destination */
  nodedata->curr_nexthop = n_hop;
  nodedata->curr_dst = dst;
  return n_hop;
}

/* ************************************************** */
/* ************************************************** */
int bind(call_t *to, void *params) {
  struct nodedata *nodedata = malloc(sizeof(struct nodedata));
  param_t *param;

  /* Find all the neighbors for this node */
  nodedata->neighbors = list_create();    
  nodedata->curr_dst = -1;
  nodedata->curr_nexthop = NULL;

  /* default values */
  nodedata->overhead = -1;
  nodedata->hop = 32;
  nodedata->range = 1;
  nodedata->random_nexthop = 0;

  /* Get parameters from config file */
  list_init_traverse((list_t *) params);
  while ((param = (param_t *) list_traverse(params)) != NULL) {
    /* Hop-limit */
    if (!strcmp(param->key, "hop")) {
      if (get_param_integer(param->value, &(nodedata->hop))) {
	goto error;
      }
    }
    /* Range in which neighbors are selected */
    if (!strcmp(param->key, "range")) {
      if (get_param_double(param->value, &(nodedata->range))) {
	goto error;
      }
    }
    /* Randomize the choice of the next hop. 0 means never (always 
     * take the nearest one from the destination), and a value >= 1 
     * randomizes the next hop every "value" time
     */
    if (!strcmp(param->key, "random")) {
      if (get_param_integer(param->value, &(nodedata->random_nexthop))) {
	goto error;
      }
    }
  }
  nodedata->random_counter = nodedata->random_nexthop;
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
  call_t to0 = {get_class_bindings_down(to)->elts[0], to->object};
  int nb_neigh = 0;
    
  /* Get mac header overhead */
  nodedata->overhead = GET_HEADER_SIZE(&to0, to);

  /* Find all the node's neighbors (i.e. the one in range) */
  nb_neigh = find_neighbors(to);
  PRINT_ROUTING("Node %d has %d neighbors\n", to->object, nb_neigh);
        
  return 0;
}

int ioctl(call_t *to, int option, void *in, void **out) {
  return 0;
}

/* ************************************************** */
/* ************************************************** */
int set_header(call_t *to, call_t *from, packet_t *packet, destination_t *dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct neighbor *n_hop = get_nexthop(to, dst->id);
  destination_t destination;
  call_t to0 = {get_class_bindings_down(to)->elts[0], to->object};

  /* No route available */
  if (dst->id != BROADCAST_ADDR && n_hop == NULL) {
    return -1;
  } 

  /* Set routing header */
  nodeid_t *header_dst = malloc(sizeof(nodeid_t));
  field_t *field_routing_header_dst = field_create(INT, 8 * sizeof(nodeid_t), header_dst);
  packet_add_field(packet, "routing_header_dst", field_routing_header_dst);

  nodeid_t *header_src = malloc(sizeof(nodeid_t));
  field_t *field_routing_header_src = field_create(INT, 8 * sizeof(nodeid_t), header_src);
  packet_add_field(packet, "routing_header_src", field_routing_header_src);

  int *header_hop = malloc(sizeof(int));
  field_t *field_routing_header_hop = field_create(INT, 8 * sizeof(int), header_hop);
  packet_add_field(packet, "routing_header_hop", field_routing_header_hop);

  *header_dst = dst->id;
  *header_src = to->object;
  *header_hop = nodedata->hop;

  /* Set MAC header */
  if (dst->id == BROADCAST_ADDR) {
    destination.id = BROADCAST_ADDR;
  } else {
    destination.id = n_hop->id;        
  }
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
    
  return nodedata->overhead + routing_header_size;
}

int get_header_real_size(call_t *to, call_t *from) {
  struct nodedata *nodedata = get_node_private_data(to);

  if (nodedata->overhead == -1) {
    call_t to0 = {get_class_bindings_down(to)->elts[0], to->object};        
    nodedata->overhead = GET_HEADER_REAL_SIZE(&to0, to);
  }
    
  return nodedata->overhead + routing_header_size;
}

/* ************************************************** */
/* ************************************************** */
void tx(call_t *to, call_t *from, packet_t *packet) {
  call_t to0 = {get_class_bindings_down(to)->elts[0], to->object};
  TX(&to0, to, packet);
}

/* ************************************************** */
/* ************************************************** */
void forward(call_t *to, call_t *from, packet_t *packet) {  
  call_t to0 = {get_class_bindings_down(to)->elts[0], to->object};
  nodeid_t UNUSED *header_dst = (nodeid_t *)  packet_retrieve_field_value_ptr(packet, "routing_header_dst");
  nodeid_t UNUSED *header_src = (nodeid_t *)  packet_retrieve_field_value_ptr(packet, "routing_header_src");
  int UNUSED *header_hop = (int *)  packet_retrieve_field_value_ptr(packet, "routing_header_hop");
  struct neighbor *n_hop = get_nexthop(to, *header_dst);
  destination_t destination;    

  /* No route available */    
  if (n_hop == NULL) {
    packet_dealloc(packet);        
    return;
  }
    
  /* Update hop count */
  (*header_hop)--;

  /* Hop count reached */
  if (*header_hop == 0) {
    packet_dealloc(packet);
    return;
  }

  /* Set MAC header */
  destination.id = n_hop->id;
  destination.position.x = -1;
  destination.position.y = -1;
  destination.position.z = -1;
  if (SET_HEADER(&to0, to, packet, &destination) == -1) {
    packet_dealloc(packet);
    return;
  }
    
  /* Forwarding packet */
  PRINT_ROUTING("forward: Node %d forwards a packet "
		"(from %d to %d, hop limit %d)\n",
		to->object, *header_src, *header_dst, 
		*header_hop);
  TX(&to0, to, packet);
}


/* ************************************************** */
/* ************************************************** */
void rx(call_t *to, call_t *from, packet_t *packet) {
  array_t *up = get_class_bindings_up(to);
  int i = up->size;
  nodeid_t UNUSED *header_dst = (nodeid_t *)  packet_retrieve_field_value_ptr(packet, "routing_header_dst");
  nodeid_t UNUSED *header_src = (nodeid_t *)  packet_retrieve_field_value_ptr(packet, "routing_header_src");
  int UNUSED *header_hop = (int *)  packet_retrieve_field_value_ptr(packet, "routing_header_hop");

  /* Forward packet if node is not the recipient */
  if ((*header_dst != BROADCAST_ADDR) && (*header_dst != to->object) ) {
    forward(to, from, packet);
    return;
  }
        
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

  return;
}

/* ************************************************** */
/* ************************************************** */
routing_methods_t methods = {rx, 
                             tx, 
                             set_header, 
                             get_header_size,
                             get_header_real_size};
