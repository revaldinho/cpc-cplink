/************************************************************/
/*                                                          */
/* The implementation of commands (services) that the pi    */
/* provides to the CPC                                      */
/*                                                          */
/* Note command return data MUST end with a \n              */
/*                                                          */
/************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#include "command.h"
#include "command_processor.h"

/**********************************************************************************/
/* To add a new command, create the entry in the table here add the commandID in  */
/* command.h (and update MaxCommands counter) and add the new function that       */
/* implements the command at the bottom of the file                               */
/* New commands need to be added here in the order they appear in CommandIDs enum */
/**********************************************************************************/
const CommandDef command_defs[MaxCommands] = 
	{
	    /* command, command_name, function to execute */
		{Unknown,    "UNKNOWN",   command_unknown}, 
		{Ping,       "PING",      command_ping},
		{Shutdown,   "SHUTDOWN",  command_shutdown},
		{Time,       "TIME",      command_time},
		{Date,       "DATE",      command_date},
		{Reset,      "RESET",     command_reset},
	
	    {CreatePage, "CPAGE",     command_create_page},
	    {FetchPage,  "FPAGE",     command_fetch_page},
	    {StorePage,  "SPAGE",     command_store_page},
	    {LoadPages,  "PILPAGES",  command_load_pages},
	    {SavePages,  "PISPAGES",  command_save_pages}
	};

/* get the command id from the command_string */                   
Command get_commandID_from_string(BYTE *command_string)
{
	/* look through the table */
	for(int index = 0; index < MaxCommands; index++ )
	{
		/* if the command_name matches the command_string */
		if( strcmp(command_defs[index].command_text, command_string) == 0 )
		{
			/* return the id of the command matched */
			return( command_defs[index].commandID );
		}
	}
	
	return(Unknown);
}

/* from the commandID return a pointer to the command function */
void *get_command_function(Command commandId)
{
	return(command_defs[commandId].return_function);
}

/* this function reads the queue for a parameter (space seperated) */	
BOOL get_command_param(Queue *in_queue, BYTE *param_found)
{
	return(FALSE);
}

/***************************************************************/
/*                                                             */
/* Built in Simple Commands                                    */
/*                                                             */
/***************************************************************/


/* unknown command - tick is ignored as all output in one call */
BOOL command_unknown(Queue *in_queue, Queue *out_queue, int tick)
{
	BYTE *response = "Unknown Command\n";
	
	if(write_data_to_queue(out_queue, response, strlen(response)) != TRUE)
	{
		printf("Unable to write response '%s' to out_queue", response);
		/* return FALSE as command is not complete i.e. will be tried again */
		return( FALSE );
	}
	
	/* return TRUE - function completed i.e. no more segments to do on each tick */
	return( TRUE );
}

/* ping command - tick is ignored as all output in one call */
BOOL command_ping(Queue *in_queue, Queue *out_queue, int tick)
{
	BYTE *response = "PONG!\n";

	if(write_data_to_queue(out_queue, response, strlen(response)) != TRUE)
	{
		printf("Unable to write response '%s' to out_queue", response);
		/* return FALSE as command is not complete i.e. will be tried again */
		return( FALSE );
	}
	
	/* return TRUE - function completed i.e. no more segments to do on each tick */
	return( TRUE );
}

BOOL command_shutdown(Queue *in_queue, Queue *out_queue, int tick)
{
	BYTE *response = "SHUTDOWN IN 90 SECONDS\n";

	#ifdef BUILD_ON_PC
		printf("Running Shutdown (Fake) : %d\n", system("/bin/pwd"));
	#else
		printf("Running Shutdown : %d\n", system("sudo /sbin/shutdown -h +1"));
	#endif
	
	/* write the OK response to the out_queue for sending to client */
	if(write_data_to_queue(out_queue, response, strlen(response)) != TRUE)
	{	BYTE *response = "Unknown Command\n";
	
	if(write_data_to_queue(out_queue, response, strlen(response)) != TRUE)
	{
		printf("Unable to write response '%s' to out_queue", response);
		/* return FALSE as command is not complete i.e. will be tried again */
		return( FALSE );
	}
	
	/* return TRUE - function completed i.e. no more segments to do on each tick */
	return( TRUE );

		printf("Unable to write response '%s' to out_queue", response);
		/* return FALSE as command is not complete i.e. will be tried again */
		return( FALSE );
	}
	return( TRUE );
}

