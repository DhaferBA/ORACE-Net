/**
 *  \file   routing_sink_interest_management.h
 *  \brief  Directed Diffusion Interest Dissemination Management Header File
 *  \author Elyes Ben Hamida (QMIC)
 *  \date   March 2015
 **/
 
#ifndef  __routing_sink_interest_management__
#define  __routing_sink_interest_management__


/** \brief Callback function for the periodic dissemination of Interest packets in Directed Diffusion (to be used with the scheduler_add_callback function).
 *  \fn int sink_interest_propagation_callback(call_t *to, void *args)
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int sink_interest_propagation_callback(call_t *to, call_t *from, void *args);

 
/** \brief Function to propagate received Interest packets (Directed Diffusion)
 *  \fn int sink_interest_propagation(call_t *to, void *args);
 *  \param c is a pointer to the called entity
 *  \param args is a pointer to the arguments of the function
 *  \return 0 if success, -1 otherwise
 **/
int sink_interest_propagation(call_t *to, call_t *from, void *args);


/** \brief Function to check if Interest packet has already been sent (Directed Diffusion)
 *  \fn int sink_interest_table_lookup(call_t *to, int sink_id, int data_type, int seq);
 *  \param c is a pointer to the called entity
 *  \param sink_id is the sink ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of Interest
 *  \return 0 if success, -1 otherwise
 **/
int sink_interest_table_lookup(call_t *to, int sink_id, int data_type, int seq);


/** \brief Function to update the local Interest table to avoid the transmission of duplicate Interest (Directed Diffusion)
 *  \fn void sink_interest_table_update(call_t *to, int sink_id, int data_type, int seq)
 *  \param c is a pointer to the called entity
 *  \param sink_id is the sink ID
 *  \param data_type is the requested data type
 *  \param seq is the sequence number of Interest
 **/
void sink_interest_table_update(call_t *to, int sink_id, int data_type, int seq);


#endif 



