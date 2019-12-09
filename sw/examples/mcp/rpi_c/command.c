/************************************************************/
/*                                                          */
/* The implementation of commands (services) that the pi    */
/* provides to the CPC                                      */
/*                                                          */
/************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <time.h>

#include "command.h"
#include "packet.h"
#include "command_processor.h"
#include "memstore_manager.h"

/**********************************************************************************/
/* To add a new command, create the entry in the table here add the commandID in  */
/* command.h (and update MaxCommands counter) and add the new function that       */
/* implements the command at the bottom of the file                               */
/* New commands need to be added here in the order they appear in CommandIDs enum */
/* The fourth parameter is the tick time for the command i.e. if it fails or has  */
/* multiple segments to run, how many milliseconds to wait till it runs again     */
/* 0 for not relevant to that command i.e. Unknown and Reset will always complete */
/* The fifth parameter is the out_queue space that the command typically          */
/* requires in order to output its return to the client - this must be met before */
/* the command_processor will actually allow the command to run                   */
/**********************************************************************************/
const CommandDef command_defs[MaxCommands] = 
	{
	    /* command, command_name, function to execute, help_function, tick_duration, out_queue space required */
		{Unknown,      "UNKNOWN",   command_unknown,        command_unknown_help,         100,   30}, 
		{Ping,         "PING",      command_ping,           command_ping_help,            100,   30},
		{Shutdown,     "SHUTDOWN",  command_shutdown,       command_shutdown_help,        3000,  30},
		{Time,         "TIME",      command_time,           command_time_help,            100,   30},
		{Date,         "DATE",      command_date,           command_date_help,            100,   30},
		{Reset,        "RESET",     command_reset,          command_reset_help,           100,   30},
		{Help,         "HELP",      command_help,           command_help_help,            100,   1024},
		{Reboot,       "REBOOT",    command_reboot,         command_reboot_help,          3000,  30},
		{Tron,         "TRON",      command_tron,           command_tron_help,            100,   30},
		{Troff,        "TROFF",     command_troff,          command_troff_help,           100,   30},
	
	    {CreateAlloc,   "CALLOC",    command_create_alloc,   command_create_alloc_help,    100,   30},
	    {FreeAlloc,     "FALLOC",    command_free_alloc,     command_free_alloc_help,      100,   30},
	    {RetrieveAlloc, "RALLOC",    command_retrieve_alloc, command_retrieve_alloc_help,  500,   30},
	    {StoreAlloc,    "SALLOC",    command_store_alloc,    command_store_alloc_help,     100,   30},
	    {InsertAlloc,   "IALLOC",    command_insert_alloc,   command_insert_alloc_help,    100,   30},
	    {CutAlloc,      "XALLOC",    command_cut_alloc,      command_cut_alloc_help,       100,   30},
	    {LoadAlloc,     "PILALLOC",  command_load_alloc,     command_load_alloc_help,      100,   30},
	    {SaveAlloc,     "PISALLOC",  command_save_alloc,     command_save_alloc_help,      100,   30}
	};

/* get the command id from the command_string */                   
Command get_commandID_from_string(char *command_string)
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
void *get_command_function(Command command_id)
{
	return(command_defs[command_id].return_function);
}

/* get the tick_time for the command */
long get_command_tick_period(Command command_id)
{
	return(command_defs[command_id].tick_time);
}

/* get the size of the out_queue typically used by the command */
int get_command_out_queue_space_required(Command command_id)
{
	return(command_defs[command_id].out_queue_space);
}

/* get the text string of the command name */
void get_command_string(Queue *in_queue, char *command_string)
{
  int index = 0;
  int temp_read_index = in_queue->read_next_byte;
  
  BOOL command_name_complete = FALSE;
  
  /* clear the command string */
  memset(command_string, 0, MAX_COMMAND_NAME_LENGTH);
  
  /* +6 to skip +++ two_byte_size one_byte_packet_type */
  increment_read_next_index(in_queue, FRONT_DELIMITER_OVERHEAD + FRONT_PACKET_OVERHEAD);
	  
  /* search the queue for a space from the current read point as the command format */
  /* is COMMANDNAME OPTIONALPARAM1 ETC\n so we will look from the current read point to */
  /* the space to get the COMMANDNAME but only for a MAXIMUM size of the COMMANDNAME */
  
  while((index < MAX_COMMAND_NAME_LENGTH) && (command_name_complete == FALSE))
  {
    /* did we find a space to terminate the command name text */
    /* or a \n indicating end of command i.e. no params       */
    if(((get_current_read_byte(in_queue) == ' ') || (get_current_read_byte(in_queue) == '\n')) && (index > 0))
    {
      /* found a space (or \n) at the end of a command name */
      command_name_complete = TRUE;
      /* increment the read_next_byte pointer to skip ' ' or '\n' */
	  increment_read_next_index(in_queue, 1);
    }
    else if((get_current_read_byte(in_queue) == '\n') && (index == 0))
    {
      /* skip the first byte as its \n which is an error */
	  increment_read_next_index(in_queue, 1);
    }
    else /* copy the byte to the command name string */
    {
      command_string[index] = get_current_read_byte(in_queue);
	  increment_read_next_index(in_queue, 1);
      index++;    
    }
    
  } /* while */  
	
  /* if we failed to find a command of the correct size */
  if(command_name_complete == FALSE)
  {
	  /* reset the read_next_byte to what it was before we started */
	  in_queue->read_next_byte = temp_read_index;
  }
}

