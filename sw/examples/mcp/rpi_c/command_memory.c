/********************************/
/*                              */
/* Memory API Command Functions */
/*                              */
/********************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "command_memory.h"
#include "memstore_manager.h"
#include "command.h"
#include "packet.h"

/*************************************************************************************/
/*                                                                                   */
/* These commands control moving pages of memory in/out of the CPC to/from the pi    */
/* Using allocation_id's you can have a number (default 20) different areas of       */
/* of memory allocated and you can request and store pages to each allocation        */
/* seperately. When you work with a memory allocation you work with RALLOC and       */
/* SALLOC and work with a page size as specified in the CALLOC command               */
/*                                                                                   */
/* CALLOC will create the number and size of pages requested in PI memory            */
/*       and return the allocation_id of that memory allocation                      */
/*       Any use of the other commands MUST have CALLOC performed first.             */
/*                                                                                   */
/* FALLOC will free the memory allocation created by a CALLOC.                       */
/*                                                                                   */
/* RALLOC is used by the CPC to fetch a page from the PI memory to its memory        */
/* The size of the page retrieved is what you specified in CALLOC                    */
/*                                                                                   */
/* SALLOC is used by the CPC to store a page from its memory to the PI memory        */
/* The size of the page to be stored is what you specified in CALLOC                 */
/*                                                                                   */
/* IALLOC is used to insert some memory at a specified point into the allocation     */
/*        on the pi                                                                  */
/*                                                                                   */
/* XALLOC is used to out a section of memory from the allocation in the pi memory    */
/*                                                                                   */
/* PILALLOC is used by the CPC to tell the PI to load a file into an allocation      */
/*          The CPC can then page the loaded file in and out of CPC memory from      */
/*          the PI's memory                                                          */
/*                                                                                   */
/* PISALLOC is used by the CPC to tell the PI to save an allocation its memory back  */
/*          to file on the PI storage                                                */
/*                                                                                   */
/* See each function for format of the messages                                      */
/*                                                                                   */
/*************************************************************************************/

