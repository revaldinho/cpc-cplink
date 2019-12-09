/*********************************************************/
/*                                                       */
/* The definition of the memstore used to track      */
/* page allocation of memory on the PI for use on CPC    */
/*                                                       */
/*********************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "memstore_manager.h"

Memory_Allocation memstore[MAX_MEMORY_ALLOCATIONS];

/* initialise the memstore data structures */
void memstore_init(void)
{
	memset(&memstore, 0, sizeof(Memory_Allocation) * MAX_MEMORY_ALLOCATIONS);
}

/* reset the memstore data structures - freeing the memory used */
void memstore_reset(void)
{
	for(int index = 0; index < MAX_MEMORY_ALLOCATIONS; index++)
	{
		/* if page was allocated i.e. non_zero page id */
		if(memstore[index].allocation_id != 0)
		{
			/* was the file pointer in use */
			if(memstore[index].file_id != NULL)
			{
				/* close the file_id */
				fclose(memstore[index].file_id);
			}
			
			/* was memory allocated */
			if(memstore[index].data != NULL)
			{
				/* free the memory */
				free(memstore[index].data);
			}
		}
	}
	
	/* now init the data structures */
	memstore_init();
}

/* get the size of a page in bytes for that allocation_id */
int memstore_get_page_size_bytes(int allocation_id)
{
	int number_bytes = 0;
	
	/* allocation_id must be in the valid range */
	if((allocation_id > 0) && (allocation_id < MAX_MEMORY_ALLOCATIONS))
	{
		/* if the allocation_id is allocated then calculate page size in bytes */
		if(memstore[allocation_id].allocation_id == allocation_id)
		{
			number_bytes = memstore[allocation_id].page_size;
		}
	}
	
	return( number_bytes );
}

/* get the size of the allocation for that allocation_id */
int memstore_get_alloc_size_bytes(int allocation_id)
{
	int number_bytes = 0;
	
	/* allocation_id must be in the valid range */
	if((allocation_id > 0) && (allocation_id < MAX_MEMORY_ALLOCATIONS))
	{
		/* if the allocation_id is allocated then calculate page size in bytes */
		if(memstore[allocation_id].allocation_id == allocation_id)
		{
			number_bytes = memstore[allocation_id].page_size * memstore[allocation_id].number_pages;
		}
	}
	
	return( number_bytes );
}


/* create a memory allocation for the number Pages * page_size_in_kb size */
int memstore_create_allocation_id(int number_pages, int page_size_in_kb)
{
	int allocation_id = get_free_memory_allocation();
	
	#ifdef DEBUG_MCP
		printf("AllocID: %d\n", allocation_id);
	#endif
	
	if((allocation_id > 0) && (allocation_id < MAX_MEMORY_ALLOCATIONS))
	{
   	    /* get the allocation page */
		Memory_Allocation *allocation = &memstore[allocation_id];

		/* we got a memory allocation id so set up and allocate the space */
		allocation->allocation_id   = allocation_id;
		allocation->number_pages    = number_pages;
		allocation->page_size       = page_size_in_kb * 1024;
		allocation->data            = (BYTE *) calloc(number_pages*page_size_in_kb*1024,1);
		
		/* we failed to allocated the requested memory */
		if(allocation->data == NULL)
		{
			if(TRON)
			{
				printf("Failed to allocate memory\n");
			}
			
			/* clear the memory allocation set up as we failed to allocate the memory to use */
			memset(&memstore[allocation_id], 0, sizeof(Memory_Allocation));
			allocation_id = 0;
		}
		else
		{
			/* copy some data into during dev so we can see it get copied to the out_queue */
			#ifdef DEBUG_MCP_IF
				memcpy(allocation->data, "ABCDEFG", 7);
			#endif
		}
	}
	
	return( allocation_id );
}

/* free a memory allocation and all the data it contains */
BOOL memstore_free_allocation_id(int allocation_id)
{
	BOOL processed_free_request = FALSE;
	
	if(allocation_id < MAX_MEMORY_ALLOCATIONS)
	{
		/* if the allocation_id to be freed is allocated */
		if(memstore[allocation_id].allocation_id == allocation_id)
		{
			/* if the memory allocation id has data allocated */
			if(memstore[allocation_id].data != NULL)
			{
				if(TRON)
				{
					printf("Freeing memory %d bytes for allocation id %d\n", 
						   memstore[allocation_id].number_pages * memstore[allocation_id].page_size, 
						   allocation_id);
				}
				
				/* free the data */
				free(memstore[allocation_id].data);
			}

			/* reset the memory allocation to defaults */
			memset(&memstore[allocation_id], 0, sizeof(Memory_Allocation));

			processed_free_request = TRUE;
		}
	}
	
	return( processed_free_request );
}