/* this function reads the queue for a parameter (space seperated) */	
BOOL get_command_param_text(Queue *in_queue, char *param_found_text, int max_size_of_param, BOOL nl_terminated)
{
	int index = 0;
	BOOL found_param = TRUE;
	
	/* read from in_queue a parameter - the expected format is */
	/* PARAM_TEXT\n or PARAM_TEXT PARAM_TEXT\n etc             */
	/* the last PARAM_TEXT is terminated by \n                 */
	
	/* clear the space to copy param into */
	memset(param_found_text, 0, max_size_of_param);
	
	/* read from current in_queue read pointer to first space or \n found */
	while( (get_current_read_byte(in_queue) != ' ') &&
		   (get_current_read_byte(in_queue) != '\n') && 
		   (index < max_size_of_param) )	
	{
		param_found_text[index] = get_current_read_byte(in_queue);
		increment_read_next_index(in_queue, 1);
		index++;
	}

	/* check we found a space or \n terminated parameter that we could copy into the space provided */
	if((index == max_size_of_param) && 
	   (get_current_read_byte(in_queue) != ' ') &&  
	   (get_current_read_byte(in_queue) != '\n'))
	{
		/* the parameter was too large or was not space or \n terminated */
		found_param = FALSE;
    	memset(param_found_text, 0, max_size_of_param);
	}
	/* was the parameter we got space terminated as expected */
	else if((get_current_read_byte(in_queue) == ' ') && (nl_terminated == FALSE))
	{
		/* yes was space terminated so skip space */
		increment_read_next_index(in_queue, 1);
	}
	/* was the parameter \n terminated as expected */
	else if((get_current_read_byte(in_queue) == '\n') && (nl_terminated == TRUE))
	{
		/* yes was \n terminated so skip the \n */
		increment_read_next_index(in_queue, 1);
	}
	else /* the parameter was not terminated as we expected so assume error in parameters of command */
	{
		found_param = FALSE;
    	memset(param_found_text, 0, max_size_of_param);
	}
	
	return(found_param);
}

/***************************************************************/
/*                                                             */
/* Built in Simple Commands                                    */
/*                                                             */
/***************************************************************/


/* unknown command - tick is ignored as all output in one call */
BOOL command_unknown(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "Unknown Command\n";

	/* write response to cpc */
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function complete */
	return( TRUE );
}

/* ping command - tick is ignored as all output in one call */
BOOL command_ping(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "PONG!\n";

	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function completed */
	return( TRUE );
}

/* this command operates over several ticks - first sending the message to the cpc     */
/* second tick actually running the shutdown command so message has time to get to cpc */
BOOL command_shutdown(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "SHUTDOWN IN 10 SECONDS\n";

	/* if we are at the first tick (tick=0) */
	if(tick == 0)
	{
		/* write the OK response to the out_queue for sending to client */
		write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	}
	else
	{
		#ifdef BUILD_ON_PC
			printf("Running Shutdown(Fake): %d\n", system("/bin/pwd"));
		#else
			printf("Running Shutdown: %d\n", system("sudo /sbin/shutdown -h now"));
		#endif
		
		return( TRUE );
	}
	
	/* return FALSE as command is not complete and we need to do */
	/* the shutdown in the next tick                             */
	return( FALSE );
}

