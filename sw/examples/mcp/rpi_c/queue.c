/*********************************/
/*                               */
/* The implementation of a queue */
/*                               */
/*********************************/
#include <string.h>
#include <stdio.h>

#include "queue.h"
#include "mcp_pi.h"

/* init_queue */
void init_queue(Queue *queue)
{
  /* this will clear the buffers and set mode to 'command' */
  memset(queue, 0, sizeof(Queue));
}

/* reset the provided queues */
void queues_reset(Queue *in_queue, Queue *out_queue)
{
  init_queue(in_queue);
  init_queue(out_queue);  
}

/* this function will read available data from the queue */
/* return value is TRUE if data from queue into *data    */
/* but only if number_of_bytes_required is available     */
BOOL get_data_from_queue(Queue *queue, BYTE *data, int number_of_bytes_required)
{
  BOOL got_all_data = FALSE;

  /* only if number_of_bytes_required is available in the queue will this command return TRUE */
  /* and copy the bytes into *data and update the read_next_byte pointer                      */
  
  /* there are more bytes available than we need */
  if(number_bytes_in_queue_to_read(queue) >= number_of_bytes_required)
  {
    /* there are enough bytes to read as requested */

    /* copy the number_of_bytes_required to the *data parameter */
    for(int index = 0; index < number_of_bytes_required; index++ )
    {
      data[index] = get_current_read_byte(queue);
      /* update read_next_byte pointer */
      increment_read_next_index(queue, 1);      
    }
        
    got_all_data = TRUE;
  }
  
  return( got_all_data );
}

/* this function will try and write data to the queue    */
/* return value is TRUE if data is written to the queue  */
BOOL write_data_to_queue(Queue *queue, BYTE *data, int size_of_data)
{
  #ifdef DEBUG_MCP_2
    printf("Write Start OUT Queue %d: read: %d write %d\n", QUEUE_SIZE, queue->read_next_byte, queue->store_next_byte);
  #endif

  BOOL wrote_all_data = FALSE;
  
  /* is there space to store the size_of_data data in the queue */
  if(number_free_bytes_in_queue_to_write_too(queue) >= size_of_data)
  {
    /* yes there is sufficient space */
    
    /* copy the size_of_data bytes from *data to the queue */
    for(int index = 0; index < size_of_data; index++ )
    {
      store_to_current_write_byte(queue, data[index]);
      /* update read_next_byte pointer */
      increment_store_next_index(queue, 1);      
    }
    
    wrote_all_data = TRUE;
  }
  
  #ifdef DEBUG_MCP_2
    printf("Write End OUT Queue %d: read: %d write %d\n", QUEUE_SIZE, queue->read_next_byte, queue->store_next_byte);
  #endif

  return( wrote_all_data );
}

/* is there space in the queue for x bytes */
BOOL is_queue_space_available(Queue *queue, int space_required)
{
  BOOL space_available = FALSE;
  
  if((queue->store_next_byte == queue->read_next_byte) && QUEUE_SIZE > space_required)
  {
    space_available = TRUE;
  }
  else if((((QUEUE_SIZE - queue->store_next_byte) + queue->read_next_byte) % QUEUE_SIZE) >= space_required)
  {
    space_available = TRUE;
  }
  
  return(space_available);
}

/* get the current byte read_next_byte is pointing at */
BYTE get_current_read_byte(Queue *queue)
{
  return(queue->data[queue->read_next_byte]);
}

/* read ahead into the queue by x bytes (but do not change read_next_byte) */
BYTE queue_read_ahead_byte(Queue *queue, int bytes_ahead_to_read)
{
  return(queue->data[(queue->read_next_byte + bytes_ahead_to_read) % QUEUE_SIZE]);
}

/* store to the current byte write_next_byte is pointing at */
void store_to_current_write_byte(Queue *queue, BYTE value_to_store)
{
  queue->data[queue->store_next_byte] = value_to_store;
}

/* get the previous byte read_next_byte is pointing at */
BYTE get_previous_read_byte(Queue *queue)
{
  int previous_byte_index = 0;
  
  if(queue->read_next_byte == 0)
  {
    previous_byte_index = QUEUE_SIZE - 1;
  }
  else
  {
    previous_byte_index = queue->read_next_byte - 1;
  }
  
  return(queue->data[previous_byte_index]);
}

/* function increments read_next_byte by the number_bytes and takes into account queue loop around */
void increment_read_next_index(Queue *queue, int number_bytes)
{
  queue->read_next_byte = (queue->read_next_byte + number_bytes) % QUEUE_SIZE;	
}

/* function increments store_next_byte by the number_bytes and takes into account queue loop around */
void increment_store_next_index(Queue *queue, int number_bytes)
{
  queue->store_next_byte = (queue->store_next_byte + number_bytes) % QUEUE_SIZE;	
}

/* number of bytes to read in the queue */
int number_bytes_in_queue_to_read(Queue *queue)
{
  return(((QUEUE_SIZE + queue->store_next_byte) - queue->read_next_byte) % QUEUE_SIZE);
}

/* number of free bytes to write too in the queue (without going over unread bytes) */
int number_free_bytes_in_queue_to_write_too(Queue *queue)
{
  int number_bytes_free = 0;
  
  if(queue->store_next_byte == queue->read_next_byte)
  {
    number_bytes_free = QUEUE_SIZE;
  }
  else
  {
    number_bytes_free = ((QUEUE_SIZE - queue->store_next_byte) + queue->read_next_byte) % QUEUE_SIZE;
  }

  return(number_bytes_free);
}

/* get the next three bytes from the queue                       */
/* (reading into queue but NOT changing the read_next_byte index */
BOOL get_next_three_bytes_from_queue(Queue *queue, BYTE *buffer)
{
  BOOL did_we_get_bytes = FALSE;
  int temp_read_index = queue->read_next_byte;
  
  /* check we are have three bytes to read */
  if(number_bytes_in_queue_to_read(queue) >= 3)
  {
    for(int index = 0; index < 3; index++)
    {
      buffer[index] = get_current_read_byte(queue);
      increment_read_next_index(queue,1);
    }

    /* restore read index to what it was as this function must not change it */
    queue->read_next_byte = temp_read_index;
    
    did_we_get_bytes = TRUE;
  }
  
  return(did_we_get_bytes);
}

/* print out the out_queue so you can see what is going on */
void print_out_queue(int index_start, int number_bytes_to_display)
{
  int temp_read_next_byte = out_queue.read_next_byte;
  
  /* set read from where we want to read from to output */
  out_queue.read_next_byte = index_start;
  
  printf("OutQueue: ");
  for(int x = 0; x < number_bytes_to_display; x++)
  {
    printf("%02d ", get_current_read_byte(&out_queue));
    increment_read_next_index(&out_queue, 1);
  }
  printf("\n");
  
  /* set queue read_next_byte back to what it was */
  out_queue.read_next_byte = temp_read_next_byte;  
}

/* print out the in_queue so you can see what is going on */
void print_in_queue(int index_start, int number_bytes_to_display)
{
  int temp_read_next_byte = in_queue.read_next_byte;
  
  /* set read from where we want to read from to output */
  in_queue.read_next_byte = index_start;
  
  printf("InQueue: ");
  for(int x = 0; x < number_bytes_to_display; x++)
  {
    printf("%02d ", get_current_read_byte(&in_queue));
    increment_read_next_index(&in_queue, 1);
  }
  printf("\n");
  
  /* set queue read_next_byte back to what it was */
  in_queue.read_next_byte = temp_read_next_byte;  
}