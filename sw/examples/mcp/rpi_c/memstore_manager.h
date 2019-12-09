/*********************************************************/
/*                                                       */
/* The definition of the memstore used to track          */
/* allocation of memory on the PI for use on CPC         */
/*                                                       */
/*********************************************************/

#ifndef memstore_manager_H
#define memstore_manager_H 1

#include <stdio.h>

#include "mcp_pi.h"

/* this is the maximum number of memory allocations at one time              */
/* when creating an allocation with CPAGE you create one memory allocation   */
/* of number_pages * page_size bytes                                         */
#define MAX_MEMORY_ALLOCATIONS 21

/* this is the maximum size for each page of an allocation                   */
#define MAX_PAGE_SIZE 32768

typedef struct Memory_Allocation
{
	int allocation_id;
	int number_pages;
	int page_size;
	
	FILE *file_id;
	int file_size;
	
	BYTE *data;
	
} Memory_Allocation;

/* initialise the memstore data structures */
void memstore_init(void);

/* reset the memstore data structures - freeing the memory used */
void memstore_reset(void);

/* get the size of a page in bytes for that allocation_id */
int memstore_get_page_size_bytes(int allocation_id);

/* get the size of the allocation for that allocation_id */
int memstore_get_alloc_size_bytes(int allocation_id);

/* create a memory allocation for the number Pages * page_size_in_kb size */
int memstore_create_allocation_id(int number_pages, int page_size_in_kb);

/* free a memory allocation and all the data it contains */
BOOL memstore_free_allocation_id(int allocation_id);

/* retrieve a page of data in the memory_allocation_id memory at page_number */
BYTE *memstore_retrieve_page_of_data(int allocation_id, int page_number, int *number_of_bytes);

/* store the page of data in the memory_allocation_id memory at page_number */
BOOL memstore_store_page_of_data(int allocation_id, int page_number, BYTE *page_data);

/* insert the data in the memory_allocation_id memory at a defined point for a defined number of bytes */
BOOL memstore_insert_data(int allocation_id, int insert_at_index, int insert_length, BYTE *insert_data);

/* cut data from the memory_allocation_id memory at a defined point for a defined number of bytes */
BOOL memstore_cut_data(int allocation_id, int cut_at_index, int cut_length);

/* load the filename into the allocation_id pages memory */
BOOL memstore_load_data(int allocation_id, char *file_name, int *file_size, BOOL *file_to_large);

/* save filename from the allocation_id pages in memory */
BOOL memstore_save_data(int allocation_id, char *file_name, int file_size);

/* finds a free memory allocation to use */
int get_free_memory_allocation(void);


#endif