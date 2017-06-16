/**
 *  \file   routing_neighbors_management.h
 *  \brief  Neighbor Management Header File
 *  \author Dhafer Ben Arbia & Elyes Ben Hamida
 *  \date   March 2015
 **/
#ifndef  __routing_neighbors_management__
#define __routing_neighbors_management__


/** \brief Callback function for the periodic hello packet transmission in Directed Diffusion (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_directed_diffusion(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_discovery_callback_directed_diffusion(call_t *to, call_t *from, void *args);


/** \brief Callback function for the periodic advert packet transmission in ORACENET (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_directed_diffusion(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/

int advert_callback_oracenet(call_t *to, call_t *from, void *args);


 
/** \brief Callback function for the periodic hello packet transmission in AODV (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_aodv(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_discovery_callback_aodv(call_t *to, call_t *from, void *args);

/** \brief Callback function for the periodic hello packet transmission in OLSRv2 (to be used with the scheduler_add_callback function).
 *  \fn int neighbor_discovery_callback_aodv(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 *  \ Updated by Dhafer BEN ARBIA May-2015
 **/
int neighbor_discovery_callback_olsrv2(call_t *to, call_t *from, void *args);

/** \brief Callback function for the periodic TC packet transmission in OLSRv2 (to be used with the scheduler_add_callback function).
 *  \fn int neighborhood_broadcast_olsrv2(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 *  \ Updated by Dhafer BEN ARBIA May-2015
 **/

int tc_broadcast_olsrv2(call_t *to, call_t *from, void *args);

/** \brief Function to update the local node neighbor table in Directed Diffusion according to a received hello packet.
 *  \fn int neighbor_update(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_update(call_t *to, packet_t *packet);


/** \brief Function to update the local node neighbor table in AODV according to a received AODV based packet (RREQ, RREP, etc.).
 *  \fn int neighbor_update_from_aodv_packet(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_update_from_aodv_packet(call_t *to, packet_t *packet);


/** \brief Function to update the local node neighbor table in AODV according to a received AODV hello packet.
 *  \fn int neighbor_update_from_aodv_hello(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 **/
int neighbor_update_from_aodv_hello(call_t *to, packet_t *packet);


/** Update neighbor list from OLSRv2 Hello message
/* Updated by Dhafer BEN ARBIA 5-5-2015 */


/** \brief Function to update the local node neighbor table in AODV according to a received AODV hello packet.
 *  \fn int neighbor_update_from_aodv_hello(call_t *to, packet_t *packet)
 *  \param c is a pointer to the called entity
 *  \param packet is a pointer to the received packet
 *  \return 0 if success, -1 otherwise
 *  \ Updated by Dhafer BEN ARBIA May-2015
 **/
int neighbor_update_from_olsrv2_hello(call_t *to, packet_t *packet);


int route_update_from_tc(call_t *to, packet_t *packet);


/** \brief Function to remove from the local node neighbor table outdated neighbor entries.
 *  \fn int neighbor_timeout_management(void *data, void *arg)
 *  \param data is a pointer to the neighbor entry
 *  \param arg is a pointer to the function arguments
 *  \return 1 if the entry is no longuer valid, 0 otherwise
 **/
int neighbor_timeout_management(void *data, void *arg);


/** \brief Function to count hops to the standard output the local neighbor table.
 *  \fn  int neighbor_hop_count(call_t *to)
 *  \param c is a pointer to the called entity
 **/
int neighbor_hop_count(call_t *to);

/** \brief Function to list on the standard output the local neighbor table.
 *  \fn  void neighbor_show(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void neighbor_show(call_t *to);

/** \brief Function to read new neighbor and add it to a table contained in the hello_header.
 *  \fn  void neighbor_parse(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void neighbor_show(call_t *to);

void neighbor_2hops_list(call_t *to);



/** \brief Function to get all 2 hops neighbors towards all 1st hop neighbors.
 *  \fn  void neighbor_parse(call_t *to)
 *  \param c is a pointer to the called entity
 **/
void get_all_2hop_neighbors(call_t *to, int T[MAX_NEIGHBORS_SIZE]);

int neighbor_update_from_oracenet_data_packet(call_t *to, packet_t *packet, int prevhop);

int oracenet_neighbor_update(call_t *to, packet_t *packet);

int neighbor_discovery_callback_oracenet(call_t *to, call_t *from, void *args);

/* Cross layer neighbor update for ORACENET */
int oracenet_neighbor_crosslayer_update(call_t *to, packet_t *packet, int prevhop);

#endif  