BOOL command_time(Queue *in_queue, Queue *out_queue, int tick)
{
	char current_time[7];
	
	time_t rawtime;
	struct tm * timeinfo;
	
	memset(current_time, 0, 7);
	
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	
	sprintf(current_time, "%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min);
	
	/* write the current_time response to the out_queue for sending to client */
	if(write_data_to_queue(out_queue, current_time, strlen(current_time)) != TRUE)
	{
		printf("Unable to write current_time '%s' to out_queue", current_time);
		/* return FALSE as command is not complete i.e. will be tried again */
		return( FALSE );
	}
	
	return( TRUE );
}

BOOL command_date(Queue *in_queue, Queue *out_queue, int tick)
{
	char current_date[11];
	
	time_t rawtime;
	struct tm * timeinfo;
	
	memset(current_date, 0, 11);
	
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	
	sprintf(current_date, "%02d:%02d:%04d\n", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900);
	
	/* write the current_date to the out_queue for sending to client */
	if(write_data_to_queue(out_queue, current_date, strlen(current_date)) != TRUE)
	{
		printf("Unable to write current_date '%s' to out_queue", current_date);
		/* return FALSE as command is not complete i.e. will be tried again */
		return( FALSE );
	}
	
	return( TRUE );
}

BOOL command_reset(Queue *in_queue, Queue *out_queue, int tick)
{
	BYTE *response = "OK\n";

    printf("Reset Queues and Command Processor\n");
	
	/* reset the queues */
	reset_queues();
	
	if(write_data_to_queue(out_queue, response, strlen(response)) != TRUE)
	{
		printf("Unable to write response '%s' to out_queue", response);
		/* return FALSE as command is not complete i.e. will be tried again */
		return( FALSE );
	}
	
	/* return TRUE - function completed i.e. no more segments to do on each tick */
	return( TRUE );

	/* reset the command processor */
	command_processor_init();
	
	/* write the OK response to the out_queue for sending to client */
	if(write_data_to_queue(out_queue, response, strlen(response)) != TRUE)
	{
		printf("Unable to write response '%s' to out_queue", response);
		/* return FALSE as command is not complete i.e. will be tried again */
		return( FALSE );
	}
	return( TRUE );
}

/**********************************************************************************/
/*                                                                                */
/* These commands control moving pages of memory in/out of the CPC to/from the pi */
/*                                                                                */
/* CPAGE will create the number and size of pages requested in PI memory          */
/* FPAGE is used by the CPC to fetch a page from the PI memory to its memory      */
/* SPAGE is used by the CPC to store a page from its memory to the PI memory      */
/* PILPAGES is used by the CPC to tell the PI to load a file into its page memory */
/*          The CPC can then page the loaded file in and out of CPC memory from   */
/*          the PI's page memory
/* PISPAGES is used by the CPC to tell the PI to save its page memory back to     */
/*          file on the PI storage                                                */
/*                                                                                */
/* See each function for format of the messages                                   */
/*                                                                                */
/**********************************************************************************/

BOOL command_create_page(Queue *in_queue, Queue *out_queue, int tick)
{
	BYTE *response  = "OK\n";
	BYTE *response2 = "CPAGE INVALID PARAMETERS\n";
	
	/* format of CPAGE NUMBER_PAGES SIZEOFPAGE_IN_KB\n */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	char number_pages_text[4];
	char page_size_in_kb_text[3];

	if( get_command_param(in_queue, number_pages_text) && get_command_param(in_queue, page_size_in_kb_text))
	{
		/* we got the two parameters */
		int number_pages = atoi(number_pages_text);
		int page_size_in_kb = atoi(page_size_in_kb_text);
		
		#ifdef DEBUG_MCP
			printf("CPAGE Requested %d pages of %d KB", number_pages, page_size_in_kb);
		#endif
		
		/* init the page manager with the number and size of pages */
        /* TBD */
		
		/* send an OK message back to the CPC */
		if(write_data_to_queue(out_queue, response, strlen(response)) != TRUE)
		{
			printf("Unable to write response '%s' to out_queue", response);
			/* return FALSE as command is not complete i.e. will be tried again */
			return( FALSE );
		}
	}
	else /* didnt read the two parameters */
	{
		/* we failed too read the parameters */
		
		/* send an error message back to the CPC */
		if(write_data_to_queue(out_queue, response2, strlen(response2)) != TRUE)
		{
			printf("Unable to write response '%s' to out_queue", response2);
			/* return FALSE as command is not complete i.e. will be tried again */
			return( FALSE );
		}
	}
		
	return( TRUE );
}

BOOL command_fetch_page(Queue *in_queue, Queue *out_queue, int tick)
{
	return( TRUE );
}
	
BOOL command_store_page(Queue *in_queue, Queue *out_queue, int tick)
{
	return( TRUE );
}

BOOL command_load_pages(Queue *in_queue, Queue *out_queue, int tick)
{
	return( TRUE );
}

BOOL command_save_pages(Queue *in_queue, Queue *out_queue, int tick)
{
	return( TRUE );
}

