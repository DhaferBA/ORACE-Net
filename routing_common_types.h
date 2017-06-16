/**
 *  \file   routing_common_types.h
 *  \brief  Common Types for all Routing Protocols
 *  \author Dhafer Ben Arbia & Elyes Ben Hamida
 *  \date   March 2015
 **/
 
 
#ifndef  __routing_common_types__
#define __routing_common_types__

/* Macro definitions for stats log outputs.*/
#define ROUTING_LOG_CLASS_STATS 1
#define ROUTING_LOG_NODE_STATS 1

/* Macro definitions for log outputs.*/
#define ROUTING_DEBUG 1
#define ROUTING_LOG_DATA_TX 1
#define ROUTING_LOG_DATA_RX 1
#define ROUTING_LOG_RX 1
#define ROUTING_RREQ_GENERATION 1
#define ROUTING_LOG_HELLO 1
#define ROUTING_LOG_DATA_FORWARDING 1
#define ROUTING_LOG_RREP_PROPAGATION 1
#define ROUTING_LOG_ROUTES 1
#define LOCALG_LOG_INTEREST_PROPAGATION 1

/* Macro definitions for nodes types.*/
#define SINK_NODE 0
#define SENSOR_NODE 1
#define ANCHOR_NODE 2

/* Macro definitions for protocol states.*/
#define STATUS_ON 1
#define STATUS_OFF 0

/* Macro definitions for packet types.*/
#define HELLO_PACKET               0
#define SINK_INTEREST_PACKET       1
#define DATA_PACKET                2
#define RREQ_PACKET                3
#define RREP_PACKET                4
#define TC_PACKET                  5
#define RE_PACKET		   6 		// Route Explore Packet: Added for ORACE-NET Protocol
#define ADVERT_PACKET		   7 		// Route Explore Packet: Added for ORACE-NET Protocol

/* Global Macro definitions.*/

#define MAX_NEIGHBORS_SIZE         1000

/* UPDATED by Dhafer 01-05-2015 */

/* ************************************************** */
/* ************************************************** */
/** \brief A structure containing the GLOBAL node entity variables and parameters
 *  \struct classdata
 **/
struct classdata {
  /* current values */
  int current_tx_control_packet;              	/*!< Defines the current total number of transmitted packets. */
  int current_rx_control_packet;              	/*!< Defines the current total number of received packets. */
  int current_tx_control_packet_bytes;    	/*!< Defines the current total amount of transmitted packets (bytes). */
  int current_rx_control_packet_bytes;   	/*!< Defines the current total amount of received packets (bytes). */
  /* final values */
  double global_establishment_time;     	/*!< Defines the global path establishment time from all the sensors to the sink (ms). */
  int global_tx_control_packet;              	/*!< Defines the total number of transmitted packets. */
  int global_rx_control_packet;              	/*!< Defines the total number of received packets. */
  int global_tx_control_packet_bytes;    	/*!< Defines the total amount of transmitted packets (bytes). */
  int global_rx_control_packet_bytes;   	/*!< Defines the total amount of received packets (bytes). */
};
 
 
/** \brief A structure containing the LOCAL node variables and parameters
 *  \struct nodedata
 **/
struct nodedata {
  
  /* Local variables at the node */
  int node_type;                              	/*!< Defines the node type. */
  position_t node_position;                   	/*!< Defines the node position. */
  void *neighbors;                            	/*!< Defines the local node neighbor table. */
  void *routing_table;                        	/*!< Defines the local node route table. */
  void *interest_table;                       	/*!< Defines the local node interest packet table. */

  int MPR_set[MAX_NEIGHBORS_SIZE];		/*! MPRs nodes : Updated by Dhafer BEN ARBIA 2-5-2015 */
  int olsr_path[MAX_NEIGHBORS_SIZE]; 		/*< Whole route from src to dst >*/
  int olsr_path_index;

  int neighbors_2hops[MAX_NEIGHBORS_SIZE][MAX_NEIGHBORS_SIZE]; /* Matrix containing 2 hop neighbors for each node */
  int neighbors_2hops_nbr[MAX_NEIGHBORS_SIZE][2];  	/* Matrix containing number of 2 hop neighbors for each node */