BOOL command_create_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                   */
	/* CALLOC NUM_PAGES SIZE_OF_PAGES                       */
	/* CALLOC (1 byte) value 21                             */
    /* NUM_PAGEs (2 bytes) range 1 to 9999                  */
    /* SIZE_OF_PAGES_IN_KB (1 byte) range 1 to 32           */
	/* example:                                             */
    /* 21 05 00 08                                          */
    /* where 05 + (00 << 8) is 5 pages of 8KB               */
	/* format of response:                                  */
	/* CALLOC ALLOC_ID ERROR_ID                             */ 
    /* CALLOC 21 (1 byte)                                   */
    /* ALLOC_ID (1 byte) range 0 to 20                      */
    /* ALLOC_ID value 0 means error occured                 */
    /* ERROR_ID (1 byte) range 0 to 2                       */
	/* ERROR_ID value 0 no error occured                    */
	/* ERROR_ID value 1 when parameters are incorrect       */
	/* ERROR_ID value 2 when pi can't allocate memory       */
    /* example:                                             */
    /* 21 01 00                                             */
    /* where 01 is alloc_ID 1 allocated with 00 errors      */
	
	int alloc_page_number = get_command_next_16bit_param_bin(in_queue);
	int alloc_size_in_kb  = get_command_next_8bit_param_bin(in_queue);

	if(TRON)
	{
		printf("CALLOC Requested %d PAGES of %dKB\n", alloc_page_number, alloc_size_in_kb);
	}

	/* check page_size is 0 or larger than 32K */
	if((alloc_size_in_kb == 0) || (alloc_size_in_kb > 32))
	{
		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* add the error number - 1 as parameters are invalid */
		add_3_bytes_of_data_to_packet(out_queue, CreateAlloc, 0, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	/* check if page number is 0 or larger than 9999 */
	else if((alloc_page_number == 0) || (alloc_page_number > 9999))
	{
		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* add the error number - 1 as parameters are invalid */
		add_3_bytes_of_data_to_packet(out_queue, CreateAlloc, 0, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	else
	{
		/* create an allocation with the memstore manager for the number and size of pages */
		int allocation_id = memstore_create_allocation_id(alloc_page_number, alloc_size_in_kb);

		/* if we were unable to create an allocation (0 is invalid) */		
		if(allocation_id == 0)
		{
			/* failed to create an allocation of memory */

			/* start to build the 3 byte packet body response */
			start_build_data_packet(out_queue, 3, BinaryCommand);
			/* add the command id */
			/* add the alloc id return which is 0 cos its an error */
			/* add the error number - 2 pi unable to allocate memory */
			add_3_bytes_of_data_to_packet(out_queue, CreateAlloc, 0, 2);
			/* finish and send the packet to the out_queue */
			finish_build_data_packet_and_send(out_queue);
		}
		else /* we created the allocation id - inform the client */
		{
			/* start to build the 3 byte packet body response */
			start_build_data_packet(out_queue, 3, BinaryCommand);
			/* add the command id */
			/* add the alloc id returned as part of creating the allocation */
			/* add the error number - 0 as no error */
			add_3_bytes_of_data_to_packet(out_queue, CreateAlloc, allocation_id, 0);
			/* finish and send the packet to the out_queue */
			finish_build_data_packet_and_send(out_queue);
		}			
	}
		
	return( TRUE );
}

BOOL command_free_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                */
	/* FALLOC ALLOC_ID                                   */
	/* FALLOC (1 byte) value 22                          */
	/* ALLOC_ID (1 byte) range 1 to 20                   */
	/* example:                                          */
	/* 22 01                                             */
	/* where 01 is free alloc_id 01                      */
	/* format of response:                               */
	/* FALLOC ALLOC_ID ERROR_ID                          */ 
	/* FALLOC (1 byte) value 22                          */
	/* ALLOC_ID (1 byte) range 0 to 20                   */
    /* ALLOC_ID value 0 means error occured              */
    /* ALLOC_ID value 1 to 20 alloc_id freed ok          */
    /* ERROR_ID (1 byte) range 0 to 1                    */
	/* ERROR_ID value 0 no error occured                 */
	/* ERROR_ID value 1 when parameters are incorrect    */
    /* example:                                          */
    /* 22 01 00                                          */
    /* where 01 is alloc_ID 01 was freed with 00 errors  */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	int allocation_id = get_command_next_8bit_param_bin(in_queue);
	
	if(TRON)
	{
		printf("FALLOC Free ALLOC_ID %d\n", allocation_id);
	}

	/* if we can free the allocation */
	if(memstore_free_allocation_id( allocation_id ))
	{
		/* allocation was freed */

		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id returned as part of freeing the allocation */
		/* add the error number - 0 as no error */
		add_3_bytes_of_data_to_packet(out_queue, FreeAlloc, allocation_id, 0);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	else /* failed to free the allocation_id as the allocation_id from CPC was invalid */
	{
		/* send an error message back to the CPC */

		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* add the error number - 1 as parameters are invalid */
		add_3_bytes_of_data_to_packet(out_queue, FreeAlloc, 0, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	
	return( TRUE );
}

BOOL command_retrieve_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                */
	/* RALLOC ALLOC_ID PAGE_NUMBER                       */
	/* RALLOC (1 byte) value 23                          */
	/* ALLOC_ID (1 byte) range 1 to 20                   */
	/* PAGE_NUMBER (2 bytes lo/high) value 0 to 9999     */
	/* example:                                          */
	/* 23 01 05 00                                       */
	/* where 01 is ALLOC_ID 1                            */
	/* where 05 + (00 << 8) is the page number 5         */
	/* format of response:                               */
	/* RALLOC ALLOC_ID PAGE_NUMBER ERROR_ID              */
	/* RALLOC (1 byte) value 23                          */
	/* ALLOC_ID (1 byte) range 0 to 20                   */
    /* ALLOC_ID value 0 means error occured              */
    /* ALLOC_ID value 1 to 20 alloc_id retreiving ok     */
	/*          A BinaryData packet of PAGE_NUMBER data  */
	/*          from the allocation ALLOC_ID will follow */
	/*          this response packet.                    */
	/* PAGE_NUMBER (2 bytes lo/high) value 0 to 9999     */
	/*          The page number retrieved                */
    /* ERROR_ID (1 byte) range 0 to 1                    */
	/* ERROR_ID value 0 no error occured                 */
	/* ERROR_ID value 1 when parameters are incorrect    */
	/* example:                                          */
	/* 23 01 05 00 00                                    */
	/* Where retrieved page 5 of alloc_id 1 memory with  */
	/* no errors                                         */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	int allocation_id = get_command_next_8bit_param_bin(in_queue);	
	int alloc_page_number = get_command_next_16bit_param_bin(in_queue);

	if(TRON)
	{
		printf("RALLOC ALLOC_ID %d PAGE_NUMBER %d\n", allocation_id, alloc_page_number);
	}

	/* check if alloc page number is greater than 9999 */
	if(alloc_page_number > 9999)
	{
		/* start to build the 5 byte packet body response */
		start_build_data_packet(out_queue, 5, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* output the page number requested (low byte) */
		/* output the page number requested (high byte) */
		/* add the error number - 1 as parameters are invalid */
		add_5_bytes_of_data_to_packet(out_queue, RetrieveAlloc, 0, alloc_page_number % 256, alloc_page_number / 256, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	else
	{
		int bytes_to_retrieve = 0;
		BYTE *page_data_pointer = memstore_retrieve_page_of_data(allocation_id, alloc_page_number, &bytes_to_retrieve);			

		/* if we retrieved some data for that page from the allocation */		
		if(bytes_to_retrieve > 0)
		{			
			/* is there space in the out_queue to build the BinaryCommand packet response and the BinaryData packet response */
			if(is_queue_space_available(out_queue, 5 + PACKET_OVERHEAD +  bytes_to_retrieve + PACKET_OVERHEAD))
			{
				/* write the BinaryCommand response out first */
				/* start to build the 5 byte packet body response */
				start_build_data_packet(out_queue, 5, BinaryCommand);
				/* add the command id */
				/* add the alloc id of the alloc the page data is from */
				/* output the page number requested (low byte) */
				/* output the page number requested (high byte) */
				/* add the error number 0 as no error */
				add_5_bytes_of_data_to_packet(out_queue, RetrieveAlloc, allocation_id, alloc_page_number % 256, alloc_page_number / 256, 0);
				/* finish and send the packet to the out_queue */
				finish_build_data_packet_and_send(out_queue);

				/* now write the BinaryData response (containing the requested data) */
				write_data_to_packet(out_queue, page_data_pointer, bytes_to_retrieve, BinaryData);

				/* set the send lock mode i.e. do not process any more commands until the */
				/* response BinaryData above has been sent to the CPC to optimise sending */
				set_binary_data_send_lock_mode(bytes_to_retrieve + PACKET_OVERHEAD);
			}
			else
			{
				/* there is insufficient space to write out so return FALSE and try again later (when out_queue has space for example */
				return( FALSE );
			}
		}
		else /* failed to get the requested page from allocation_id */
		{
			/* send message back to the CPC that page for that allocation id does not exist */

			/* start to build the 5 byte packet body response */
			start_build_data_packet(out_queue, 5, BinaryCommand);
			/* add the command id */
			/* add the alloc id return which is 0 cos its an error */
			/* output the page number requested (low byte) */
			/* output the page number requested (high byte) */
			/* add the error number - 1 as parameters are invalid */
			add_5_bytes_of_data_to_packet(out_queue, RetrieveAlloc, 0, alloc_page_number % 256, alloc_page_number / 256, 1);
			/* finish and send the packet to the out_queue */
			finish_build_data_packet_and_send(out_queue);
		}
	}
	
	return( TRUE );
}

BOOL command_store_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                  */
	/* SALLOC ALLOC_ID PAGE_NUMBER                         */
	/* SALLOC (1 byte) value 24                            */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* PAGE_NUMBER (2 bytes lo/high) value 0 to 9999       */
	/* example:                                            */
	/* 24 01 02 00                                         */
	/* where 01 is ALLOC_ID 1                              */
	/* where 02 + (00 << 8) is the page number 2           */
    /*   BINARY_DATA follows in the next packet where the  */
    /*   length of BINARY_DATA is the page size used in    */       
    /*   CALLOC creation                                   */
	/* format of response:                                 */
	/* SALLOC ALLOC_ID PAGE_NUMBER ERROR_ID                */
	/* SALLOC (1 byte) value 24                            */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be stored        */
	/* PAGE_NUMBER (2 bytes lo/high) value 0 to 9999       */
	/*          The page where the BinaryData is to be     */
	/*          stored                                     */
    /* ERROR_ID (1 byte) range 0 to 1                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 unable to allocate memory on pi    */
	/* example:                                            */
	/* 24 01 02 00 00                                      */
	/* Where stored the following BinaryData packet        */
	/* at page 2 of alloc_id 1 memory with no errors       */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	BOOL skip_next_binary_packet = TRUE;

	int allocation_id = get_command_next_8bit_param_bin(in_queue);	
	int alloc_page_number = get_command_next_16bit_param_bin(in_queue);
	int page_size = memstore_get_page_size_bytes(allocation_id);

	BYTE *buffer_of_alloc_data = calloc(page_size + 1, 1);

	if(TRON)
	{
		printf("SALLOC ALLOC_ID %d PAGE_NUMBER %d\n", allocation_id, alloc_page_number);
	}

	/* skip the terminator of the BinaryCommand packet - ready to read the BinaryData packet */
	queue_skip_terminator_of_packet(in_queue);

	/* if we failed to allocate storage for the buffer */
	if(buffer_of_alloc_data == NULL)
	{
		/* unable to allocate memory on pi, send error to cpc */

		/* start to build the 5 byte packet body response */
		start_build_data_packet(out_queue, 5, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* output the page number requested (low byte) */
		/* output the page number requested (high byte) */
		/* add the error number - 2 unable to allocate memory on pi */
		add_5_bytes_of_data_to_packet(out_queue, StoreAlloc, 0, alloc_page_number % 256, alloc_page_number / 256, 2);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	/* if the page has no size then allocation_id is wrong */
	else if(page_size == 0)
	{
		/* send message back to the CPC as allocation_id is invalid as no page_size */

		/* start to build the 5 byte packet body response */
		start_build_data_packet(out_queue, 5, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* output the page number requested (low byte) */
		/* output the page number requested (high byte) */
		/* add the error number - 1 as parameters are invalid */
		add_5_bytes_of_data_to_packet(out_queue, StoreAlloc, 0, alloc_page_number % 256, alloc_page_number / 256, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	/* check if page number is greater than 9999 */
	else if(alloc_page_number > 9999)
	{
		/* send message back to the CPC as number pages is wrong */

		/* start to build the 5 byte packet body response */
		start_build_data_packet(out_queue, 5, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* output the page number requested (low byte) */
		/* output the page number requested (high byte) */
		/* add the error number - 1 as parameters are invalid */
		add_5_bytes_of_data_to_packet(out_queue, StoreAlloc, 0, alloc_page_number % 256, alloc_page_number / 256, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	/* is the next packet containing BinaryData completed in queue */
	/* i.e. fully received from cpc                                */
	else if(is_complete_packet_available(in_queue) == FALSE)
	{
		/* we do not have a complete BinaryData packet in the in_queue for that page */
		/* return with FALSE so this command will be retried once we have more data  */

		if(TRON)
		{
			printf("SALLOC binary data incomplete - waiting on more to be received and will retry\n");
		}

		/* set BinaryData receive mode - we want page_size plus PACKET_OVERHEAD bytes    */
		/* which is the BinaryData we are waiting for.  Until then do not process any    */
		/* more incoming commands - just keep reading and sending bytes in the main loop */
		set_binary_data_receive_lock_mode(page_size + PACKET_OVERHEAD);
					
		return( FALSE );
	}
	/* if we can read BinaryData packet with a page_size number of bytes from in_queue */
	else if(get_data_from_packet(in_queue, buffer_of_alloc_data, page_size) == FALSE)
	{
		/* the PACKET_SIZE and page_size do not match i.e. the BinaryData packet */
		/* contain too much or too little data to fit the page_size of the page  */

		if(TRON)
		{
			printf("SALLOC binary data size error, Expecting %d but got %d bytes\n", page_size, get_packet_size(in_queue));
		}
		
		skip_next_binary_packet = FALSE;
	}
	/* can we store the binary data */
	else if(memstore_store_page_of_data(allocation_id, alloc_page_number, buffer_of_alloc_data) == TRUE)
	{
		/* we stored the data ok */

		if(TRON)
		{
			printf("Wrote to memory store ALLOC_ID %d, PAGE_NUMBER %d\n", allocation_id, alloc_page_number);
		}

		/* start to build the 5 byte packet body response */
		start_build_data_packet(out_queue, 5, BinaryCommand);
		/* add the command id */
		/* add the alloc id return */
		/* output the page number requested (low byte) */
		/* output the page number requested (high byte) */
		/* add the error number - 0 as no error */
		add_5_bytes_of_data_to_packet(out_queue, StoreAlloc, allocation_id, alloc_page_number % 256, alloc_page_number / 256, 0);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
		
		skip_next_binary_packet = FALSE;
	}
	else /* failed to store the received BinaryData to the requested page in allocation_id memory */
	{
		/* send message back to the CPC that page for that allocation id does not exist */

		/* start to build the 5 byte packet body response */
		start_build_data_packet(out_queue, 5, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* output the page number requested (low byte) */
		/* output the page number requested (high byte) */
		/* add the error number - 1 as parameters are invalid */
		add_5_bytes_of_data_to_packet(out_queue, StoreAlloc, 0, alloc_page_number % 256, alloc_page_number / 256, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);

		skip_next_binary_packet = FALSE;
	}

	/* free allocated memory (if any was) */
	if(buffer_of_alloc_data)
	{
		free(buffer_of_alloc_data);
	}
	
	/* if we need to skip the binary packet as this BinaryCommand was in error */
	if(skip_next_binary_packet)
	{
		/* skip the next BinaryData packet as its aimed at this command */
		increment_read_next_index(in_queue, 1);
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}

BOOL command_insert_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                  */
	/* IALLOC ALLOC_ID INSERT_AT INSERT_LENGTH             */
	/* IALLOC (1 byte) value 25                            */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* INSERT_AT (4 byte) range 0 to sizeof(int)           */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */
	/* INSERT_LENGTH (4 byte) range 0 to sizeof(int)       */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */
	/* example:                                            */
	/* 25 01 00 00 02 00 00 00 10 00                       */
	/* where 01 is memory allocation 1                     */
	/* where 02 is insert at position 02 for 10 bytes      */
    /*   BINARY_DATA follows in the next packet where the  */
    /*   length of BINARY_DATA is the INSERT_LENGTH        */       
	/* format of response:                                 */
	/* IALLOC ALLOC_ID ERROR_ID                            */
	/* IALLOC (1 byte) value 25                            */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be modified      */
    /* ERROR_ID (1 byte) range 0 to 3                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 unable to allocate memory on pi    */
	/* ERROR_ID value 3 insert_at beyond end of allocation memory */
	/* example:                                            */
	/* 25 01 00                                            */
	/* Where inserted the following BinaryData packet      */
	/* into alloc_id 1 memory with no errors               */

	BOOL skip_next_binary_packet = TRUE;

	int allocation_id = get_command_next_8bit_param_bin(in_queue);	
	int alloc_insert_at = get_command_next_32bit_param_bin(in_queue);
	int alloc_insert_length = get_command_next_32bit_param_bin(in_queue);
	
	BYTE *buffer_of_alloc_data = calloc(alloc_insert_length + 1, 1);
	
	/* skip the terminator of the BinaryCommand packet - ready to read the BinaryData packet */
	queue_skip_terminator_of_packet(in_queue);
									
	if(TRON)
	{
		printf("IALLOC ALLOC_ID %d INSERT_AT %d INSERT_LENGTH %d\n", allocation_id, alloc_insert_at, alloc_insert_length);
	}

	/* if we failed to allocate storage for the buffer */
	if(buffer_of_alloc_data == NULL)
	{
		/* unable to allocate memory on pi, send error to cpc */

		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* add the error number - 2 as failed to allocate memory on pi */
		add_3_bytes_of_data_to_packet(out_queue, InsertAlloc, 0, 2);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	/* check if alloc_insert_at + alloc_insert_length is within allocation memory bounds */
	else if((alloc_insert_at + alloc_insert_length) > memstore_get_alloc_size_bytes(allocation_id))
	{
		/* outside memory allocation so return error to CPC */

		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* add the error number - 3 as insert beyond memory allocation on pi */
		add_3_bytes_of_data_to_packet(out_queue, InsertAlloc, 0, 3);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	/* is the next packet containing BinaryData completed in queue */
	/* i.e. fully received from cpc                                */
	else if(is_complete_packet_available(in_queue) == FALSE)
	{
		/* we do not have a complete BinaryData packet in the in_queue to insert     */
		/* return with FALSE so the command will be retried once we have more data   */

		if(TRON)
		{
			printf("IALLOC binary data incomplete - waiting on more to be received and will retry\n");
		}

		/* set BinaryData receive mode - we want alloc_insert_length plus PACKET_OVERHEAD bytes */
		/* which is the BinaryData in the next packet in the queue.  Until the full packet      */
		/* arrives do not process any more incoming commands - just keep reading and sending    */
		/* bytes in the main loop                                                               */
		set_binary_data_receive_lock_mode(alloc_insert_length + PACKET_OVERHEAD);
	
		return( FALSE );
	}
	/* if we fail to read a BinaryData packet with a page_size number of bytes from in_queue */
	else if(get_data_from_packet(in_queue, buffer_of_alloc_data, alloc_insert_length) == FALSE)
	{
		/* the PACKET_SIZE and page_size do not match i.e. the BinaryData packet */
		/* contain too much or too little data to fit the page_size of the page  */

		if(TRON)
		{
			printf("IALLOC binary data size error, Expecting %d but got %d bytes\n", alloc_insert_length, get_packet_size(in_queue));
		}
		
		skip_next_binary_packet = FALSE;
	}
	/* if we inserted the binary data into the allocation memory ok */
	else if(memstore_insert_data(allocation_id, alloc_insert_at, alloc_insert_length, buffer_of_alloc_data) == TRUE)
	{
		if(TRON)
		{
			printf("Wrote to memory store ALLOC_ID %d\n", allocation_id);
		}
		
		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id  */
		/* add the error number - 0 as no error */
		add_3_bytes_of_data_to_packet(out_queue, InsertAlloc, allocation_id, 0);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
		
		skip_next_binary_packet = FALSE;
	}
	else /* failed to store the insert data */
	{
		/* send message back to the CPC that allocation id does not exist */
        /* (which is why we failed to insert the data )                   */		

		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* add the error number - 1 bad params */
		add_3_bytes_of_data_to_packet(out_queue, InsertAlloc, 0, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
		
		skip_next_binary_packet = FALSE;
	}		
	
	/* free allocated memory (if any was) */
	if(buffer_of_alloc_data)
	{
		free(buffer_of_alloc_data);
	}
	
	/* if we need to skip the binary packet as this BinaryCommand was in error */
	if(skip_next_binary_packet)
	{
		/* skip the next BinaryData packet as its aimed at this command */
		increment_read_next_index(in_queue, 1);
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}

BOOL command_cut_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                  */
	/* XALLOC ALLOC_ID CUT_FROM CUT_LENGTH                 */
	/* XALLOC (1 byte) value 26                            */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* CUT_FROM (4 byte) range 0 to sizeof(int)            */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */
	/* CUT_LENGTH (4 byte) range 0 to sizeof(int)          */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */
	/* example:                                            */
	/* 26 01 00 00 08 00 00 00 10 00                       */
	/* where 01 is memory allocation 1                     */
	/* where 08 is cut from position 08 for 10 bytes       */
	/* format of response:                                 */
	/* XALLOC ALLOC_ID ERROR_ID                            */
	/* XALLOC (1 byte) value 26                            */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be modified      */
    /* ERROR_ID (1 byte) range 0 to 2                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 position+cut_length go beyond end  */
	/*         of allocation memory                        */
	/* example:                                            */
	/* 26 01 00                                            */
	/* Where the cut from memory succeeeded on allocation 1*/
	
	/* The purpose of this command is to cut out a defined section of memory in allocation at a specific point */
	
	int allocation_id = get_command_next_8bit_param_bin(in_queue);	
	int alloc_cut_from = get_command_next_32bit_param_bin(in_queue);
	int alloc_cut_length = get_command_next_32bit_param_bin(in_queue);	
	
	if(TRON)
	{
		printf("XALLOC ALLOC_ID %d CUT_FROM %d CUT_LENGTH %d\n", allocation_id, alloc_cut_from, alloc_cut_length);
	}

	/* check if alloc_cut_at + alloc_cut_length is within allocation memory bounds */
	if((alloc_cut_from + alloc_cut_length) > memstore_get_alloc_size_bytes(allocation_id))
	{
		/* outside memory allocation so return error to CPC */

		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* add the error number - 2 cut attempt outside allocated memory */
		add_3_bytes_of_data_to_packet(out_queue, CutAlloc, 0, 2);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	/* if we can cut the section of memory out from the allocation_id in memory store */
	else if(memstore_cut_data(allocation_id, alloc_cut_from, alloc_cut_length) == TRUE)
	{
		/* we did cut the data out from the memory allocation */

		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id  */
		/* add the error number - 0 - no error */
		add_3_bytes_of_data_to_packet(out_queue, CutAlloc, allocation_id, 0);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}
	else /* failed to cut the data from the allocation_id memory area */
	{
		/* send message back to the CPC that allocation id does not exist */

		/* start to build the 3 byte packet body response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* add the command id */
		/* add the alloc id return which is 0 cos its an error */
		/* add the error number - 1 parameter error */
		add_3_bytes_of_data_to_packet(out_queue, CutAlloc, 0, 1);
		/* finish and send the packet to the out_queue */
		finish_build_data_packet_and_send(out_queue);
	}		
	
	return( TRUE );
}

BOOL command_load_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                  */
	/* PILALLOC ALLOC_ID /PATH/TO/FILENAME\n               */
	/* PILALLOC (1 byte) value 27                          */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* /PATH/TO/FILENAME is a string filename path  with   */
	/* \n termination                                      */
	/* example:                                            */
	/* 27 01 <<FILENAME_IN_MEMORY_BYTES>> 10               */
	/* format of response:                                 */
	/* PILALLOC ALLOC_ID FILE_SIZE ERROR_ID                */
	/* PILALLOC (1 byte) value 27                          */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be modified      */
	/* FILE_SIZE (4 byte) range 0 to sizeof(int)           */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */
    /* ERROR_ID (1 byte) range 0 to 3                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 unable to load file                */
	/* ERROR_ID value 3 file to large to fit allocation    */
	/* example:                                            */
	/* 27 01 00 00 FF FE 00                                */
	/* Where the file of FFF bytes size was loaded        */
	/* successfully in to allocation 1 with no errors      */	

	int allocation_id = get_command_next_8bit_param_bin(in_queue);	
	char file_name_to_load[MAX_PATH_SIZE_TEXT];
	
	int file_size = 0;
	BOOL file_to_large = FALSE;

	/* clear the path name */
	memset(file_name_to_load, 0, MAX_PATH_SIZE_TEXT);

	/* if we failed to read the path/name of the file to load */
	if(get_all_command_params_text(in_queue, file_name_to_load, MAX_PATH_SIZE_TEXT) == FALSE)
	{
		/* we did not get the path/filename successfully */

		/* start the 7 byte packet response */
		start_build_data_packet(out_queue, 7, BinaryCommand);
		/* write the command id */
		add_byte_of_data_to_packet(out_queue, LoadAlloc);
		/* write the allocation id - 0 as we have an error */
		add_byte_of_data_to_packet(out_queue, 0);
		/* write the file size to the packet - 00 00 00 00 as we have an error */
		add_int_to_packet(out_queue, 0);
		/* write out the error found - 1 parameter error */
		add_byte_of_data_to_packet(out_queue, 1);
		/* finish the packet ready for sending */
		finish_build_data_packet_and_send(out_queue);
	}
	/* if we read the file into allocation memory */
	else if(memstore_load_data(allocation_id, file_name_to_load, &file_size, &file_to_large) == TRUE)
	{
		/* we did read the file into the allocation memory */

		if(TRON)
		{
			printf("PILALLOC FILE_PATH %s\n", file_name_to_load);
		}

		/* start the 7 byte packet response */
		start_build_data_packet(out_queue, 7, BinaryCommand);
		/* write the command id */
		add_byte_of_data_to_packet(out_queue, LoadAlloc);
		/* write the allocation id  */
		add_byte_of_data_to_packet(out_queue, allocation_id);
		/* write the file size to the packet format high16bit(lo/high) low16bit(lo/high) */
		add_int_to_packet(out_queue, file_size);
		/* write out the error found - 0 no error */
		add_byte_of_data_to_packet(out_queue, 0);
		/* finish the packet ready for sending */
		finish_build_data_packet_and_send(out_queue);
	}
	/* if the file is to large */
	else if(file_to_large)
	{
		/* start the 7 byte packet response */
		start_build_data_packet(out_queue, 7, BinaryCommand);
		/* write the command id */
		add_byte_of_data_to_packet(out_queue, LoadAlloc);
		/* write the allocation id - 0 as we have an error */
		add_byte_of_data_to_packet(out_queue, 0);
		/* write the file size to the packet - 00 00 00 00 as we have an error */
		add_int_to_packet(out_queue, 0);
		/* write out the error found - 3 file to large to fit into allocatio memory */
		add_byte_of_data_to_packet(out_queue, 3);
		/* finish the packet ready for sending */
		finish_build_data_packet_and_send(out_queue);
	}
	else /* failed to find/read the file */
	{
		/* start the 7 byte packet response */
		start_build_data_packet(out_queue, 7, BinaryCommand);
		/* write the command id */
		add_byte_of_data_to_packet(out_queue, LoadAlloc);
		/* write the allocation id - 0 as we have an error */
		add_byte_of_data_to_packet(out_queue, 0);
		/* write the file size to the packet - 00 00 00 00 as we have an error */
		add_int_to_packet(out_queue, 0);
		/* write out the error found - 2 unable to find/read file */
		add_byte_of_data_to_packet(out_queue, 2);
		/* finish the packet ready for sending */
		finish_build_data_packet_and_send(out_queue);
	}
	
	return( TRUE );
}

BOOL command_save_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                       */
	/* PISALLOC ALLOC_ID FILESIZE /PATH/FILENAME\n              */
	/* PISALLOC (1 byte) value 28                               */
	/* ALLOC_ID (1 byte) range 1 to 20                          */
	/* FILE_SIZE (4 byte) range 0 to sizeof(int)                */
	/*   format high16bit(lo/high) low16bit(lo/high)            */
	/* /PATH/TO/FILENAME is a string filename path  with        */
	/* \n termination                                           */
	/* example:                                                 */
	/* 28 01 00 00 30 00 <<FILENAME_IN_MEMORY_BYTES>> 10        */
	/* where the 30 bytes in allocation 1 is written to the     */
	/* file name FILENAME_IN_MEMORY_BYTES                       */
	/* format of response:                                      */
	/* PISALLOC ALLOC_ID ERROR_ID                               */
	/* PILALLOC (1 byte) value 28                               */
	/* ALLOC_ID (1 byte) range 0 to 20                          */
    /* ALLOC_ID value 0 means error occured                     */
    /* ALLOC_ID value 1 to 20 alloc_id saved from               */
    /* ERROR_ID (1 byte) range 0 to 2                           */
	/* ERROR_ID value 0 no error occured                        */
	/* ERROR_ID value 1 when parameters are incorrect           */
	/* ERROR_ID value 2 unable to save file                     */
	/* example:                                                 */
	/* 28 01 00 - saved file from allocation 1 memory with      */
	/*            no errors                                     */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	int allocation_id = get_command_next_8bit_param_bin(in_queue);	
	int file_size = get_command_next_32bit_param_bin(in_queue);	

	char file_name_to_save[MAX_PATH_SIZE_TEXT];
	
	/* clear the page_buffer */
	memset(file_name_to_save, 0, MAX_PATH_SIZE_TEXT);

	/* if we failed to read the path/name of the file to save */
	if(get_all_command_params_text(in_queue, file_name_to_save, MAX_PATH_SIZE_TEXT) == FALSE)
	{
		/* we did not get the path/filename successfully */

		/* start the 3 byte packet response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* write the command id */
		/* write the allocation id - 0 as we have an error */
		/* write out the error found - 1 parameter error */
		add_3_bytes_of_data_to_packet(out_queue, SaveAlloc, 0, 1);
		/* finish the packet ready for sending */
		finish_build_data_packet_and_send(out_queue);
	}
	/* if we saved the file from allocation_id memory */
	else if(memstore_save_data(allocation_id, file_name_to_save, file_size) == TRUE)
	{
		/* we did save the memory FILE_SIZE to file from allocation */

		if(TRON)
		{
			printf("PISALLOC ALLOC_ID %d FILE_PATH %s FILE_SIZE %d\n", allocation_id, file_name_to_save, file_size);
		}

		/* start the 3 byte packet response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* write the command id */
		/* write the allocation id */
		/* write out the error found - 0 no error */
		add_3_bytes_of_data_to_packet(out_queue, SaveAlloc, allocation_id, 0);
		/* finish the packet ready for sending */
		finish_build_data_packet_and_send(out_queue);
	}
	else /* failed to save to file */
	{
		/* start the 3 byte packet response */
		start_build_data_packet(out_queue, 3, BinaryCommand);
		/* write the command id */
		/* write the allocation id - 0 as we have an error */
		/* write out the error found - 2 unable to save file */
		add_3_bytes_of_data_to_packet(out_queue, SaveAlloc, 0, 2);
		/* finish the packet ready for sending */
		finish_build_data_packet_and_send(out_queue);
	}
	
	return( TRUE );
}


/********************************************************************/
/*                                                                  */
/* Help Functions for the command services                          */
/*                                                                  */
/********************************************************************/


BOOL command_create_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rCALLOC\r\r",
			"Function: API call that Creates an ALLOCation of memory on the pi.\r\r",
			"Format: CALLOC NUM_PAGES PAGE_SIZE\r\r",
			"  Where CALLOC is 1 byte value 21\r",
			"  Where NUM_PAGES is 2 bytes (lo/high) value 1 to 9999\r",
			"  Where PAGE_SIZE is 1 bytes value 1 to 32 (in KB)\r\r",
			"  Example 21 03 00 08 which is allocate 3 pages of 8KB\r\r",
			"Return: CALLOC ALLOC_ID ERROR_NUM\r",
			"  Where CALLOC is 1 byte value 21\r",
			"  Where ALLOC_ID is 1 byte 0 to 20, 0 indicating an error\r",
			"  Where ERROR_NUM is 1 byte, 0 no error, 1 invalid params, 2 pi memory alloc failed\r",
			"  Example 21 01 00  allocation 01 was successful\r",
			"  Example 21 00 02 failed to allocate memory on pi\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_free_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rFALLOC\r\r",
			"Function: API call that frees an ALLOCation of memory on the pi.\r\r",
			"Format: FALLOC ALLOC_ID\r\r",
			"  Where FALLOC is 1 byte value 22\r",
			"  Where ALLOC_ID is 1 bytes value 1 to 20\r\r",
			"  Example 22 01 which is free allocation_id 1\r\r",
			"Return: FALLOC ALLOC_ID ERROR_NUM\r",
			"  Where FALLOC is 1 byte value 22\r",
			"  Where ALLOC_ID is 1 byte 0 to 20, 0 indicating an error, 1 to 20 alloc freed\r",
			"  Where ERROR_NUM is 1 byte, 0 no error, 1 invalid params\r",
			"  Example 22 01 00  allocation 01 was successful freed\r",
			"  Example 22 00 01 invalid parameters (allocation_id)\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_retrieve_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rRALLOC\r\r",
			"Function: API call that retrieves a page of data from the ALLOCation of memory on the pi.\r\r",
			"Format: RALLOC ALLOC_ID PAGE_NUMBER\r\r",
			"  Where RALLOC is 1 byte value 23\r",
			"  Where ALLOC_ID is 1 bytes value 1 to 20\r",
			"  Where PAGE_NUMBER is 2 bytes (lo/hi) value 0 to 9999\r\r",
			"  Example 23 01 04 00 which is retrieve page 04 of alloc_id 01 \r\r",
			"Return: RALLOC ALLOC_ID PAGE_NUMBER ERROR_ID\r",
			"  Where RALLOC is 1 byte value 23\r",
			"  Where ALLOC_ID is 1 byte 0 to 20, 0 indicating an error, 1 to 20 alloc_id requested\r",
			"  Where PAGE_NUMBER is 2 bytes (lo/hi) of page requested\r",
			"  Where ERROR_NUM is 1 byte, 0 no error, 1 invalid params\r",
			"  Example 23 01 04 00 00  retrieved alloc_id 01, page 04 with no errors\r",
			"  The next packet will be a BinaryData packet containing the data requested.\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_store_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rSALLOC\r\r",
			"Function: API call that stores a page of data from CPC to the ALLOCation of memory on the pi.\r\r",
			"Format: SALLOC ALLOC_ID PAGE_NUMBER\r\r",
			"  Where SALLOC is 1 byte value 24\r",
			"  Where ALLOC_ID is 1 bytes value 1 to 20\r",
			"  Where PAGE_NUMBER is 2 bytes (lo/hi) value 0 to 9999\r\r",
			"  Example 24 01 04 00 which is store page 04 in alloc_id 01 \r",
			"  The BinaryData to store is expected in the following BinaryData packet following this command.\r\r"
			"Return: SALLOC ALLOC_ID PAGE_NUMBER ERROR_ID\r",
			"  Where SALLOC is 1 byte value 24\r",
			"  Where ALLOC_ID is 1 byte 0 to 20, 0 indicating an error, 1 to 20 alloc_id requested\r",
			"  Where PAGE_NUMBER is 2 bytes (lo/hi) of page requested\r",
			"  Where ERROR_NUM is 1 byte, 0 no error, 1 invalid params\r",
			"  Example 24 01 04 00 00  stored alloc_id 01, page 04 with no errors\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}
	
BOOL command_insert_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rIALLOC\r\r",
			"Function: API call that inserts a chunk of data into the memory allocation on the pi.\r\r",
			"Format: IALLOC ALLOC_ID INSERT_AT INSERT_LENGTH \r\r",
			"  Where IALLOC is 1 byte value 25\r",
			"  Where ALLOC_ID is 1 bytes value 1 to 20\r",
			"  Where INSERT_AT is 4 bytes format high16bit(lo/high) low16bit(lo/high) range 0 to sizeof(int)\r",
			"  Where INSERT_LENGTH is 4 bytes format high16bit(lo/high) low16bit(lo/high) range 0 to sizeof(int)\r\r",
			"  Example 25 01 00 00 02 00 00 00 10 00 which is insert at memory position 2, in alloc_id 01, 10 bytes \r",
			"  The BinaryData to insert is expected in the following BinaryData packet following this command.\r\r"
			"Return: IALLOC ALLOC_ID ERROR_ID\r",
			"  Where IALLOC is 1 byte value 25\r",
			"  Where ALLOC_ID is 1 byte 0 to 20, 0 indicating an error, 1 to 20 alloc_id requested\r",
			"  Where ERROR_NUM is 1 byte, 0 no error, 1 invalid params,\r",
			"  2 unable to allocate memory on pi, 3 insert out side allocation memory\r",
			"  Example 25 01 00 inserted the following BinaryData packet data into alloc_id 01 with no errors\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_cut_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rXALLOC\r\r",
			"Function: API call that cuts a chunk of data from a memory allocation on the pi.\r\r",
			"Format: XALLOC ALLOC_ID CUT_FROM CUT_LENGTH \r\r",
			"  Where XALLOC is 1 byte value 26\r",
			"  Where ALLOC_ID is 1 bytes value 1 to 20\r",
			"  Where CUT_FROM is 4 bytes format high16bit(lo/high) low16bit(lo/high) range 0 to sizeof(int)\r",
			"  Where CUT_LENGTH is 4 bytes format high16bit(lo/high) low16bit(lo/high) range 0 to sizeof(int)\r\r",
			"  Example 26 01 00 00 08 00 00 00 10 00 which is cut from memory position 8, in alloc_id 01, 10 bytes \r",
			"Return: XALLOC ALLOC_ID ERROR_ID\r",
			"  Where XALLOC is 1 byte value 26\r",
			"  Where ALLOC_ID is 1 byte 0 to 20, 0 indicating an error, 1 to 20 alloc_id requested\r",
			"  Where ERROR_NUM is 1 byte, 0 no error, 1 invalid params, 2 cut outside memory allocation\r",
			"  Example 26 01 00 cut data requested from alloc_id 01 with no errors\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_load_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rPILALLOC\r\r",
			"Function: API call that loads a file from the pi storage into a memory allocation on the pi.\r\r",
			"Format: PILALLOC ALLOC_ID /PATH/TO/FILE\n\r\r",
			"  Where PILALLOC is 1 byte value 27\r",
			"  Where ALLOC_ID is 1 bytes value 1 to 20\r",
			"  Where /PATH/TO/FILE is a \n terminated string\r\r",
			"  Example 27 01 BINARY_STRING_OF_TEXT 10 which is load file into allocation 01\r",
			"Return: PILALLOC ALLOC_ID FILE_SIZE ERROR_ID\r",
			"  Where PILALLOC is 1 byte value 27\r",
			"  Where ALLOC_ID is 1 byte 0 to 20, 0 indicating an error, 1 to 20 alloc_id requested\r",
			"  Where FILE_SIZE is 4 bytes format high16bit(lo/high) low16bit(lo/high) range 0 to sizeof(int) of file loaded\r",
			"  Where ERROR_NUM is 1 byte, 0 no error, 1 invalid params, 2 unable to load file, 3 file to large for allocation\r",
			"  Example 27 01 00 00 FF 7F 00 loaded the file, 32K in size into memory alloc_id 01 with no errors\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_save_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rPISALLOC\r\r",
			"Function: API call that saves a file from a pi memory allocation to pi storage.\r\r",
			"Format: PISALLOC ALLOC_ID FILE_SIZE /PATH/TO/FILE\n\r\r",
			"  Where PISALLOC is 1 byte value 28\r",
			"  Where ALLOC_ID is 1 bytes value 1 to 20\r",
			"  Where FILE_SIZE is 4 bytes format high16bit(lo/high) low16bit(lo/high) range 0 to sizeof(int)\r",
			"  of bytes to save from the pi memory allocation to the file on pi storage\r",
			"  Where /PATH/TO/FILE is a \n terminated string of the file to save too.\r\r",
			"  Example 28 01 00 00 FF 04 BINARY_STRING_OF_TEXT 10 which is save a 1K file from allocation 01 to file name\r",
			"Return: PISALLOC ALLOC_ID ERROR_ID\r",
			"  Where PISALLOC is 1 byte value 28\r",
			"  Where ALLOC_ID is 1 byte 0 to 20, 0 indicating an error, 1 to 20 alloc_id requested\r",
			"  Where ERROR_NUM is 1 byte, 0 no error, 1 invalid params, 2 unable to save file\r",
			"  Example 28 01 00 save the allocation 01 to the requested file with no errors\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

