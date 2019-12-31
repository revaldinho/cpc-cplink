/************************************************************/
/*                                                          */
/* The implementation of commands (services) that the pi    */
/* provides to the CPC                                      */
/*                                                          */
/************************************************************/

#include <string.h>
#include <stdio.h>
#include "command.h"
#include "packet.h"
#include "command_general.h"
#include "command_memory.h"
#include "command_file.h"

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
		{Unknown,          "UNKNOWN",      command_unknown,            command_unknown_help,             100,   30}, 
		{Ping,             "PING",         command_ping,               command_ping_help,                100,   30},
		{Shutdown,         "SHUTDOWN",     command_shutdown,           command_shutdown_help,            3000,  30},
		{Time,             "TIME",         command_time,               command_time_help,                100,   30},
		{Date,             "DATE",         command_date,               command_date_help,                100,   30},
		{Reset,            "RESET",        command_reset,              command_reset_help,               100,   30},
		{Help,             "HELP",         command_help,               command_help_help,                100,   1024},
		{Reboot,           "REBOOT",       command_reboot,             command_reboot_help,              3000,  30},
		{Tron,             "TRON",         command_tron,               command_tron_help,                100,   30},
		{Troff,            "TROFF",        command_troff,              command_troff_help,               100,   30},
		{Version,          "VERSION",      command_version,            command_version_help,             100,   30},
		{WifiIP,           "WIFIIP",       command_wifi_ip,            command_wifi_ip_help,             100,   MAX_WIFI_LINE_LENGTH},
		{ShellExec,        "SHELLEXEC",    command_shellexec,          command_shellexec_help,           100,   2048},
		{Spare2,           "SPARE2",       command_unknown,            command_unknown_help,             100,   30},
		{Spare3,           "SPARE3",       command_unknown,            command_unknown_help,             100,   30},
		{Spare4,           "SPARE4",       command_unknown,            command_unknown_help,             100,   30},
		{Spare5,           "SPARE5",       command_unknown,            command_unknown_help,             100,   30},
		{Spare6,           "SPARE6",       command_unknown,            command_unknown_help,             100,   30},
		{Spare7,           "SPARE7",       command_unknown,            command_unknown_help,             100,   30},
		{Spare8,           "SPARE8",       command_unknown,            command_unknown_help,             100,   30},
		{Spare9,           "SPARE9",       command_unknown,            command_unknown_help,             100,   30},

		/* Memory API Commands */
		{CreateAlloc,      "CALLOC",       command_create_alloc,       command_create_alloc_help,        100,   30},
		{FreeAlloc,        "FALLOC",       command_free_alloc,         command_free_alloc_help,          100,   30},
		{RetrieveAlloc,    "RALLOC",       command_retrieve_alloc,     command_retrieve_alloc_help,      500,   30},
		{StoreAlloc,       "SALLOC",       command_store_alloc,        command_store_alloc_help,         100,   30},
		{InsertAlloc,      "IALLOC",       command_insert_alloc,       command_insert_alloc_help,        100,   30},
		{CutAlloc,         "XALLOC",       command_cut_alloc,          command_cut_alloc_help,           100,   30},
		{LoadAlloc,        "PILALLOC",     command_load_alloc,         command_load_alloc_help,          100,   30},
		{SaveAlloc,        "PISALLOC",     command_save_alloc,         command_save_alloc_help,          100,   30},

		/* File Commands */
		{GetFile,          "GETFILE",      command_get_file,           command_get_file_help,            100,   30},
		{PutFile,          "PUTFILE",      command_put_file,           command_put_file_help,            100,   30},
		{GetMemFile,       "GETMEMFILE",   command_get_mem_file,       command_get_mem_file_help,        100,   30},
		{PutMemFile,       "PUTMEMFILE",   command_put_mem_file,       command_put_mem_file_help,        100,   30}

	};

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

/* get the command string name from the command id */
const char *get_command_name_from_id(Command command_id)
{
	return(command_defs[command_id].command_text);
}

/************************************************************/
/*                                                          */
/* Functions for working with Text Commands                 */
/*                                                          */
/************************************************************/

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

/* get the text of the command name */
BOOL get_command_text(Queue *in_queue, char *command_string)
{
	int index = 0;

	BOOL command_name_complete = FALSE;

	int packet_body_size = 0;

	/* clear the command string */
	memset(command_string, 0, MAX_COMMAND_NAME_LENGTH);

	/* +6 to skip +++ two_byte_size one_byte_packet_type */
	increment_read_next_index(in_queue, FRONT_DELIMITER_OVERHEAD);
	
	/* read packet size low byte */
	packet_body_size = get_current_read_byte(in_queue);
	/* increment read index */
	increment_read_next_index(in_queue, 1);
	/* read packet size high byte and add to low */
	packet_body_size = ((get_current_read_byte(in_queue) << 8) + packet_body_size) - FRONT_PACKET_OVERHEAD;
	/* increment read index */
	increment_read_next_index(in_queue, 1);

	/* skip over packet_type */
	increment_read_next_index(in_queue, 1);

	/* if we got a short command (1 byte or just \n) */
	if((get_current_read_byte(in_queue) == '\n') || (packet_body_size == 1))
	{
		/* bad command packet as either \n or only 1 byte of command */
		increment_read_next_index(in_queue, 1);
	}
	/* was the packet body empty */
	else if(packet_body_size == 0)
	{
		/* there was no packet body size i.e. empty packet body */

		/* return FALSE below to handle no command */
	}
	else
	{	
		/* search the queue for a space from the current read point as the command format */
		/* is COMMANDNAME OPTIONALPARAM1 ETC\n so we will look from the current read point to */
		/* the space to get the COMMANDNAME but only for a MAXIMUM size of the COMMANDNAME */

		while((index < MAX_COMMAND_NAME_LENGTH) && (command_name_complete == FALSE)  && (index <= packet_body_size))
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
			else /* copy the byte to the command name string */
			{
				command_string[index] = get_current_read_byte(in_queue);
				increment_read_next_index(in_queue, 1);
				index++;    
			}

		} /* while */

		/* if the command name is too long i.e. we didnt get to the end of it */
		if((index == MAX_COMMAND_NAME_LENGTH) && (command_name_complete == FALSE))
		{
			/* command name is too long - skip to the end of the command name */			
			increment_read_next_index(in_queue, packet_body_size - index);
			/* return below as command_name_complete == FALSE */
		}
	}

	return( command_name_complete );
}