  int neighbor_mprs[MAX_NEIGHBORS_SIZE]; 	/* Matrix containing MPRs of each node */
  int tc_1st_hop_neighbors[MAX_NEIGHBORS_SIZE];

  int topology_matrix[MAX_NEIGHBORS_SIZE][MAX_NEIGHBORS_SIZE];  /* Connectivity Matrix */

  void *rreq_table;                           	/*!< Defines the local node RREQ packet table. */
  void *rrep_table;                           	/*!< Defines the local node RREP packet table. */
  int overhead;                               	/*!< Defines the total size of the lower-layer protocol headers. */
  int sink_id;                                	/*!< Defines the ID the sink (necessary for AODV). */

  /* Parameters related to the real sizes of packets  */
  int hello_packet_real_size;              	/*!< Defines the real size of a hello packet (in bytes). */
  int interest_packet_real_size;          	/*!< Defines the real size of an interest packet (in bytes). */
  int rreq_packet_real_size;               	/*!< Defines the real size of a RREQ packet (in bytes). */
  int rrep_packet_real_size;               	/*!< Defines the real size of a RREP packet (in bytes). */
  
  /* Parameters of the HELLO protocol  */
  int hello_status;                            	/*!< Defines the status of the hello protocol. */
  int hello_nbr;                               	/*!< Defines the maximal number of hello packets to be transmitted. */
  uint64_t hello_start;                         /*!< Defines the startup time of the hello protocol. */
  uint64_t hello_period;                       	/*!< Defines the periodicity of the hello packet transmission. */
  uint64_t hello_timeout;                     	/*!< Defines the timeout related to the manegement of the local neighbors table. */
  uint64_t previous_hello_slot_time;
/* UPDATED By Dhafer BEN ARBIA 2-5-2015 */ 
/* Parameters of the TC packet  */
  int tc_packet_real_size;
  int tc_status;                               	/*!< Defines the status of the TC packet. */
  int tc_nbr;                                   /*!< Defines the maximal number of TC packets to be transmitted. */
  uint64_t tc_start;                         	/*!< Defines the startup time of the TC packet. */
  uint64_t tc_period;                       	/*!< Defines the periodicity of the TC packet transmission. */
  uint64_t tc_timeout;                     	/*!< Defines the timeout related to the management of the local neighbors table. */
  uint64_t previous_tc_slot_time;
  int tc_seq;
  int tc_cache[MAX_NEIGHBORS_SIZE];

  /* Parameters of the SINK interest propagation protocol  */
  int sink_interest_status;                     /*!< Defines the status of the interest dissemination protocol. */
  double sink_interest_propagation_probability; /*!< Defines the probability of the interest dissemination protocol. */
  uint64_t sink_interest_propagation_backoff;  	/*!< Defines the backoff of the interest dissemination protocol. */
  int sink_interest_nbr;                       	/*!< Defines the maximal number of interest packets to be transmitted. */
  int sink_interest_seq;                       	/*!< Defines the sequence of the interest packets. */
  uint64_t sink_interest_start;                	/*!< Defines the startup time of the interest dissemination protocol. */
  uint64_t sink_interest_period;               	/*!< Defines the periodicity of the interest dissemination protocol. */
  int sink_interest_ttl;                       	/*!< Defines the TTL for the dissemination of interest packets. */
  int sink_interest_data_type;                 	/*!< Defines the requested data types of the interest packets. */
  uint64_t previous_sink_interest_slot_time;
 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/* Parameters of ORACE-NET protocol  */

