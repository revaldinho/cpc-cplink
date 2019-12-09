/**********************************/
/*                                */
/* Functions working with packets */
/*                                */
/**********************************/

#include "packet.h"
#include <string.h>
#include <stdio.h>

int global_temp_queue_store_start = 0;
int global_temp_payload_size = 0;
PacketType global_temp_packet_type = UnknownPacket;

/* is a complete packet in the in_queue */
BOOL is_packet_available(Queue *queue)
{
  BOOL packet_found = FALSE;
  
  /**************************************************************************************/
  /* A packet has the format + + + 2byte_PACKET_SIZE one_byte_PACKET_TYPE PAYLOAD - - - */
  /* e.g. + + + 08 00 01 P I N G 0xA - - -                                              */
  /*                     |         |                                                    */
  /*                     -----------                                                    */
  /*                      PAYLOAD                                                       */
  /*                                                                                    */
  /* PACKET_SIZE is number of bytes between +++ and ---                                 */
  /* PACKET_TYPE is 1 byte of PacketType indicating what type of packet has arrived     */
  /* See mcp_pi.h PacketType for details                                                */
  /*                                                                                    */
  /* The +++ signify the start of a packet, the --- the end of the packet               */
  /**************************************************************************************/
  
  /* find start of packet */
  if(queue_skip_to_next_packet(queue))
  {
    /* +++ found so have start of next packet */
    int number_bytes_available_in_queue = number_bytes_in_queue_to_read(queue);

    /* do we have enough bytes in the queue to determine +++PACKET_SIZE */
    if(number_bytes_available_in_queue >= (FRONT_DELIMITER_OVERHEAD + FRONT_PACKET_OVERHEAD))
    {
      /* yes we do so read packet_size */
      int packet_size = get_packet_size(queue);

      /* are there enough bytes in queue for a '+++' <complete_packet> '---'*/
      if(number_bytes_available_in_queue >= (packet_size + PACKET_DELIMITER_OVERHEAD) )
      {
        /* yes then we have a full packet of data potentially */
        
        /* check we have a valid PACKET_TYPE */
        if(get_packet_type(queue) == UnknownPacket || 
           get_packet_type(queue) >= Max_PacketType )
        {
          /* unknown packet type - oops */
          
          if(TRON)
          {
            printf("Received Packet with bad PACKET_TYPE %d\n", get_packet_type(queue));
          }
          
          /* discard the whole packet */
          increment_read_next_index(queue, packet_size + PACKET_DELIMITER_OVERHEAD);
        }
        /* check we have a terminating '---' */
        else if(check_packet_for_terminator(queue, packet_size) == FALSE)
        {
          /* bad formed packet oops */
          
          if(TRON)
          {
            printf("Received Malform Packet with no terminator\n");
            
            /* print the packet data */
            print_in_queue(queue->read_next_byte, packet_size + PACKET_DELIMITER_OVERHEAD);
          }
          
          /* discard the whole packet */
          increment_read_next_index(queue, packet_size + PACKET_DELIMITER_OVERHEAD);
        }
        else
        {
          packet_found = TRUE;
          
          if(TRON)
          {
            /* print the packet data */
            printf("Received Packet\n");
            print_in_queue(queue->read_next_byte, packet_size + PACKET_DELIMITER_OVERHEAD);
          }
        }
      }
      else
      {
        /* there are not enough bytes in the queue to complete the packet - so do nothing and wait for more data */
      }
    }
    else
    {
      /* not enough bytes to read the start of the packet header i.e. +++ and 2 byte PACKET_SIZE */
    }
  }
  else /* +++ not found in the available queue data */
  {
    /* do nothing and wait for more data to arrive */
  }

  return( packet_found );
}

