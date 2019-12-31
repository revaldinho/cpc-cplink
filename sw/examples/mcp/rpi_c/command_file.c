/********************************/
/*                              */
/* File Command Functions       */
/*                              */
/********************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "command_file.h"
#include "memstore_manager.h"
#include "command.h"
#include "packet.h"

/********************************************************************/
/*                                                                  */
/* TextCommand Functions providing the command services             */
/*                                                                  */
/********************************************************************/

/* get a file from the pi's storage to write to cpc storage */
BOOL command_get_file(Queue *in_queue, Queue *out_queue, int tick)
{
	return( FALSE );/* TBD */
}

/* put a file on cpc storage on to the pi's storage */
BOOL command_put_file(Queue *in_queue, Queue *out_queue, int tick)
{
	return( FALSE );/* TBD */
}

/********************************************************************/
/*                                                                  */
/* BinaryCommand Functions providing the command services           */
/*                                                                  */
/********************************************************************/

/* get a file from the pi storage into cpc memory */
BOOL command_get_mem_file(Queue *in_queue, Queue *out_queue, int tick)
{
	return( FALSE );/* TBD */
}

/* put a file in cpc memory into pi storage */
BOOL command_put_mem_file(Queue *in_queue, Queue *out_queue, int tick)
{
	return( FALSE );/* TBD */
}


/********************************************************************/
/*                                                                  */
/* Help Functions for the command services                          */
/*                                                                  */
/********************************************************************/

BOOL command_get_file_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rGETFILE\r\r",
			"Function: Retrieves a file from pi storage and stores it on cpc storage.\r\r",
			"Format: GETFILE /PATH/TO/FILE\n\r\r",
			"Return: GETFILE\r\r\n",
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

BOOL command_put_file_help(Queue *in_queue, Queue *out_queue)
{
	return( FALSE ); /* TBD */
}

BOOL command_get_mem_file_help(Queue *in_queue, Queue *out_queue)
{
	return( FALSE ); /* TBD */
}

BOOL command_put_mem_file_help(Queue *in_queue, Queue *out_queue)
{
	return( FALSE ); /* TBD */
}