  /* Parameters of the Advertisment protocol  */
  int advert_status;                            	/*!< Defines the status of the hello protocol. */
  int advert_nbr;                               	/*!< Defines the maximal number of hello packets to be transmitted. */
  uint64_t advert_start;                         	/*!< Defines the startup time of the hello protocol. */
  uint64_t advert_period;                       	/*!< Defines the periodicity of the hello packet transmission. */
  uint64_t advert_timeout;                     		/*!< Defines the timeout related to the manegement of the local neighbors table. */
  uint64_t previous_advert_slot_time;
  int nbr_received_hello[MAX_NEIGHBORS_SIZE]; 		/* nbr of received hello packet / node */
  float prr[MAX_NEIGHBORS_SIZE];              		/* PRR estimate -> Link quality */
  uint64_t last_tx_time; 				/* Needed to optimize Hello Bcast when a Data_Tx or ADV_Tx is done in that period*/
  int delayed_hello;					/* Used to calculate LQE parameters */
  int expected_hello;					/* LQE for ORACENET Based on Time */
  int data_seq;
  int last_seq[MAX_NEIGHBORS_SIZE];
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  /* Parameters of the AODV protocol  */
  int rreq_status;                             	/*!< Defines the status of the RREQ based path discovery protocol. */
  int rreq_nbr;                                	/*!< Defines the maximal number of RREQ packets to be transmitted. */
  uint64_t rreq_start;                         	/*!< Defines the startup time of the RREQ protocol. */
  uint64_t rreq_period;                        	/*!< Defines the periodicity of the RREQ protocol. */
  double rreq_propagation_probability;         	/*!< Defines the probability of the RREQ propagation protocol. */
  uint64_t rreq_propagation_backoff;           	/*!< Defines the backoff of the RREQ propagation protocol. */
  uint64_t rrep_propagation_backoff;           	/*!< Defines the probability of the RREP dissemination protocol. */
  int rreq_seq;                                	/*!< Defines the sequence number of RREQ packets. */
  int rreq_ttl;                                	/*!< Defines the TTL number of RREQ packets. */
  int rrep_seq;                                	/*!< Defines the sequence number of RREP packets. */
  int rreq_data_type;                          	/*!< Defines the data types requested by RREQ packets. */
  uint64_t previous_rreq_slot_time;
  
  /* Parameters of the link quality estimation (LQE) */
  int rssi_smoothing1_nbr;                     	/*!< Defines the nbr of required measurements to smooth RSSI values (1). */
  int rssi_smoothing2_nbr;                     	/*!< Defines the nbr of required measurements to smooth RSSI values (2). */
  int rssi_smoothing_factor;                   	/*!< Defines the factor for the smoothing of RSSI measurements. */
  int lqe_w;                                   	/*!< Defines the history size of LQE algorithm. */
  double lqe_threshold;                        	/*!< Defines the threshold for selection of the most reliable radio links. */
  
  /* Local variable for stats */
  int rx_nbr[5];    				/*!< Defines the number of received packets. */
  int tx_nbr[5];                               	/*!< Defines the number of transmitted packets. */
  int data_packet_size;                        	/*!< Defines the size of data packets. */
  double path_establishment_delay;             	/*!< Defines the path establishment delay (ms). */
  uint64_t first_rreq_startup_time;            	/*!< Defines the startup time of the first RREQ transmission. */
};

/** \brief A structure for the neighbor table management
 *  \struct neighbor
 **/
struct neighbor {
  int id;                      	/*!<  Neighbor node ID */
  int type;                  	/*!<  Neighbor node type: Uni-directional, Bi-directional or MPR */
  position_t position;  	/*!<  Neighbor position (if known) */
  int hop_to_sink;      	/*!<  Number of hop to the sink */
  int *second_hop_ngbr;		/*! List of 2nd Hop neighbors.: Updated by Dhafer BEN ARBIA 2-5-2015 */
  int rx_nbr;            	/*!<  number of received hello packet */
  int loss_nbr;           	/*!<  number of lost hello packet */
  double lqe;             	/*!<  Estimated long-term link quality */
  double rssi;            	/*!<  Smoothed RSSI measurement */
  double rxdbm;        		/*!<  Smoothed RSS measurement */
  uint64_t time;        	/*!<  Time related to the last hello packet reception */
  double RSSI_gathering_time1;  /*!<  Time related to the RSSI gathering time (ms) (1) */
  double RSSI_gathering_time2;  /*!<  Time related to the RSSI gathering time (ms) (2) */
  int slot;                 	/*!<  Slot number related to the last hello paquet reception */
  int slot_init;               	/*!<  Slot number related to the first hello paquet reception */
  int neighbors_2hop[MAX_NEIGHBORS_SIZE]; /* Second hop neighbors */
  int neighbors_2hop_nbr; 		  /* Number of second hop neighbors */
  double prr;			/* LQE for ORACENET */

};