/* time command - tick is ignored as all output in one call */
BOOL command_time(Queue *in_queue, Queue *out_queue, int tick)
{
	char current_time[40];
	
	time_t rawtime;
	struct tm * timeinfo;
	
	memset(current_time, 0, 40);
	
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	
	/* format the time response */
	sprintf(current_time, "%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	
	/* write the current_time response to the out_queue for sending to client */
	write_data_to_packet(out_queue, (BYTE *) current_time, strlen(current_time), TextCommand);
	
	return( TRUE );
}

/* date command - tick is ignored as all output in one call */
BOOL command_date(Queue *in_queue, Queue *out_queue, int tick)
{
	char current_date[40];
	
	time_t rawtime;
	struct tm * timeinfo;
	
	memset(current_date, 0, 40);
	
	time( &rawtime );
	timeinfo = localtime( &rawtime );

	/* format the date response */
	sprintf(current_date, "%02d:%02d:%04d\n", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900);
	
	/* write the current_date to the out_queue for sending to client */
	write_data_to_packet(out_queue, (BYTE *) current_date, strlen(current_date), TextCommand);
	
	return( TRUE );
}

/* reset command - tick is ignored as all output in one call */
BOOL command_reset(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "OK\n";

	if(TRON)
	{
    	printf("Reset Queues, MemStore and Command Processor\n");
	}
	
	/* reset the queues */
	queues_reset(in_queue, out_queue);
	/* reset the memstore */
	memstore_reset();		
	/* reset the command processor */
	command_processor_init();
	
	/* write the OK response to the out_queue for sending to client */
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	return( TRUE );
}

/* help command - tick is ignored as all output in one call */
BOOL command_help(Queue *in_queue, Queue *out_queue, int tick)
{
	char command_name_text[MAX_COMMAND_NAME_LENGTH];

	BOOL is_command_complete = FALSE;

	#ifdef DEBUG_MCP_IF
		/* if debugging then clear out_queue so we can read output of command clearly */
		init_queue(out_queue);
	#endif
	
	/* if previous byte read was \n then no parameter so just do HELP help text */
	if(get_previous_read_byte(in_queue) == '\n' )
	{
		is_command_complete = command_defs[Help].help_function(in_queue, out_queue);
	}
	else if(get_command_param_text(in_queue, command_name_text, MAX_COMMAND_NAME_LENGTH, TRUE))
	{
		/* read the parameter (the command name you want help on) */
		Command commandID = get_commandID_from_string( command_name_text );
	
		/* if the command found was unknown then get the HELP help text */
		if(commandID == Unknown)
		{
			is_command_complete = command_defs[Help].help_function(in_queue, out_queue);
		}
		else /* get the help for the identified command */
		{
			is_command_complete = command_defs[commandID].help_function(in_queue, out_queue);		
		}
	}
	else
	{
		/* failed to read the parameter with the help command (the command name that you want help on) */
		/* so lets return help on the HELP command */
		is_command_complete = command_defs[Help].help_function(in_queue, out_queue);
	}
	
	return(is_command_complete);
}

/* this command operates over several ticks - first sending the message to the cpc     */
/* second tick actually running the reboot command so message has time to get to cpc */
BOOL command_reboot(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "Rebooting PI IN 3 SECONDS\n";

	/* if we are at the first tick (tick=0) */
	if(tick == 0)
	{
		/* write the OK response to the out_queue for sending to client */
		write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	}
	else
	{
		#ifdef BUILD_ON_PC
			printf("Running Reboot(Fake): %d\n", system("/bin/pwd"));
		#else
			printf("Running Reboot: %d\n", system("sudo /sbin/reboot"));
		#endif
		
		return( TRUE );
	}
	
	/* return FALSE as command is not complete and we need to do */
	/* the shutdown in the next tick                             */
	
	return( FALSE );
}

/* tron command - tick is ignored as all output in one call */
BOOL command_tron(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "OK\n";

	TRON = TRUE;
	
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function completed */
	return( TRUE );
}

/* troff command - tick is ignored as all output in one call */
BOOL command_troff(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "OK\n";

	TRON = FALSE;
	
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function completed */
	return( TRUE );
}


/*************************************************************************************/
/*                                                                                   */
/* These commands control moving pages of memory in/out of the CPC to/from the pi    */
/* Using allocation_id's you can have a number (default 20) different areas of       */
/* of memory allocated and you can request and store pages to each area seperately   */
/* When you work with a memory allocation you work with RALLOC and SALLOC and work   */
/* with page size as specified in the CALLOC command e.g. if you specified           */
/* CALLOC 5 8\n then you will have 5 pages of 8K and each RALLOC or SALLOC will work */
/* with an 8KB page at a time.                                                       */
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
/* IALLOC is used to insert some memory (up to a CALLOC page_size) into the pi       */
/*                                                                                   */
/* XALLOC is used to out a section of memory from the allocation in the pi memory    */
/*                                                                                   */
/* PILALLOC is used by the CPC to tell the PI to load a file into its memory         */
/*          The CPC can then page the loaded file in and out of CPC memory from      */
/*          the PI's memory                                                          */
/*                                                                                   */
/* PISALLOC is used by the CPC to tell the PI to save its memory back to             */
/*          file on the PI storage                                                   */
/*                                                                                   */
/* See each function for format of the messages                                      */
/*                                                                                   */
/*************************************************************************************/

/* create allocation command - tick is ignored as all output in one call */
BOOL command_create_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                   */
	/* CALLOC alloc_page_number SIZE_OF_alloc_IN_KB\n       */
	/* example:                                             */
	/* CALLOC 5 8\n which requests 5 x 8KB of memory        */
	/* MAX alloc_page_number is 9999                        */
	/* MAX SIZE_OF_alloc_IN_KB is 32                        */
	/* return values:                                       */
	/* CALLOC ERROR 1\n when parameters are incorrect       */
	/* CALLOC ERROR 2\n when pi can't allocate memory       */
	/* CALLOC ALLOCATION_ID\n when complete successfully    */
	/* The ALLOCATION_ID is a positive int (1..20)          */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	char *response1_text = "CALLOC ERROR 1\n"; /* parameter error         */
	char *response2_text = "CALLOC ERROR 2\n"; /* memory allocation error */
	
	char alloc_page_number_text[5];
	char alloc_alloc_size_in_kb_text[3];

	if(get_command_param_text(in_queue, alloc_page_number_text, 5, FALSE) && 
	   get_command_param_text(in_queue, alloc_alloc_size_in_kb_text, 3, TRUE))
	{
		/* we got the two parameters */
		int alloc_page_number = atoi(alloc_page_number_text);
		int alloc_size_in_kb = atoi(alloc_alloc_size_in_kb_text);
		
		if(TRON)
		{
			printf("CALLOC Requested %d pages of %dKB\n", alloc_page_number, alloc_size_in_kb);
		}

		/* check page_size is 0 or larger than 32K */
		if((alloc_size_in_kb == 0) || (alloc_size_in_kb > 32))
		{
			/* send message back to the CPC page size too large */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		}
		/* check page number is 0 or larger than 9999 */
		else if((alloc_page_number == 0) || (alloc_page_number > 9999 ))
		{
			/* send message back to the CPC number pages too large */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		}
		else
		{
			/* init the memstore manager with the number and size of pages */
			int allocation_id = memstore_create_allocation_id(alloc_page_number,alloc_size_in_kb);
			
			if(allocation_id == 0)
			{
				/* send message mem full back to the CPC i.e. they are asking for too much */
				write_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text), TextCommand);
			}
			else
			{
				char output_text[20];
				
				/* send message back to the CPC informing them of allocation reference created */
				sprintf(output_text,"CALLOC %d\n", allocation_id);
				write_data_to_packet(out_queue, (BYTE *) output_text, strlen(output_text), TextCommand);
			}			
		}
	}
	else /* didnt read the two parameters */
	{
		/* send an error message back to the CPC */
		write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
		/* you need to read to the next packet and skip erroronous command data */
		queue_skip_to_next_packet(in_queue);
	}
		
	return( TRUE );
}

