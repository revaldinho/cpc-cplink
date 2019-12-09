/********************************/
/*                              */
/* The definition of a queue    */
/*                              */
/********************************/

#ifndef QUEUE_H
#define QUEUE_H 1

/* size of the data for each queue */
#define QUEUE_SIZE 131070

/* typedefs which the compiler does not seem to have for some weird reason */
typedef unsigned char BYTE;
typedef unsigned char BOOL;

#define FALSE 0
#define TRUE 1

typedef struct Queue 
{
	BYTE data[QUEUE_SIZE];
	
	int store_next_byte;
	int read_next_byte;
		
} Queue;

/* init_queue */
void init_queue(Queue *queue);

/* reset the provided queues */
void queues_reset(Queue *in_queue, Queue *out_queue);

/* this function will read available data from the queue */
/* return value is TRUE if data from queue into *data    */
/* but only if number_of_bytes_required is available     */
BOOL get_data_from_queue(Queue *queue, BYTE *data, int number_of_bytes_required);

/* this function will try and write data to the queue    */
/* return value is TRUE if data is written to the queue  */
BOOL write_data_to_queue(Queue *queue, BYTE *data, int size_of_data);

/* is there space in the queue for x bytes */
BOOL is_queue_space_available(Queue *queue, int space_required);

/* get the current byte read_next_byte is pointing at */
BYTE get_current_read_byte(Queue *queue);

/* read ahead into the queue by x bytes (but do not change read_next_byte) */
BYTE queue_read_ahead_byte(Queue *queue, int bytes_ahead_to_read);

/* store to the current byte write_next_byte is pointing at */
void store_to_current_write_byte(Queue *queue, BYTE value_to_store);

/* get the previous byte read_next_byte is pointing at */
BYTE get_previous_read_byte(Queue *queue);

/* function increments read_next_byte by the number_bytes and takes into account queue loop around */
void increment_read_next_index(Queue *queue, int number_bytes);

/* function increments store_next_byte by the number_bytes and takes into account queue loop around */
void increment_store_next_index(Queue *queue, int number_bytes);

/* number of bytes to read in the queue */
int number_bytes_in_queue_to_read(Queue *queue);

/* number of free bytes to write too in the queue (without going over unread bytes) */
int number_free_bytes_in_queue_to_write_too(Queue *queue);

/* get the next three bytes from the queue                       */
/* (reading into queue but NOT changing the read_next_byte index */
BOOL get_next_three_bytes_from_queue(Queue *queue, BYTE *buffer);

/* will print sample of in_queue to the console */
void print_in_queue(int index_start, int number_bytes_to_display);

/* will print sample of out_queue to the console */
void print_out_queue(int index_start, int number_bytes_to_display);

#endif