/* retrieve a page of data in the allocation_id memory at page_number */
BYTE *memstore_retrieve_page_of_data(int allocation_id, int page_number, int *number_of_bytes)
{
	BYTE *data = NULL;
	
	/* check allocation_id is within the legal range, page number is in the legal range */
	/* and the allocation is enabled for that allocation_id                                  */
	if((allocation_id > 0) && 
	   (allocation_id < MAX_MEMORY_ALLOCATIONS) &&
	   (page_number >= 0) && 
	   (page_number < memstore[allocation_id].number_pages) &&
	   (memstore[allocation_id].allocation_id == allocation_id))
   	{
		/* get the allocation page */
		Memory_Allocation *allocation = &memstore[allocation_id];
		
		/* set data to the pointer based on page_number * page_size */
		data = &allocation->data[page_number * allocation->page_size];
		
		/* return the number of bytes that can be read */
		*number_of_bytes = allocation->page_size;
	}
	
	return( data );
}

/* store the page of data in the allocation_id memory at page_number */
BOOL memstore_store_page_of_data(int allocation_id, int page_number, BYTE *page_data_buffer)
{
	BOOL store_complete = FALSE;
	
	/* check allocation_id is within the legal range, page number is in the legal range */
	/* and the allocation is enabled for that allocation_id                                  */
	if((allocation_id > 0) && 
	   (allocation_id < MAX_MEMORY_ALLOCATIONS) &&
	   (page_number >= 0) && 
	   (page_number < memstore[allocation_id].number_pages) &&
 	   (memstore[allocation_id].allocation_id == allocation_id))
	{
		/* get the allocation page */
		Memory_Allocation *allocation = &memstore[allocation_id];
		
		/* copy the page worth of data from the page_data_buffer into the allocation_id page */
		memcpy(&allocation->data[page_number * allocation->page_size],
			   page_data_buffer,
			   allocation->page_size);
		
		store_complete = TRUE;			
	}
	
	return( store_complete );
}

/* insert data in the allocation_id memory at a defined point for a defined number of bytes */
BOOL memstore_insert_data(int allocation_id, int insert_at_index, int insert_length, BYTE *insert_data)
{
	BOOL store_complete = FALSE;
	
	/* check allocation_id is within the legal range, insert fits within allocation */
	/* and the allocation is enabled for that allocation_id                         */
	if((allocation_id > 0) && (allocation_id < MAX_MEMORY_ALLOCATIONS) &&
	   (insert_at_index >= 0) && (insert_length > 0) &&
	   ((insert_at_index + insert_length) < (memstore[allocation_id].number_pages * memstore[allocation_id].page_size)) &&
 	   (memstore[allocation_id].allocation_id == allocation_id))
	{
		/* get the allocation page */
		Memory_Allocation *allocation = &memstore[allocation_id];

		int bytes_in_allocation_to_move = (allocation->number_pages * allocation->page_size) - insert_at_index - insert_length;
			
		/* copy the data from insert_at_index up insert_length bytes to move it out of the way      */
		/* insert_length bytes will be lost from the top end of the memory allocation to make space */
		/* its up to the requester to ensure this is not data they want                             */
		memmove(&allocation->data[insert_at_index+insert_length],&allocation->data[insert_at_index], bytes_in_allocation_to_move);
		/* copy the insert_length of data from the insert_data into the allocation_id */
		memcpy(&allocation->data[insert_at_index], insert_data, insert_length);
		
		store_complete = TRUE;			
	}
	
	return( store_complete );
}

/* cut data from the allocation_id memory at a defined point for a defined number of bytes */
BOOL memstore_cut_data(int allocation_id, int cut_at_index, int cut_length)
{
	BOOL store_complete = FALSE;
	
	/* check allocation_id is within the legal range, cut fits within allocation */
	/* and the allocation is enabled for that allocation_id                      */
	if((allocation_id > 0) && (allocation_id < MAX_MEMORY_ALLOCATIONS) &&
	   (cut_at_index >= 0) && (cut_length > 0) &&
	   ((cut_at_index + cut_length) < (memstore[allocation_id].number_pages * memstore[allocation_id].page_size)) &&
 	   (memstore[allocation_id].allocation_id == allocation_id))
	{
		/* get the allocation page */
		Memory_Allocation *allocation = &memstore[allocation_id];

		int bytes_in_allocation_to_move_down = (allocation->number_pages * allocation->page_size) - cut_at_index - cut_length;
		int byte_index_in_allocation_to_clear = (allocation->number_pages * allocation->page_size) - cut_length;
			
		/* cut the data from cut_at_index for cut_length, shuffling memory down to fill that cut */
		memmove(&allocation->data[cut_at_index],&allocation->data[cut_at_index + cut_length], bytes_in_allocation_to_move_down);
		/* clear the memory from top of allocation down for cut_length */
		memset(&allocation->data[byte_index_in_allocation_to_clear], 0, cut_length);
		
		store_complete = TRUE;			
	}
	
	return( store_complete );
}