/* free alloc command - tick is ignored as all output in one call */
BOOL command_free_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                */
	/* FALLOC ALLOC_ID\n                                 */
	/* example:                                          */
	/* FALLOC 1\n which releases all memory associated   */
	/* with allocation_id 1.  The allocation id is what  */
	/* what was returned after doing a CALLOC request    */
	/* NOTE: When you have allocated memory with CALLOC  */
	/* and are now using FALLOC to release it - you are  */
	/* releaseing all the memory for that allocation_ID  */
	/* i.e. you can't release just part of the memory.   */
	/* return values:                                    */
	/* FALLOC ERROR 1\n when parameters are incorrect    */
    /* FALLOC OK ALLOC_ID\n when all memory for that allocation */
	/* id has been released in the pi                    */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	char *response1_text = "FALLOC ERROR 1\n"; /* parameters are invalid */
	
	char allocation_id_text[3];

	/* if we can read the allocation_id parameter from the in_queue */
	if(get_command_param_text(in_queue, allocation_id_text, 3, TRUE))
	{
		int allocation_id = atoi(allocation_id_text);
		
		if(TRON)
		{
			printf("FALLOC Free alloc_ID %d\n", allocation_id);
		}

		/* we read the parameter so free the allocation_id */
		if(memstore_free_allocation_id( allocation_id ))
		{
			char response2_text[22];

			/* we freed the allocation_id */
			sprintf(response2_text, "FALLOC OK %d", allocation_id);
			
			/* send a response back to the CPC */
			write_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text), TextCommand);
			
		}
		else /* failed to free the allocation_id as the allocation_id from CPC was invalid */
		{
			/* send an error message back to the CPC */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		}
	}
	else /* didnt read the parameter */
	{
		/* send an error message back to the CPC */
		write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
		/* you need to read to the next packet and skip erroronous command data */
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}
	
/* retrieve memory page from alloc - tick is ignored as all output in one call */
BOOL command_retrieve_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                 */
	/* RALLOC ALLOCATION_ID alloc_page_number\n           */
	/* example:                                           */
	/* RALLOC 1 0\n which retrieve the first page(from 0) */
	/* for memory in allocation_id 1                      */
	/* return values:                                     */
	/* RALLOC OK 1 0\nBINARY_DATA                         */
	/* The binary data is the number of KB you specified  */
	/* in your CALLOC command.                            */
	/* RALLOC ERROR 1\n when parameters are incorrect     */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	char *response1_text = "RALLOC ERROR 1\n";
	
	char allocation_id_text[3];
	char alloc_page_number_text[5];

	/* if we can read the parameters from the in_queue */
	if(get_command_param_text(in_queue, allocation_id_text, 3, FALSE) &&
	   get_command_param_text(in_queue, alloc_page_number_text, 5, TRUE))
	{
		int allocation_id = atoi(allocation_id_text);
		int alloc_page_number = atoi(alloc_page_number_text);
		
		if(TRON)
		{
			printf("RALLOC alloc_ID %d page_number %d\n", allocation_id, alloc_page_number);
		}

		/* check if alloc page number is greater than 9999 */
		if(alloc_page_number > 9999)
		{
			/* send message back to the CPC if number pages is wrong */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		}
		else
		{
			int bytes_to_retrieve = 0;
			BYTE *page_data_pointer = memstore_retrieve_page_of_data(allocation_id, alloc_page_number, &bytes_to_retrieve);			
			
			if(bytes_to_retrieve > 0)
			{
				char response[36];
				
				int string_response_length = 0;
				
				/* format the response header */
				sprintf(response, "RALLOC OK %d %d\n", allocation_id, alloc_page_number);
				string_response_length = strlen(response);
				
				/* is there space in the out_queue to build the TextCommand packet response and the BinaryData packet response */
				if(is_queue_space_available(out_queue, string_response_length + PACKET_OVERHEAD +  bytes_to_retrieve + PACKET_OVERHEAD))
				{
					/* write out the TextCommand response */
					write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
					/* now write the binary data response (containing the requested data) */
					write_data_to_packet(out_queue, page_data_pointer, bytes_to_retrieve, BinaryData);
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
				write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
			}
		}
	}
	else /* didnt read the parameters */
	{
		/* send an error message back to the CPC */
		write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
		/* you need to read to the next packet and skip erroronous command data */
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}
	
/* store pages command - tick is ignored as all output in one call */
BOOL command_store_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                              */
	/* SALLOC ALLOCATION_ID alloc_page_number\n                        */
	/* example:                                                        */
	/* SALLOC 1 0\n                                                    */
    /*   BINARY_DATA follows in the next packet where the              */
    /*   length of BINARY_DATA is the page size used in                */       
    /*   CALLOC creation                                               */
	/* return values:                                                  */
	/* SALLOC OK ALLOCATION_ID alloc_page_number\n                     */
	/* e.g. SALLOC OK 1 0\n                                            */
	/* SALLOC ERROR 1\n when parameters are incorrect                  */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	char *response1_text = "SALLOC ERROR 1\n";
	
	char allocation_id_text[3];
	char alloc_page_number_text[5];
	
	BOOL skip_next_binary_packet = TRUE;

	/* if we can read the parameters from the in_queue */
	if(get_command_param_text(in_queue, allocation_id_text, 3, FALSE) &&
	   get_command_param_text(in_queue, alloc_page_number_text, 5, TRUE))
	{
		int allocation_id = atoi(allocation_id_text);
		int alloc_page_number = atoi(alloc_page_number_text);
		
		BYTE buffer_of_alloc_data[MAX_PAGE_SIZE];
		int page_size = memstore_get_page_size_bytes(allocation_id);

		if(TRON)
		{
			printf("SALLOC alloc_ID %d page_number %d\n", allocation_id, alloc_page_number);
		}

		/* skip the terminator of the TextCommand packet - ready to read the BinaryData packet */
		queue_skip_terminator_of_packet(in_queue);
										
		/* clear the page_buffer */
		memset(buffer_of_alloc_data, 0, MAX_PAGE_SIZE);

		/* if the page has no size then allocation_id is wrong */
		if(page_size == 0)
		{
			/* send message back to the CPC as allocation_id is invalid as no page_size */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		}
		/* check if page number is greater than 9999 */
		else if(alloc_page_number > 9999)
		{
			/* send message back to the CPC if number pages is wrong */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		}
		/* is the next packet contain BinaryData */
		else if(is_packet_available(in_queue) == FALSE)
		{
			/* we do not have a complete BinaryData packet in the in_queue for that page */
			/* return with FALSE so the command will be retried once we have more data   */
			if(TRON)
			{
				printf("SALLOC binary data incomplete - waiting on more to be received and will retry\n");
			}
					   
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
			char response[36];
						
			if(TRON)
			{
				printf("Wrote to memory store alloc_ID %d, page_number %d\n", allocation_id, alloc_page_number);
			}
			
			/* format the response header */
			sprintf(response, "SALLOC OK %d %d\n", allocation_id, alloc_page_number);
			
			/* send the complete response to the out_queue */
			write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
			
			skip_next_binary_packet = FALSE;
		}
		else /* failed to store the requested page BINARY data to page of allocation_id */
		{
			/* send message back to the CPC that page for that allocation id does not exist */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
			
			skip_next_binary_packet = FALSE;
		}
	}
	else /* didnt read the parameters */
	{
		/* send an error message back to the CPC */
		write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);		
		
		/* you need to read to the next packet and skip erroronous command data */
		queue_skip_to_next_packet(in_queue);
	}

	/* if we need to skip the binary packet as the TextCommand was in error */
	if(skip_next_binary_packet)
	{
		/* skip the next BinaryData packet as its aimed at this command */
		increment_read_next_index(in_queue, 1);
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}