/* skip to the start of a new packet */
BOOL queue_skip_to_next_packet(Queue *queue)
{
  BOOL did_we_find_packet_start = FALSE;  
  BYTE next_three_bytes_buffer[4];
   
  memset(next_three_bytes_buffer, 0, 4);

  /* get the next three bytes in the buffer */
  if(get_next_three_bytes_from_queue(queue, next_three_bytes_buffer))
  {
    #ifdef DEBUG_MCP_2
      printf("Looking for start next packet at read_index %d - ", queue->read_next_byte);
      print_in_queue(queue->read_next_byte, 20);
    #endif
    
    /* we got three bytes from the buffer */
    while(memcmp(next_three_bytes_buffer, "+++", FRONT_PACKET_OVERHEAD) != 0)
    {
      /* move on a byte in the queue to look at the next 3 bytes */
      increment_read_next_index(queue, 1);

      memset(next_three_bytes_buffer, 0, 4);
      
      /* get the next three bytes in the buffer */
      if(get_next_three_bytes_from_queue(queue, next_three_bytes_buffer) == FALSE)
      {
        /* didn't get three bytes so exit */
        return( FALSE );        
      }
    }
    
    /* if we did find +++ */
    if(memcmp(next_three_bytes_buffer, "+++", FRONT_PACKET_OVERHEAD) == 0)
    {
      /* then we think we have found start of packet */
      did_we_find_packet_start = TRUE;
    }
  }

  return(did_we_find_packet_start);
}

/* skip the terminator of a packet */
void queue_skip_terminator_of_packet(Queue *queue)
{
  BYTE next_three_bytes_buffer[4];
   
  memset(next_three_bytes_buffer, 0, 4);

  /* get the next three bytes in the buffer */
  if(get_next_three_bytes_from_queue(queue, next_three_bytes_buffer))
  {
    if(memcmp(next_three_bytes_buffer, "---", REAR_DELIMITER_OVERHEAD) == 0)
    {
      increment_read_next_index(queue, REAR_DELIMITER_OVERHEAD);
    }
  }
}

/* get the PACKET_TYPE */
PacketType get_packet_type(Queue *queue)
{
  /* +5 from start of packet to get PACKET_TYPE */
  return( queue->data[(queue->read_next_byte + 5) % QUEUE_SIZE] );
}

/* calculate the packet size */
int get_packet_size(Queue *queue)
{
  /* +3 and +4 from start of packet are the PACKET_SIZE bytes */
  int byte1 = queue->data[(queue->read_next_byte + 3) % QUEUE_SIZE];
  int byte2 = queue->data[(queue->read_next_byte + 4) % QUEUE_SIZE];

  return(byte1 + (byte2 * 256));
}

/* write the data provided into a packet */
BOOL write_data_to_packet(Queue *queue, BYTE *data, int size_of_data, PacketType packet_type)
{
  BOOL write_to_queue_ok = FALSE;
  
  if(TRON)
  {
     global_temp_queue_store_start = queue->store_next_byte;
     printf("StartWrite to OutQueue (size %d) packet (type %d): read: %d write: %d\n", QUEUE_SIZE, packet_type, queue->read_next_byte, queue->store_next_byte);
  }

  /* is there space in the out queue for the data - should be!! */
  if(is_queue_space_available(queue, size_of_data + PACKET_OVERHEAD))
  {
    int packet_size = size_of_data + FRONT_PACKET_OVERHEAD;
    int packet_size_lower = packet_size & 0xff;
    int packet_size_upper = (packet_size >> 8) & 0xff;
    
    /* take the data and build a packet for that data and write to the out_queue */
    
    /* write the leading delimiter */
    write_data_to_queue(queue, (BYTE *) "+++", 3);
    
    /* write the PACKET_SIZE out */
    write_data_to_queue(queue, (BYTE *) &packet_size_lower, 1);
    write_data_to_queue(queue, (BYTE *) &packet_size_upper, 1);
    
    /* write the PACKET_TYPE out */
    write_data_to_queue(queue, (BYTE *) &packet_type, 1);
    
    /* write the PACKET_DATA out */
    write_data_to_queue(queue, data, size_of_data);
    
    /* write the trailing delimiter */
    write_data_to_queue(queue, (BYTE *) "---", 3);
    
    write_to_queue_ok = TRUE;
  }
  
  if(TRON)
  {
    printf("EndWrite to OutQueue (size %d) packet (type %d): read: %d write: %d\n", QUEUE_SIZE, packet_type, queue->read_next_byte, queue->store_next_byte);  
    printf("Wrote Out Packet to ");
    print_out_queue(global_temp_queue_store_start, size_of_data + PACKET_OVERHEAD);
    
    global_temp_queue_store_start = 0;
  }
  
  return( write_to_queue_ok );
}