/* this function reads the queue for a parameter (space seperated) */	
BOOL get_command_next_param_text(Queue *in_queue, char *param_found_text, int max_size_of_param, BOOL nl_terminated)
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

/* this function reads the queue for all parameters in the command e.g. for shell usage */	
BOOL get_all_command_params_text(Queue *in_queue, char *param_found_text, int max_size_of_param)
{
	int index = 0;
	BOOL found_param = TRUE;
	
	/* read from in_queue all parameters up to the \n */
	
	/* clear the space to copy param into */
	memset(param_found_text, 0, max_size_of_param);
	
	/* read from current in_queue read pointer to \n found */
	while( (get_current_read_byte(in_queue) != '\n') && 
		   (index < max_size_of_param) )	
	{
		param_found_text[index] = get_current_read_byte(in_queue);
		increment_read_next_index(in_queue, 1);
		index++;
	}

	/* check we found a space or \n terminated parameter that we could copy into the space provided */
	if((index == max_size_of_param) && (get_current_read_byte(in_queue) != '\n'))
	{
		/* the parameter was too large and not \n terminated */
    	memset(param_found_text, 0, max_size_of_param);
		found_param = FALSE;
	}
	/* was the parameter \n terminated as expected */
	else if(get_current_read_byte(in_queue) == '\n')
	{
		/* yes was \n terminated so skip the \n */
		increment_read_next_index(in_queue, 1);
	}
	
	return(found_param);
}

/************************************************************/
/*                                                          */
/* Functions for working with Binary Commands               */
/*                                                          */
/************************************************************/

/* get the commandID binary from the packet body */
Command get_command_bin(Queue *in_queue)
{
	Command command = Unknown;

	/* +6 to skip +++ two_byte_size one_byte_packet_type */
	increment_read_next_index(in_queue, FRONT_DELIMITER_OVERHEAD + FRONT_PACKET_OVERHEAD);

	/* get the command byte */
	command = (Command) get_current_read_byte(in_queue);

	/* increment ready for the param (or delimiter if no params) */
	increment_read_next_index(in_queue, 1);

	return(command);
}

/* this function reads the queue for the next binary parameter */	
int get_command_next_8bit_param_bin(Queue *in_queue)
{
	int param = 0;

	param = get_current_read_byte(in_queue);
	increment_read_next_index(in_queue, 1);

	return( param );
}

/* this function reads the queue for the next binary parameter */	
int get_command_next_16bit_param_bin(Queue *in_queue)
{
	unsigned int param = 0;

	param = get_current_read_byte(in_queue);
	increment_read_next_index(in_queue, 1);
	param = (get_current_read_byte(in_queue) << 8) + param;
	increment_read_next_index(in_queue, 1);

	return( (int) param );
}

/* this function reads the queue for the next binary parameter */	
int get_command_next_32bit_param_bin(Queue *in_queue)
{
	unsigned int param = 0;

	param = get_current_read_byte(in_queue);
	increment_read_next_index(in_queue, 1);
	param = ((get_current_read_byte(in_queue) << 8) + param) << 16;
	increment_read_next_index(in_queue, 1);
	param = get_current_read_byte(in_queue) + param;
	increment_read_next_index(in_queue, 1);
	param = (get_current_read_byte(in_queue) << 8) + param;
	increment_read_next_index(in_queue, 1);

	return( (int) param );
}

/************************************************************/
/*                                                          */
/* Functions for working with Help Commands                 */
/*                                                          */
/************************************************************/

/* add up the length of the array of strings */
int size_of_string_array(char **text_array)
{
	int size_of_array = 0;
	int index = 0;

	while(text_array[index] != NULL)
	{
		size_of_array = size_of_array + strlen(text_array[index]);
		index++;
	}

	return( size_of_array );
}

/* write out an array of text for help commands */
BOOL write_out_help_text_array(Queue *queue, char **help_text_array)
{
	BOOL function_complete = FALSE;
	
	int response_size = size_of_string_array(help_text_array);

	/* write response to out_queue */
	if(start_build_data_packet(queue, response_size, TextCommand))
	{	
		int index = 0;

		while(help_text_array[index] != NULL)
		{
			/* add all the strings to the message */
			add_data_to_packet(queue, (BYTE *) help_text_array[index], strlen(help_text_array[index]));
			index++;
		}

		/* finish and write out the packet */
		finish_build_data_packet_and_send(queue);
		
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