/* insert a section of memory command - tick is ignored as all output in one call */
BOOL command_insert_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                                          */
	/* IALLOC ALLOCATION_ID INSERT_AT INSERT_LENGTH\n                              */
	/* example:                                                                    */
	/* IALLOC 1 20 50\n                                                            */
    /* Where 1 is ALLOC_ID and 20 is insert at 20 bytes from start of ALLOC_ID     */
	/* memory and insert 50 bytes                                                  */
    /* BINARY_DATA follows in the next packet of INSERT_LENGTH                     */
	/* return values:                                                              */
	/* IALLOC OK ALLOCATION_ID\n                                                   */
	/* IALLOC ERROR 1\n when parameters are incorrect                              */
	/* IALLOC ERROR 2\n unable to allocate memory on pi                            */
	/* IALLOC ERROR 3\n insert_at beyond end of allocation memory                  */
	
	/* The purpose of this command is to insert a defined section of memory into the allocation at a specific point */
	
	BOOL skip_next_binary_packet = TRUE;

	char *response1_text = "IALLOC ERROR 1\n";
	char *response2_text = "IALLOC ERROR 2\n";
	char *response3_text = "IALLOC ERROR 3\n";
	
	char allocation_id_text[3];
	char alloc_insert_at_text[12];
	char alloc_insert_length_text[12];

	/* if we can read the parameters from the in_queue */
	if(get_command_param_text(in_queue, allocation_id_text, 3, FALSE) &&
	   get_command_param_text(in_queue, alloc_insert_at_text, 12, FALSE) &&
	   get_command_param_text(in_queue, alloc_insert_length_text, 12, TRUE))
	{
		int allocation_id = atoi(allocation_id_text);
		int alloc_insert_at = atoi(alloc_insert_at_text);
		int alloc_insert_length = atoi(alloc_insert_length_text);
		
		BYTE *buffer_of_alloc_data = calloc(alloc_insert_length + 1, 1);
		
		memset(buffer_of_alloc_data, 0, alloc_insert_length + 1);
		
		/* skip the terminator of the TextCommand packet - ready to read the BinaryData packet */
		queue_skip_terminator_of_packet(in_queue);
										
		if(TRON)
		{
			printf("IALLOC alloc_ID %d insert_at %d insert_length %d\n", allocation_id, alloc_insert_at, alloc_insert_length);
		}

		if(buffer_of_alloc_data == NULL)
		{
			/* unable to allocate memory on pi, send error to cpc */
			write_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text), TextCommand);
		}
		/* check if alloc_insert_at + alloc_insert_length is within allocation memory bounds */
		else if((alloc_insert_at + alloc_insert_length) > memstore_get_alloc_size_bytes(allocation_id))
		{
			/* outside memory allocation so return error to CPC */
			write_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text), TextCommand);
		}
		/* is the next packet contain BinaryData */
		else if(is_packet_available(in_queue) == FALSE)
		{
			/* we do not have a complete BinaryData packet in the in_queue for that page */
			/* return with FALSE so the command will be retried once we have more data   */
			if(TRON)
			{
				printf("SALLOC binary data incomplete - waiting on more to be received and will retry\n");
			}
					   
			return( FALSE );
		}
		/* if we can read BinaryData packet with a page_size number of bytes from in_queue */
		else if(get_data_from_packet(in_queue, buffer_of_alloc_data, alloc_insert_length) == FALSE)
		{
			/* the PACKET_SIZE and page_size do not match i.e. the BinaryData packet */
			/* contain too much or too little data to fit the page_size of the page  */
			if(TRON)
			{
				printf("SALLOC binary data size error, Expecting %d but got %d bytes\n", alloc_insert_length, get_packet_size(in_queue));
			}
			
			skip_next_binary_packet = FALSE;
		}
		/* did we store the binary data */
		else if(memstore_insert_data(allocation_id, alloc_insert_at, alloc_insert_length, buffer_of_alloc_data) == TRUE)
		{
			char response[24];
			
			if(TRON)
			{
				printf("Wrote to memory store alloc_ID %d\n", allocation_id);
			}
			
			/* format the response header */
			sprintf(response, "IALLOC OK %d\n", allocation_id);
			
			/* send the complete response to the out_queue */
			write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
			
			skip_next_binary_packet = FALSE;
		}
		else /* failed to store the insert data */
		{
			/* send message back to the CPC that allocation id does not exist */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
			
			skip_next_binary_packet = FALSE;
		}		
		
		/* free allocated memory (if any was) */
		if(buffer_of_alloc_data)
		{
			free(buffer_of_alloc_data);
		}
	}
	else /* didnt read the parameters */
	{
		/* send an error message back to the CPC */
		write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
		/* you need to read to the next packet and skip erroronous command data */
		queue_skip_to_next_packet(in_queue);
	}
	
	/* if we need to skip the binary packet as the TextCommand was in error */
	if(skip_next_binary_packet)
	{
		/* skip the next BinaryData packet as its aimed at this command */
		increment_read_next_index(in_queue, 1);
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}