/** \brief A structure for the route table management (Directed Diffusion)
 *  \struct route
 **/
struct route {
  int dst;                   /*!< dst node ID */
  int end_dst;		/* route end dst node */
  int sink_id;             	/*!<  Sink node ID */
  int nexthop_id;       	/*!<  Nexthop node ID */
  double nexthop_prr;
  int nexthop_lqe;     		/*!<  Nexthop node LQE */
  int hop_to_sink;     		/*!<  Number of hop to the sink */
  int hop_to_dst;       /*!< Number of hop to the dst */
  uint64_t time;        	/*!<  Time related to the last route update */
  int seq;          /*!< Sequence number related to the last route update */
  int type;		/* Type of the route: Best PRR (1) or Min Nbr of ReTX(2) */
  int retx_nbr;		/* estimated nbr of retransmission through this route */
  double E2E_PRR;	/* used by ORACENET for the routing decision */
};

/** \brief A structure for the route table management (AODV)
 *  \struct route_aodv
 **/
struct route_aodv {
  int dst;                   /*!< dst node ID */
  int nexthop_id;        /*!< Nexthop node ID */
  int nexthop_lqe;      /*!< Nexthop node LQE */
  int hop_to_dst;       /*!< Number of hop to the dst */
  uint64_t time;         /*!< Time related to the last route update */
  int seq_rreq;          /*!< Sequence number related to the last route update */
  int seq_rrep;          /*!< Sequence number related to the last route update */
};

/** \brief A structure for the route table management (ORACENET)
 *  \struct route_aodv
 **/
struct route_oracenet {
  int sink_id; 		/* Sink ID */
  int dst;                   /*!< dst node ID */
  int end_dst;		/* route end dst node */
  int nexthop_id;        /*!< Nexthop node ID */
  int nexthop_lqe;      /*!< Nexthop node LQE */
  int hop_to_dst;       /*!< Number of hop to the dst */
  uint64_t time;         /*!< Time related to the last route update */
  int seq;          /*!< Sequence number related to the last route update */
};

/** \brief A structure for the route table management (AODV)
 *  \struct route_olsrv2
 **/
struct route_olsrv2 {
  int dst;                   /*!< dst node ID */
  int nexthop_id;        /*!< Nexthop node ID */
  int nexthop_lqe;      /*!< Nexthop node LQE */
  int hop_to_dst;       /*!< Number of hop to the dst */
  uint64_t time;         /*!< Time related to the last route update */
  int seq_rreq;          /*!< Sequence number related to the last route update */
  int seq_rrep;          /*!< Sequence number related to the last route update */
  int path[MAX_NEIGHBORS_SIZE]; /*< Whole route from src to dst >*/
};


/** \brief A structure for the interest propagation management (Directed Diffusion)
 *  \struct interest
 **/
struct interest {
  int sink_id;            /*!< Sink node ID */
  int data_type;        /*!< Sink data type*/
  int seq;                 /*!< Sequence number of the last interest received */
  uint64_t time;        /*!< Time related to the last interest update */
};

/** \brief A structure for the RREQ propagation management (AODV)
 *  \struct rreq
 **/
struct rreq {
  int src;             /*!<  src node ID */
  int dst;             /*!<  dst node ID */
  int data_type;   /*!<   data type*/
  int seq;            /*!<  Sequence number of the last RREQ received */
  uint64_t time;   /*!<  Time related to the last RREQ update */
};

/** \brief A structure for the RREP propagation management (AODV)
 *  \struct rrep
 **/
struct rrep {
  int src;              /*!<  src node ID */
  int dst;              /*!<  dst node ID */
  int data_type;    /*!<   data type*/
  int seq;             /*!<  Sequence number of the last RREQ received */
  uint64_t time;    /*!<  Time related to the last RREQ update */
};

/* ************************************************** */
/* ************************************************** */

/** \brief A structure defining the general header related to the localg_network_* simulation modules
 *  \struct packet_header
 **/
