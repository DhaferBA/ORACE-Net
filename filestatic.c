/**
 *  \file   filestatic.c
 *  \brief  Static routing extracted from file
 *  \author Elyes Ben Hamida and Guillaume Chelius
 *  \date   2007
 **/
#include <stdio.h>

#include <kernel/modelutils.h>

#define UNUSED __attribute__((unused))

/* ************************************************** */
/* ************************************************** */
model_t model =  {
  "Static routing",
  "Elyes Ben Hamida and Guillaume Chelius",
  "0.1",
  MODELTYPE_ROUTING
};


/* ************************************************** */
/* ************************************************** */
struct route {
  nodeid_t dst;
  nodeid_t n_hop;
};

struct routing_header {
  nodeid_t dst;
  nodeid_t src;
};

struct classdata {
  FILE *file;
};

struct nodedata {
  void *routes;
  int overhead;
};


/* ************************************************** */
/* ************************************************** */
int routing_header_size = sizeof(nodeid_t) + sizeof(nodeid_t);
/* header dst, header src */


/* ************************************************** */
/* ************************************************** */
unsigned long route_hash(void *key) { 
  return (unsigned long) key;
}

int route_equal(void *key0, void *key1) { 
  return (int) (key0 == key1);
}


/* ************************************************** */
/* ************************************************** */
int init(call_t *to, void *params) {
  struct classdata *classdata = malloc(sizeof(struct classdata));
  param_t *param;
  char *filepath = NULL;

  /* default values */
  filepath = "routing.data";

  /* get parameters */
  list_init_traverse(params);
  while ((param = (param_t *) list_traverse(params)) != NULL) {
    if (!strcmp(param->key, "file")) {
      filepath = param->value;
    }
  }
  
  /* open file */
  if ((classdata->file = fopen(filepath, "r")) == NULL) {
    fprintf(stderr, "filestatic: can not open file %s in init()\n", filepath);
    goto error;
  }
    
  set_class_private_data(to, classdata);
  return 0;

 error:
  free(classdata);
  return -1;
}

int destroy(call_t *to) {
  struct classdata *classdata = get_class_private_data(to);

  if (classdata->file != NULL) {
    fclose(classdata->file);
  }

  free(classdata);
  return 0;
}


/* ************************************************** */
/* ************************************************** */
int bind(call_t *to, void *params) {
  struct classdata *classdata = get_class_private_data(to);
  struct nodedata *nodedata = malloc(sizeof(struct nodedata));
  char str[128];
  int id, dst, n_hop;
    
  /* extract routing table from file */
  nodedata->routes = hashtable_create(route_hash, route_equal, NULL, NULL);
    
  /* extract routing table from file */
  fseek(classdata->file, 0L, SEEK_SET);
  while (fgets(str, 128, classdata->file) != NULL) {
    if (sscanf(str, "%d %d %d\n",  &id, &dst, &n_hop) != 3) {
      fprintf(stderr, "filestatic: unable to read route in bind()\n");
      goto error;
    }
        
    if (id == to->object) {
      struct route *route = (struct route *) malloc(sizeof(struct route));
      route->dst = dst;
      route->n_hop = n_hop;
      hashtable_insert(nodedata->routes, (void *) ((unsigned long) (route->dst)), (void *) route);
    }
  }
    
  nodedata->overhead = -1;
  set_node_private_data(to, nodedata);
  return 0;

 error:
  free(classdata);
  return -1;
}

int unbind(call_t *to) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route;
  int i = get_node_count();
  while (i--) {
    if ((route = (struct route *) hashtable_retrieve(nodedata->routes, (void *) ((unsigned long) i))) != NULL) {
      free(route);
    }
  } 
  hashtable_destroy(nodedata->routes); 
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
 
  return 0;
}

int ioctl(call_t *to, int option, void *in, void **out) {
  return 0;
}


/* ************************************************** */
/* ************************************************** */
int set_header(call_t *to, call_t *from, packet_t *packet, destination_t *dst) {
  struct nodedata *nodedata = get_node_private_data(to);
  struct route *route = hashtable_retrieve(nodedata->routes, (void *) ((unsigned long) (dst->id)));
  call_t to0 = {get_class_bindings_down(to)->elts[0], to->object};
  destination_t n_hop;

  nodeid_t *header_dst = malloc(sizeof(nodeid_t));
  field_t *field_routing_header_dst = field_create(INT, 8 * sizeof(nodeid_t), header_dst);
  packet_add_field(packet, "routing_header_dst", field_routing_header_dst);

  nodeid_t *header_src = malloc(sizeof(nodeid_t));
  field_t *field_routing_header_src = field_create(INT, 8 * sizeof(nodeid_t), header_src);
  packet_add_field(packet, "routing_header_src", field_routing_header_src);

  if (dst->id == BROADCAST_ADDR) {
    *header_dst = BROADCAST_ADDR;
    *header_src = to->object; 
    n_hop.id = BROADCAST_ADDR;
  }
  else {
    if (route == NULL) {
      fprintf(stderr, "filestatic.c : set header, route null\n");
      return -1;
    }
    *header_dst = dst->id;
    *header_src = to->object;
    n_hop.id = route->n_hop;
  }

  return SET_HEADER(&to0, to, packet, &n_hop);
}

int get_header_size(call_t *to, call_t *from) {
  struct nodedata *nodedata = get_node_private_data(to);
    
  if (nodedata->overhead == -1) {
    call_t to0 = {get_class_bindings_down(to)->elts[0], to->object};
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
  struct nodedata *nodedata = get_node_private_data(to);
  call_t to0 = {get_class_bindings_down(to)->elts[0], to->object};
  nodeid_t UNUSED *header_dst = (nodeid_t *) packet_retrieve_field_value_ptr(packet, "routing_header_dst");
  nodeid_t UNUSED *header_src = (nodeid_t *) packet_retrieve_field_value_ptr(packet, "routing_header_src");
  struct route *route = hashtable_retrieve(nodedata->routes, (void *) ((unsigned long) (*header_dst)));
  destination_t destination;

  if (route == NULL) {
    packet_dealloc(packet);
    return;
  }

  destination.id = route->n_hop;
  if (SET_HEADER(&to0, to, packet, (void *) &destination) == -1) {
    packet_dealloc(packet);
    return;
  }
  TX(&to0, to, packet);
}

void rx(call_t *to, call_t *from, packet_t *packet) {
  array_t *up = get_class_bindings_up(to);
  int i = up->size;
  nodeid_t UNUSED *header_dst = (nodeid_t *) packet_retrieve_field_value_ptr(packet, "routing_header_dst");
  nodeid_t UNUSED *header_src = (nodeid_t *) packet_retrieve_field_value_ptr(packet, "routing_header_src");

  if ((*header_dst != BROADCAST_ADDR) && (*header_dst != to->object)) {
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
}


/* ************************************************** */
/* ************************************************** */
routing_methods_t methods = {rx, 
                             tx, 
                             set_header, 
                             get_header_size,
                             get_header_real_size};