/* cut a section of memory command - tick is ignored as all output in one call */
BOOL command_cut_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                                            */
	/* XALLOC ALLOCATION_ID START_CUT LENGTH_OF_CUT\n                                */
	/* example:                                                                      */
	/* XALLOC 1 20 50\n Will cut out 50_bytes of data from position 20 in allocation */
	/* return values:                                                                */
	/* XALLOC OK ALLOCATION_ID\n                                                     */
	/* XALLOC ERROR 1\n when parameters are incorrect                                */
	/* XALLOC ERROR 2\n position+cut_length go beyond end of allocation memory       */
	
	/* The purpose of this command is to cut out a defined section of memory in allocation at a specific point */
	
	char *response1_text = "XALLOC ERROR 1\n";
	char *response2_text = "XALLOC ERROR 2\n";
	
	char allocation_id_text[3];
	char alloc_cut_at_text[12];
	char alloc_cut_length_text[12];

	/* if we can read the parameters from the in_queue */
	if(get_command_param_text(in_queue, allocation_id_text, 3, FALSE) &&
	   get_command_param_text(in_queue, alloc_cut_at_text, 12, FALSE) &&
	   get_command_param_text(in_queue, alloc_cut_length_text, 12, TRUE))
	{
		int allocation_id = atoi(allocation_id_text);
		int alloc_cut_at = atoi(alloc_cut_at_text);
		int alloc_cut_length = atoi(alloc_cut_length_text);
		
		if(TRON)
		{
			printf("XALLOC alloc_ID %d cut_at %d cut_length %d\n", allocation_id, alloc_cut_at, alloc_cut_length);
		}

		/* check if alloc_cut_at + alloc_cut_length is within allocation memory bounds */
		if((alloc_cut_at + alloc_cut_length) > memstore_get_alloc_size_bytes(allocation_id))
		{
			/* outside memory allocation so return error to CPC */
			write_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text), TextCommand);
		}
		/* did we store the binary data */
		else if(memstore_cut_data(allocation_id, alloc_cut_at, alloc_cut_length) == TRUE)
		{
			char response[24];
			
			/* format the response header */
			sprintf(response, "XALLOC OK %d\n", allocation_id);
			/* send the complete response to the out_queue */
			write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
		}
		else /* failed to store the insert data */
		{
			/* send message back to the CPC that allocation id does not exist */
			write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		}		
	}
	else /* didnt read the parameters */
	{
		/* send an error message back to the CPC */
		write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
		/* you need to read to the next packet and skip erroronous command data */
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}

/* load file into allocation command - tick is ignored as all output in one call */
BOOL command_load_alloc(Queue *in_queue, Queue *out_queue, int tick)
{	
	/* format of request:                                      */
	/* PILALLOC ALLOCATION_ID /PATH/FILENAME\n                 */
	/* example:                                                */
	/* PILALLOC 1 /home/pi/mcp_pi/HowThisWorks.txt\n           */
	/* return values:                                          */
	/* PILALLOC ALLOCATION_ID FILE_SIZE\n                      */
	/* PILALLOC ERROR 1\n when parameters are incorrect        */
	/* PILALLOC ERROR 2\n unable to load file                  */
	/* PILALLOC ERROR 3\n file to large to fit in memory       */
	/*                    allocation                           */
	
	char *response1_text = "PILALLOC ERROR 1\n"; /* parameter error               */
	char *response2_text = "PILALLOC ERROR 2\n"; /* unable to load file from path */
	char *response3_text = "PILALLOC ERROR 3\n"; /* file too large to load        */
	
	char file_path[MAX_PATH_SIZE_TEXT];
	char allocation_id_text[3];
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	/* clear the page_buffer */
	memset(file_path, 0, MAX_PATH_SIZE_TEXT);

	/* if we can read the parameters from the in_queue */
	if(get_command_param_text(in_queue, allocation_id_text, 3, FALSE) &&
	   get_command_param_text(in_queue, file_path, MAX_PATH_SIZE_TEXT, TRUE))
	{
		int allocation_id = atoi(allocation_id_text);
		int file_size = 0;
		BOOL file_to_large = FALSE;

		if(TRON)
		{
			printf("PILALLOC FILE_PATH %s\n", file_path);
		}

		/* did we store read the file into allocation memory */
		if(memstore_load_data(allocation_id, file_path, &file_size, &file_to_large) == TRUE)
		{
			char response[36];
			
			/* format the response header */
			sprintf(response, "PILALLOC %d %d\n", allocation_id, file_size);
			/* send the complete response to the out_queue */
			write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
		}
		else if( file_to_large )
		{
			/* send message back to the CPC that file does not fit in allocation memory */
			write_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text), TextCommand);
		}
		else /* failed to find/read the file */
		{
			/* send message back to the CPC that file cant be found */
			write_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text), TextCommand);
		}
	}
	else /* didnt read the parameters */
	{
		/* send an error message back to the CPC */
		write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
		/* you need to read to the next packet and skip erroronous command data */
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}