struct packet_header {
  int sink_id;
  int end_dst;
  int nexthop;
  int prevhop;				/*!<  Last sender of the packet */
  nodeid_t src;                     	/*!<  source node ID */
  nodeid_t dst;                     	/*!<  destination node ID */
  int type;                            	/*!<  source node type */
  int packet_type;                	/*!<  packet type (hello, interest, RREQ, RREP, DATA) */
  int olsr_path[MAX_NEIGHBORS_SIZE];    /*< Whole route from src to dst >*/
  int olsr_path_index;
  int hop;
/* LQE Header fields */
  int seq;                  		/*!< Packet sequence number */
  int hop_to_dst;  			/*!< nbr of hops to the destination */
  double lqe;				/*!< Long Term Link Quality Estimation */
  double retx_nbr;			/*!< Nbr of Retransmissions */
  double E2E_PRR;			/*!< End to End Packet Reception Rate */
  double E2E_ReTx;			/*!< End to End Number of Retranmissions */
};

/** \brief A structure defining the header of hello packets
 *  \struct hello_packet_header
 **/
struct hello_packet_header {
  int sink_id;              		/*!< Closest sink ID */
  int hop_to_sink;       		/*!< Number of hop towards the SINK node */ 
  position_t position;   		/*!< Neighbor position (if known) */
  int first_hop_neighbors[MAX_NEIGHBORS_SIZE]; 		/*!< List of boradcasted 1 hops neighbors : UDATED By Dhafer BEN ARBIA 3-5-2015 */
  int link_type;		  /*!< Neighbor link type : Uni-directional, Bi-directional or MPR : UDATED By Dhafer BEN ARBIA 3-5-2015*/
  int hop;
/* LQE Header fields */
  int retx; 				/* retreansmission number */
  int prr;				/* PRR: */
};	


/** \brief A structure defining the header of hello packets
 *  \struct tc_packet_header
 **/
struct tc_packet_header {
  int dst;
  int seq;						/* Seq number used for tc packet processing */
  int first_hop_neighbors[MAX_NEIGHBORS_SIZE]; 		/*!< List of boradcasted 1 hops neighbors : UDATED By Dhafer BEN ARBIA 3-5-2015 */
  int mpr[MAX_NEIGHBORS_SIZE]; 		/*!< List of MPRs : UDATED By Dhafer BEN ARBIA 24-5-2015 */
  int link_type;		  /*!< Neighbor link type : Uni-directional, Bi-directional or MPR : UDATED By Dhafer BEN ARBIA 3-5-2015*/
  int tc_route[MAX_NEIGHBORS_SIZE];
};


/** \brief A structure defining the header of interest packets
 *  \struct sink_interest_packet_header
 **/
struct sink_interest_packet_header {
  int sink_id;             /*!< Closest sink ID */
  int seq;                  /*!< Packet sequence number */
  int ttl;                    /*!< Time-To-Live to limit packet propagation */
  int ttl_max;            /*!< Initial Time-To-Live to limit packet propagation */
  int data_type;         /*!< Requested Data Type (e.g. RSSI measurements, temperature values, etc.) */
  uint64_t time;         /*!< TX time instant */
  position_t position;  /*!< Neighbor position (if known) */
};

/** \brief A structure defining the header of RREQ packets
 *  \struct rreq_packet_header
 **/
struct rreq_packet_header {
  int dst;                   /*!<  Destination node ID */
  int src;                   /*!<  Source node ID */
  int seq;                  /*!<  Packet sequence number */
  int ttl;                    /*!<  Time-To-Live to limit packet propagation */
  int ttl_max;            /*!<  Initial Time-To-Live to limit packet propagation */
  int data_type;         /*!<  Requested Data Type (e.g. RSSI measurements, temperature values, etc.) */
  position_t position;  /*!<  Neighbor position (if known) */
};

/** \brief A structure defining the header of RREP packets
 *  \struct rrep_packet_header
 **/
struct rrep_packet_header {
  int dst;                   /*!< Destination node ID */
  int src;                   /*!< Source node ID */
  int seq;                  /*!< Packet sequence number */
  int seq_rreq;           /*!< Sequence number of RREQ */
  int hop_to_dst;  /*!< nbr of hops to the destination */
  position_t position;  /*!< Neighbor position (if known) */
};

#endif 
