/**
 *  \file   routing_rreq_management.h
 *  \brief  AODV RREQ/RREP Management Header File
 *  \author Elyes Ben Hamida (QMIC)
 *  \date   March 2015
 **/

#ifndef  __routing_rreq_management__
#define  __routing_rreq_management__


/** \brief Callback function for the RREQ packet dissemination in AODV (to be used with the scheduler_add_callback function).
 *  \fn int rreq_propagation_callback(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int rreq_propagation_callback(call_t *to, call_t *from, void *args);

/** \brief Callback function for the periodic generation of RREQ packets in AODV (to be used with the scheduler_add_callback function).
 *  \fn int rreq_periodic_generation_callback(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int rreq_periodic_generation_callback(call_t *to, call_t *from, void *args);

/** \brief Function to forward RREQ  packet towards the destination (AODV)
 *  \fn int rreq_propagation(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int rreq_propagation(call_t *to, call_t *from, void *args);


/** \brief Function to check if RREQ  packet has already been sent (AODV)
 *  \fn int rreq_table_lookup(call_t *to, int src, int dst, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param src is the source ID
 *  \param dst is the destination ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of RREQ
 *  \return 0 if success, -1 otherwise
 **/
int rreq_table_lookup(call_t *to, int src, int dst, int data_type, int seq);


/** \brief Function to update the local RREQ table to avoid the transmission of duplicate RREQ (AODV)
 *  \fn void rreq_table_update(call_t *to, int src, int dst, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param src is the source ID
 *  \param dst is the destination ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of RREQ
 **/
void rreq_table_update(call_t *to, int src, int dst, int data_type, int seq);



/** \brief Function to check if RREP packet has already been sent (AODV)
 *  \fn int rrep_table_lookup(call_t *to, int src, int dst, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param src is the source ID
 *  \param dst is the destination ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of RREQ
 *  \return 0 if success, -1 otherwise
 **/
int rrep_table_lookup(call_t *to, int src, int dst, int data_type, int seq);


/** \brief Function to update the local RREP table to avoid the transmission of duplicate RREP (AODV)
 *  \fn void rrep_table_update(call_t *to, int src, int dst, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param src is the source ID
 *  \param dst is the destination ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of RREQ
 **/
void rrep_table_update(call_t *to, int src, int dst, int data_type, int seq);


/** \brief Function for the transmission of RREP packet to the source node (AODV) => to be done by the sink node
 *  \fn int rrep_transmission(call_t *to, struct rreq_packet_header *rreq_header)
 *  \param c is a pointer to the called entity
 *  \param rreq_header is a pointer to the received RREQ packet
 *  \return 0 if success, -1 otherwise
 **/
int rrep_transmission(call_t *to, call_t *from, struct rreq_packet_header *rreq_header);


/** \brief Function for the transmission of RREP packet to the source node (AODV) => to be done by intermediate sensor node
 *  \fn int rrep_transmission_from_sensor(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the function argument
 *  \return 0 if success, -1 otherwise
 **/
int rrep_transmission_from_sensor(call_t *to, call_t *from, void *args);


#endif