/* save allocation memory to file command - tick is ignored as all output in one call */
BOOL command_save_alloc(Queue *in_queue, Queue *out_queue, int tick)
{
	/* format of request:                                       */
	/* PISALLOC ALLOCATION_ID /PATH/FILENAME FILESIZE\n         */
	/* FILESIZE is size you want to save from memory allocation */
	/* example:                                                 */
	/* PISALLOC 1 /home/pi/mcp_pi/HowThisWorks.txt 4086\n       */
	/* return values:                                           */
	/* PISALLOC ALLOCATION_ID OK\n                              */
	/* PILALLOC ERROR 1\n parameter error                       */
	/* PILALLOC ERROR 2\n unable to write to file               */
	
	/* this command uses parameters so it HAS to read those to remove them from the in_queue */
	
	char *response1_text = "PISALLOC ERROR 1\n"; /* parameter error         */
	char *response2_text = "PISALLOC ERROR 2\n"; /* unable to write to file */
	
	char file_path[MAX_PATH_SIZE_TEXT];
	char allocation_id_text[3];
	char file_size_text[MAX_FILE_SIZE_TEXT];
	
	/* clear the page_buffer */
	memset(file_path, 0, MAX_PATH_SIZE_TEXT);
	memset(file_size_text, 0, MAX_FILE_SIZE_TEXT);

	/* if we can read the parameters from the in_queue */
	if(get_command_param_text(in_queue, allocation_id_text, 3, FALSE) &&
	   get_command_param_text(in_queue, file_path, MAX_PATH_SIZE_TEXT, FALSE) &&
	   get_command_param_text(in_queue, file_size_text, MAX_FILE_SIZE_TEXT, TRUE) )
	{
		int allocation_id = atoi(allocation_id_text);
		int file_size = atoi(file_size_text);

		if(TRON)
		{
			printf("PISALLOC alloc_ID %d FILE_PATH %s FILE_SIZE %d\n", allocation_id, file_path, file_size);
		}

		/* did we save the file from allocation_id memory */
		if(memstore_save_data(allocation_id, file_path, file_size) == TRUE)
		{
			char response[26];
			
			/* format the response header */
			sprintf(response, "PISALLOC OK %d\n", allocation_id);
			/* send the complete response to the out_queue */
			write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
		}
		else /* failed to write to file */
		{
			/* send message back to the CPC unable to write to the file specified */
			write_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text), TextCommand);
		}
	}
	else /* didnt read the parameters */
	{
		/* send an error message back to the CPC */
		write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
		/* you need to read to the next packet and skip erroronous command data */
		queue_skip_to_next_packet(in_queue);
	}
	
	return( TRUE );
}


/********************************************************************/
/*                                                                  */
/* Help Functions for the command services                          */
/*                                                                  */
/********************************************************************/

/* simple built in commands help for basic services */
BOOL command_unknown_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rAn UNKNOWN command was entered\r\rTry entering HELP\\n.\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_ping_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rPING\r\rFunction: Used to test pi is responding to CPC commands.\r\rFormat: PING\\n\r\rReturn: PONG!\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}
	
BOOL command_shutdown_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rSHUTDOWN\r\rFunction: Command to shutdown the raspberry pi before powering off CPC.\rYou need to wait the number of seconds specified then check the pi is inactive    i.e. red led only on.\r\rFormat: SHUTDOWN\\n\r\rReturn: SHUTDOWN in x seconds\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_time_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rTIME\r\rFunction: Returns pi current time.  Will need time on pi to be set e.g. via ntp.\r\rFormat: TIME\\n\r\rReturn: HH:MM:SS\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
	/* command comepleted */
	return(TRUE);
}

