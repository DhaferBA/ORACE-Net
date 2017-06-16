/**
 *  \file   routing_route_management.h
 *  \brief  Route Management Header File
 *  \author Dhafer Ben Arbia & Elyes Ben Hamida
 *  \date   March 2015
 **/
#ifndef  __routing_route_management__
#define __routing_route_management__


#define V 10



typedef int bool;
#define true 1
#define false 0


/** \brief Function to forward received data packet towards the destination (Directed Diffusion)
 *  \fn void route_forward_data_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 **/
void route_forward_data_packet(call_t *to, call_t *from, packet_t *packet);


/** \brief Function to forward received data packet towards the destination (AODV)
 *  \fn void route_forward_data_packet_to_destination(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 **/
void route_forward_data_packet_to_destination(call_t *to, call_t *from, packet_t *packet);


/** \brief Function to forward received RREP packet towards the destination (AODV)
 *  \fn int route_forward_rrep_packet(call_t *to, packet_t* packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_forward_rrep_packet(call_t *to, call_t *from, packet_t* packet);


/** \brief Function to update the local routing table according to received hello packets
 *  \fn int route_update_from_hello(call_t *to, struct packet_header *header, struct hello_packet_header *hello_header, double lqe)
 *  \param c is a pointer to the called entity
 *  \param header is a pointer to the network packet header
 *  \param hello_header is a pointer to the hello packet header
 *  \param lqe is the estimated link quality indicator
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_hello(call_t *to, struct packet_header *header, struct hello_packet_header *hello_header, double lqe);


/** \brief Function to update the local routing table according to received interest packets
 *  \fn  int route_update_from_interest(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_interest(call_t *to, packet_t *packet);

 
/** \brief Function to update the local routing table according to received RREQ packets
 *  \fn  int route_update_from_rreq(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_rreq(call_t *to, packet_t *packet);


/** \brief Function to update the local routing table according to received RREP packets
 *  \fn  int route_update_from_rrep(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_rrep(call_t *to, packet_t *packet);


/** \brief Function to compute the nexthop towards the closest SINK node (Directed Diffusion)
 *  \fn   struct route* route_get_nexthop(call_t *to, int sink_id)
 *  \param c is a pointer to the called entity
 *  \param sink_id is the sink ID
 *  \return NULL if failure, otherwise the destination information of nexthop node
 **/
struct route* route_get_nexthop(call_t *to, int sink_id);

 
/** \brief Function to compute the nexthop towards a given destination (AODV)
 *  \fn   struct route_aodv* route_get_nexthop_to_destination(call_t *to, int dst)
 *  \param c is a pointer to the called entity
 *  \param dst is the ID of the destination
 *  \return NULL if failure, otherwise the destination information of nexthop node
 **/
struct route_aodv* route_get_nexthop_to_destination(call_t *to, int dst);

struct route* route_get_nexthop_to_destination_oracenet(call_t *to, int dst);
struct route* oracenet_route_get_nexthop(call_t *to, int dst);

/** \brief Function to list the routing table on the standard output (Directed Diffusion)
 *  \fn    void route_show(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void route_show(call_t *to);

 
/** \brief Function to list the routing table on the standard output (AODV)
 *  \fn     void route_aodv_show(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void route_aodv_show(call_t *to);

/** \brief Function to update the global entity stats informations (global path establishment time, etc.)
 *  \fn     void route_update_global_stats(call_t *to)
 *  \param c is a pointer to the called entity
 *  \param delay is the path establishment time
 **/
void route_update_global_stats(call_t *to, double path_delay);

/** \brief Function to forwad TC packet
 *  \param to is a pointer to the called entity
 *  \param from is a pointer to the source entity
 *  \param packet is a pointer to the TC packet 
 **/ 

int route_forward_tc_packet(call_t *to, call_t *from, packet_t* packet) ;

/** \brief Function to calculate the Shortest path with Dijkstra Algorithm
 *  \param to is a pointer to the called entity
 *  \param src is the source of the packet
 *  \param dst is the final destination of the packet
 *  \ printsolution, minDistance and printpath are used with Dijkstra
 **/
void dijkstra(call_t *to, int src, int dst);

int printSolution(int dist[], int n);

int minDistance(int dist[], bool sptSet[]);

/** \brief Function to update the local routing table according to received RREP packets
 *  \fn  int route_update_from_oracenet_hello_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int route_update_from_oracenet_data_packet(call_t *to, packet_t *packet, int last_src);

//void route_neighbor_lqe_update(call_t *to, packet_t *packet)
/** Brief Function to remove routes starting by neighbors that disappeared */
void route_remove_oracenet(call_t *to, int nexthop);

void route_get_oracenet_lqe(call_t *to, packet_t *packet, int nexthop);

void route_update_oracenet_prr_from_adv(call_t *to, packet_t *packet, int prevhop);

void route_update_oracenet_prr(call_t *to, packet_t *packet, int nexthop);

int route_update_from_oracenet_hello_packet(call_t *to, struct packet_header *header, struct hello_packet_header *hello_header, double prr);

void route_update_oracenet_prr_from_hello(call_t *to, packet_t *packet, int prevhop);

int oracenet_neighbor_update(call_t *to, packet_t *packet);

#endif  