/* check if the packet is terminated with '---' */
BOOL check_packet_for_terminator(Queue *queue, int packet_size)
{
  BOOL found_terminator = FALSE;

  BYTE byte1 = queue_read_ahead_byte(queue, FRONT_DELIMITER_OVERHEAD + packet_size);
  BYTE byte2 = queue_read_ahead_byte(queue, FRONT_DELIMITER_OVERHEAD + packet_size + 1);
  BYTE byte3 = queue_read_ahead_byte(queue, FRONT_DELIMITER_OVERHEAD + packet_size + 2);
  
  if((byte1 == '-') && (byte2 == '-') && (byte3 == '-' ))
  {
    found_terminator = TRUE;
  }
                 
  return( found_terminator );
}

/* start the build of a data packet */
BOOL start_build_data_packet(Queue *queue, int payload_size, PacketType packet_type)
{
  BOOL space_to_build_packet_in_queue = FALSE;
  
  if(TRON)
  {
    global_temp_payload_size = payload_size + PACKET_OVERHEAD;
    global_temp_queue_store_start = queue->store_next_byte;
    global_temp_packet_type = packet_type;
    printf("StartWrite to OutQueue (size %d) packet (type %d): read: %d write: %d\n", QUEUE_SIZE, global_temp_packet_type, queue->read_next_byte, queue->store_next_byte);
  }
  
  /* is there space in the out queue for the data - should be!! */
  if(is_queue_space_available(queue, payload_size + PACKET_OVERHEAD))
  {
    int packet_size = payload_size + FRONT_PACKET_OVERHEAD;
    int packet_size_lower = packet_size & 0xff;
    int packet_size_upper = (packet_size >> 8) & 0xff;
    
    /* take the data and build a packet for that data and write to the out_queue */
    
    /* write the leading delimiter */
    write_data_to_queue(queue, (BYTE *) "+++", 3);
    
    /* write the PACKET_SIZE out */
    write_data_to_queue(queue, (BYTE *) &packet_size_lower, 1);
    write_data_to_queue(queue, (BYTE *) &packet_size_upper, 1);
    
    /* write the PACKET_TYPE out */
    write_data_to_queue(queue, (BYTE *) &packet_type, 1);
    
    space_to_build_packet_in_queue = TRUE;
  }
   
  return( space_to_build_packet_in_queue );
}

/* add data to the payload of the packet */
void add_data_to_packet(Queue *queue, BYTE *data, int data_size)
{
  /* write the data to the queue */
  write_data_to_queue(queue,  data, data_size);
}

/* finish the packet and send it to the queue */
void finish_build_data_packet_and_send(Queue *queue)
{
  /* write the terminator to the queue */
  write_data_to_queue(queue, (BYTE *) "---", 3);
  
  if(TRON)
  {
    printf("EndWrite to OutQueue (size %d) packet (type %d): read: %d write: %d\n", QUEUE_SIZE, global_temp_packet_type, queue->read_next_byte, queue->store_next_byte);  
    printf("Wrote Out Packet to ");
    print_out_queue(global_temp_queue_store_start, global_temp_payload_size);
    
    global_temp_queue_store_start = 0;
    global_temp_payload_size = 0;
    global_temp_packet_type = UnknownPacket;
  }
}

/* try and read the number of bytes from the packet - which should be the pay load */
BOOL get_data_from_packet(Queue *queue, BYTE *buffer_of_alloc_data, int page_size)
{
  BOOL read_data_ok = FALSE;
  
  int packet_body_size = get_packet_size(queue) - FRONT_PACKET_OVERHEAD;
  
  if(TRON)
  {
    printf("Read BinaryData of Size: %d\n", packet_body_size);
  }
  
  /* check the number of bytes being read (page_size) is less than or equal to packet_size */
  if(packet_body_size >= page_size)
  {
    /* there is enough data to read what we have been asked to read */
    
    if(TRON)
    {
      printf("Status at read start: Read %d Write %d\n", queue->read_next_byte, queue->store_next_byte);
    }

    /* skip over the packet front delimiter and packet header */
    increment_read_next_index(queue, FRONT_DELIMITER_OVERHEAD + FRONT_PACKET_OVERHEAD);      
    
    /* copy the page_size to the *data parameter */
    for(int index = 0; index < page_size; index++ )
    {
      buffer_of_alloc_data[index] = get_current_read_byte(queue);
      /* update read_next_byte pointer */
      increment_read_next_index(queue, 1);      
    }
    
    if(TRON)
    {
      printf("Status at read end: Read %d Write %d\n", queue->read_next_byte, queue->store_next_byte);
    }
    
    read_data_ok = TRUE;
  }
  else
  {
    printf("Not enough data in the packet for what we are looking for - error!!");
  }
  
  return( read_data_ok );
}