BOOL command_date_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rDATE\r\rFunction: Returns pi current date.  Will need date on pi to be set e.g. via ntp.\r\rFormat: DATE\\n\r\rReturn: DD:MM:YYYY\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_reset_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rRESET\r\r";
	char *response2_text = "Function: Reset MCP data structures, flushes queues, closes files, frees        allocated memory etc.\r\rUse this to bring the MCP back to a clean known state.\r\r";
	char *response3_text = "Format: RESET\\n\r\r";
	char *response4_text = "Return: OK\\n\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_help_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rHELP\r\r";
	char *response2_text = "Function: Provides help on the COMMAND_NAME provided.\r\r";
	char *response3_text = "Help is available for the following commands:\r\r";
	char *response4_text = "\r\rFormat: HELP COMMAND_NAME\\n\r\r";
	char *response5_text = "Return: String of text terminated with \\n\r\n";
	
	int command_text_size = 0;

	/* get the command string text so we can determine the size */
	for(int index = 1; index < MaxCommands; index++ )
	{
		/* +1 for space after the command text */
		command_text_size = command_text_size + strlen(command_defs[index].command_text) + 1;
	}

	command_text_size = command_text_size + strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text) + strlen(response5_text);
	
	/* ensure space in out_queue */
	if(start_build_data_packet(out_queue, command_text_size, TextCommand))
	{
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		
		for(int index = 1; index < MaxCommands; index++ )
		{
			add_data_to_packet(out_queue, (BYTE *) command_defs[index].command_text, strlen(command_defs[index].command_text));
			add_data_to_packet(out_queue, (BYTE *) " ", 1);
		}

		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		add_data_to_packet(out_queue, (BYTE *) response5_text, strlen(response5_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	
	/* was command completed */
	return(function_complete);
}

BOOL command_reboot_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rREBOOT\r\rFunction: Command to reboot the raspberry pi.\rYou need to wait the number of seconds before the reboot starts.\r\rFormat: REBOOT\\n\r\rReturn: REBOOT PI in x seconds\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_tron_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rTRON\r\rFunction: Turns on debugging information sent to pi console.\r\rFormat: TRON\\n\r\rReturn: OK\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_troff_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rTROFF\r\rFunction: Turns off debugging information sent to pi console.\r\rFormat: TROFF\\n\r\rReturn: OK\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

/* help for commands that control moving pages of memory in/out of the CPC to/from the pi */
BOOL command_create_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rCALLOC\r\r";
	char *response2_text = "Function: Creates an ALLOCation of memory on the pi.\r\r";
	char *response3_text = "Format: CALLOC NUM_PAGES PAGE_SIZE\\n\r\r  Where NUM_PAGES is 1 to 9999\r  Where PAGE_SIZE is 1 to 32 (in KB)\r\r";
	char *response4_text = "Return: CALLOC ALLOC_ID\\n Where ALLOC_ID is 1 to 20.\rReturn: CALLOC ERROR 1\\n Invalid Parameters\rReturn: CALLOC ERROR 2\\n Can't allocate memory on pi\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_free_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rFALLOC\r\r";
	char *response2_text = "Function: Free Allocation of memory make by CALLOC.\r\r";
	char *response3_text = "Format: FALLOC ALLOC_ID\\n\r\r  Where ALLOC_ID is between 1 and 20\r\r";
	char *response4_text = "Return: FALLOC OK ALLOC_ID\\n\rReturn: FALLOC ERROR 1\\n Invalid Parameters\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_retrieve_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rRALLOC\r\r";
	char *response2_text = "Function: Retrieve a memory page (size specified in CALLOC) from the Allocation.\r\r";
	char *response3_text = "Format: RALLOC ALLOC_ID PAGE_NUM\\n\r\r  Where ALLOC_ID is between 1 and 20\r  Where PAGE_NUM is 0 to 9999 (depends on CALLOC params)\r\r";
	char *response4_text = "Return: RALLOC OK 1 0\\n. The BINARY_DATA will follow in the next packet\r  The BINARY_DATA is xKB long according to CALLOC\rReturn: RALLOC ERROR 1\\n Invalid Parameters\r\r";
	char *response5_text = "Note: An ALLOCation consists of x pages of KB specified in CALLOC.\rRALLOC and SALLOC are used to page in and out a xKB page of that allocation at a time\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text) + strlen(response5_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		add_data_to_packet(out_queue, (BYTE *) response5_text, strlen(response5_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_store_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rSALLOC\r\r";
	char *response2_text = "Function: Store a page of memory back to the allocation on pi.\r\r";
	char *response3_text = "Format: SALLOC ALLOC_ID PAGE_NUM\\n\r  Where ALLOC_ID is between 1 and 20\r  Where PAGE_NUM us 0 to 9999 (depends on CALLOC params)\r  The BINARY_DATA of xKB of data then follows in the next packet.\r\r";
	char *response4_text = "Return: SALLOC OK ALLOC_ID PAGE_NUM\\n\rReturn: SALLOC ERROR 1\\n Invalid Parameters\r";
	char *response5_text = "Note: An ALLOCation consists of x pages of KB specified in CALLOC.\rRALLOC and SALLOC are used to page in and out a xKB page of that allocation at a time\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text) + strlen(response5_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		add_data_to_packet(out_queue, (BYTE *) response5_text, strlen(response5_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_insert_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rIALLOC\r\r";
	char *response2_text = "Function: Insert number of bytes into allocation at location.\r\r";
	char *response3_text = "Format: IALLOC ALLOC_ID INSERT_AT INSERT_LENGTH\\n\r  Where ALLOC_ID is between 1 and 20\r  Where INSERT_AT is number of bytes from start of allocation\r  Where INSERT_LENGTH is the number of bytes to insert.  The BINARY_DATA to       insert follows in the next packet.\r";
	char *response4_text = "Return: IALLOC OK ALLOC_ID\\n\rReturn: IALLOC ERROR 1\\n Invalid Parameters\rReturn: IALLOC ERROR 2\\n Failed to alloc memory on pi\rReturn: IALLOC ERROR 3\\n Does not fit in allocation size\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

	/* format of request:                                                            */
	/* XALLOC ALLOCATION_ID START_CUT LENGTH_OF_CUT\n                                */
	/* example:                                                                      */
	/* XALLOC 1 20 50\n Will cut out 50_bytes of data from position 20 in allocation */
	/* return values:                                                                */
	/* XALLOC OK ALLOCATION_ID\n                                                     */
	/* XALLOC ERROR 1\n when parameters are incorrect                                */
	/* XALLOC ERROR 2\n position+cut_length go beyond end of allocation memory       */
	
BOOL command_cut_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rXALLOC\r\r";
	char *response2_text = "Function: Cut a number of bytes from a position in allocation.\r\r";
	char *response3_text = "Format: XALLOC ALLOC_ID CUT_AT CUT_LENGTH\\n\r  Where ALLOC_ID is between 1 and 20\r  Where CUT_AT is number of bytes from start of allocation to cut from\r  Where CUT_LENGTH is the number of bytes to cut\r";
	char *response4_text = "Return: XALLOC OK ALLOC_ID\\n\rReturn: XALLOC ERROR 1\\n Invalid Parameters\rReturn: XALLOC ERROR 2\\n Cut length goes beyond allocation size\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_load_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rPILALLOC\r\r";
	char *response2_text = "Function: Load the allocation memory from a file on pi.\r\r";
	char *response3_text = "Format: PILALLOC ALLOC_ID /ABS_FILE_PATH/FILE_NAME\\n\r  Where ALLOC_ID is between 1 and 20\r";
	char *response4_text = "Return: PILALLOC ALLOC_ID FILE_SIZE\\n\rReturn: PILALLOC ERROR 1\\n Invalid Parameters\rReturn: PILALLOC ERROR 2\\n Unable to read file\rReturn: PILALLOC ERROR 3\\n File to large for allocation\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_save_alloc_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rPISALLOC\r\r";
	char *response2_text = "Function: Save an allocation of memory to file on pi.\r\r";
	char *response3_text = "Format: PISALLOC ALLOC_ID /ABS_FILE_PATH/FILE_NAME FILE_SIZE\\n\r  Where ALLOC_ID is between 1 and 20\r  Where FILE_SIZE is number of bytes to write\r";
	char *response4_text = "Return: PISALLOC ALLOC_ID OK\\n\rReturn: PISALLOC ERROR 1\\n Invalid Parameters\rReturn: PISALLOC ERROR 2\\n Unable to write to file\r\n";
	
	/* write response to out_queue */
	if(start_build_data_packet(out_queue, strlen(response1_text) + strlen(response2_text) + strlen(response3_text) + strlen(response4_text), TextCommand))
	{	
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}