/* load the filename into the allocation_id pages memory */
BOOL memstore_load_data(int allocation_id, char *file_name, int *file_size, BOOL *file_to_large)
{
	BOOL read_file_ok = FALSE;
	BOOL close_file_id = FALSE;
	
	/* check allocation_id is within the legal range and a file_name was passed */
	/* and the allocation is enabled for that allocation_id                          */
	if((allocation_id > 0) && 
	   (allocation_id < MAX_MEMORY_ALLOCATIONS) &&
 	   (memstore[allocation_id].allocation_id == allocation_id) &&
	   (file_name != NULL))
	{
		/* get the allocation page */
		Memory_Allocation *allocation = &memstore[allocation_id];
		
		/* if the file_id is already allocated */
		if(allocation->file_id != NULL)
		{
			/* close the file_id */
			fclose(allocation->file_id);
			allocation->file_id = NULL;
			
			/* clear the data ready for a fresh file load */
			memset(allocation->data, 0, allocation->number_pages * allocation->page_size);			
		}
		
		/* try and open the file */
		allocation->file_id = fopen(file_name, "r");
		
		if(TRON)
		{
			printf("Opened File_ID %p\n", allocation->file_id );
		}

		/* if we found and opened the file */
		if(allocation->file_id)
		{
			/* seek to the end of the file*/
			if(fseek(allocation->file_id, 0, SEEK_END) == 0)
			{
				/* read the file length */
				int file_length = ftell(allocation->file_id);

				/* if the file length will fit within the allocation memory size */
				if(file_length <= (allocation->number_pages * allocation->page_size))
				{
					/* try and read the file from the begining */
					
					/* see to the begining of the file */
					if(fseek(allocation->file_id, 0, SEEK_SET) == 0)
					{
						/* now read into the allocation memory */
						if(fread(allocation->data, 1, file_length, allocation->file_id))
						{
							/* return the file length */
							*file_size = file_length;
							/* store the file size as part of the allocation description */
							allocation->file_size = file_length;
							/* we read the file fine */
							read_file_ok = TRUE;
						}
						else
						{
							close_file_id = TRUE;
						}
					}
					else /* close the file_id */
					{
						close_file_id = TRUE;
					}
				}
				else /* the file will not fit within the allocation memory */
				{
					*file_to_large = TRUE;
					
					close_file_id = TRUE;
				}
			}
			else /* close the file_id */
			{
				close_file_id = TRUE;
			}

			/* if we failed some where then close the file_id */
			if( close_file_id )
			{
				/* close the file_id */
				fclose(allocation->file_id);
				allocation->file_id = NULL;

				/* clear the data ready for a fresh file load */
				memset(allocation->data, 0, allocation->number_pages * allocation->page_size);			
			}
		}
	}	

	return( read_file_ok );
}

/* save filename from the allocation_id pages in memory */
BOOL memstore_save_data(int allocation_id, char *file_name, int file_size)
{
	BOOL save_file_ok = FALSE;

	/* check allocation_id is within the legal range and a file_name was passed */
	/* and file_size > 0 and the allocation is enabled for that allocation_id        */
	if((allocation_id > 0) && 
	   (allocation_id < MAX_MEMORY_ALLOCATIONS) &&
 	   (memstore[allocation_id].allocation_id == allocation_id) &&
	   (file_name != NULL) &&
	   (file_size > 0))
	{
		/* get the allocation page */
		Memory_Allocation *allocation = &memstore[allocation_id];
		
		/* if the file_id is already allocated */
		if(allocation->file_id != NULL)
		{
			/* close the file_id */
			fclose(allocation->file_id);
			allocation->file_id = NULL;
		}
		
		/* try and open the file to write too */
		allocation->file_id = fopen(file_name, "w");
		
		if(TRON)
		{
			printf("Opened File_ID %p\n", allocation->file_id );
		}

		/* if we found and opened the file */
		if(allocation->file_id)
		{
			/* if the file length to write is within the allocation memory size */
			if(file_size <= (allocation->number_pages * allocation->page_size))
			{
				/* try and read the file from the begining to file_size from the allocation memory */

				/* now read into the allocation memory */
				if(fwrite(allocation->data, 1, file_size, allocation->file_id))
				{
					save_file_ok = TRUE;
				}
			}

			/* close the file if we wrote to it or not */
			fclose(allocation->file_id);
			allocation->file_id = NULL;
		}
	}	
	
	return( save_file_ok );
}

int get_free_memory_allocation(void)
{
	for( int index = 1; index < MAX_MEMORY_ALLOCATIONS; index++ )
	{
		/* look for a free page_id */
		if( memstore[index].allocation_id == 0 )
		{
			return( index );
		}
	}
	
	return( 0 );
}