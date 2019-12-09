/**********************************/
/*                                */
/* Functions working with packets */
/*                                */
/**********************************/

#ifndef PACKET_H
#define PACKET_H 1

#include "mcp_pi.h"
#include "queue.h"

/* is a complete packet in the in_queue */
BOOL is_packet_available(Queue *queue);

/* skip to the start of a new packet */
BOOL queue_skip_to_next_packet(Queue *queue);

/* skip the terminator of a packet */
void queue_skip_terminator_of_packet(Queue *queue);

/* get the PACKET_TYPE */
PacketType get_packet_type(Queue *queue);

/* calculate the packet size */
int get_packet_size(Queue *queue);

/* write the data provided into a packet */
BOOL write_data_to_packet(Queue *queue, BYTE *data, int size_of_data, PacketType packet_type);
    
/* check if the packet is terminated with '---' */
BOOL check_packet_for_terminator(Queue *queue, int packet_size);

/* start the build of a data packet */
BOOL start_build_data_packet(Queue *queue, int payload_size, PacketType packet_type);

/* add data to the payload of the packet */
void add_data_to_packet(Queue *queue, BYTE *data, int data_size);

/* finish the packet and send it to the queue */
void finish_build_data_packet_and_send(Queue *queue);

/* try and read the number of bytes from the packet - which should be the pay load */
BOOL get_data_from_packet(Queue *queue, BYTE *buffer_of_alloc_data, int page_size);
    
#endif