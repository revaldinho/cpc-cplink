/*********************************************************/
/*                                                       */
/* The definition of a command processor that the pi     */
/* uses to process the service requests from the CPC     */
/*                                                       */
/*********************************************************/

#include <string.h>
#include <stdio.h>
#include <sys/time.h>

#include "command_processor.h"
#include "queue.h"

Command_State command_states[MAX_COMMAND_STATES];

/* initialise the command processor data structures */
void command_processor_init(void)
{
	memset( &command_states, 0, sizeof(Command_State) * MAX_COMMAND_STATES);
}

/* process the specified text command */
BOOL process_text_command(Command command_id, Queue *in_queue, Queue *out_queue)
{
	BOOL (*return_function)(Queue *, Queue *, int) = get_command_function(command_id);	
	int output_space_required = get_command_out_queue_space_required(command_id);
	
	/* is there a function for that command id to run */
	if(return_function == NULL)
	{
		if(TRON)
		{
			printf("ERROR Unable to find function for Command %d\n", command_id);
		}
	}
	/* is there space in the out_queue for the messages the command will generate */
	else if(is_queue_space_available(out_queue, output_space_required) == FALSE)
	{
		/* no there is insufficient space in the out_queue to store output from the command  */
		/* no point in running the command (as the output will be lost or truncated)         */
		/* return FALSE so that the command text in the in_queue will be 'unread' and we     */
		/* will try and reread the in_queue and process the command again once the out_queue */
		/* has the space to store the output from the command trying to be processed         */
		
		return(FALSE);		
	}
	else /* we have a function to execute and space in out_queue to store the response */
	{
		/* execute the function */
		BOOL function_complete_state = return_function(in_queue, out_queue, 0);
		
		if(TRON)
		{
			printf("Command Function Returned: %d\n\n", function_complete_state);
		}
		
		/* did the function complete (it may have completed with errors messages to the client but it is complete) */
		/* a function that is not complete is one that can't complete due to some condition e.g. out_queue full    */
		/* or one that runs in multipe sections and we have only run section 0, see shutdown command for example   */
		if(function_complete_state)
		{
			/* function completed so we need do nothing else with tracking this command as it */
			/* has no more stages to perform and the command is complete.                     */
		}
		else /* function is not complete */
		{
			/* if the command was to retrieve a page and it failed to complete */
			if(command_id == RetrieveAlloc)
			{
				/* we failed to complete due to out_queue being full and as the retrieve is based on the page_size */
				/* we could not check for it above as we do for other commands hence why we have to handle it here */
				/* we will return FALSE which will cause the command to be 'unread' in the in_queue so that it can */
				/* be tried again later maybe when the out_queue is not so full                                    */
				
				return(FALSE);
			}
			/* if the command was to store a page and it failed to complete */
			else if(command_id == StoreAlloc)
			{
				/* we failed to complete the StorePage function for one reason - we have not received a whole page */
				/* worth of data from the client into the in_queue.  So we will return FALSE which will cause the  */
				/* command to be 'unread' in the in_queue so that it can be tried again later when all the binary  */
				/* data we are expecting from the client has arrived in the in_queue.                              */
				
				return(FALSE);
			}
			/* if the command was to insert a page and it failed to complete */
			else if(command_id == InsertAlloc)
			{
				/* we failed to complete the InsertAlloc function for one reason - we have not received the data   */
				/* containing the BinaryData to insert from the client. So we will return FALSE                    */
				/* which will cause the command to be 'unread' in the in_queue so that it can be tried again later */
				/* when all the binary data we are expecting from the client has arrived in the in_queue.          */
				
				return(FALSE);
			}
			else
			{
				/* a function may not complete due to being designed to execute in stages (and this is step 0) */
				/* we need to store the state of the current function so it can be run again later as a TICK   */
				/* which occurs in the main loop                                                               */

				Command_State *command_state = get_empty_command_state();

				/* if we got command_state to store in */
				if(command_state)
				{
					/* create a Command_State so that the command can be executed again */
					command_state->command_id       = command_id;
					command_state->command_function = return_function;
					command_state->next_tick_count  = 1;
					command_state->next_exec_time   = get_next_exec_time(command_id);					
				}
				else
				{
					/* if we get there there is nothing we can do, can't complete command and can't store it for ticking */
					printf("CRITICAL ERROR command_states full - unable to store command %d for later execution\n", command_id);
				}
			}
		}
	}
	
	return(TRUE);
}

/* check to see if any command has another step to perform */
void tick_command( Queue *in_queue, Queue *out_queue )
{
	/* this function will only execute a single command tick   */
	/* regardless of how many are waiting to run               */

	long current_exec_time = get_current_exec_time();
	Command_State *current_command_state = NULL;
	
	/* check all command_states - do any command need ticking */
	for(int index = 0; index < MAX_COMMAND_STATES; index++)
	{
		current_command_state = &command_states[index];
		
		/* if we have a command_state thats valid */
		if(current_command_state->command_id != Unknown)
		{
			/* we have a command - now check if it needs ticking i.e. next_exec_time is in the past */
			if(current_command_state->next_exec_time < current_exec_time)
			{
				/* yes it needs ticking so run the function with the next_tick_count */
				BOOL command_complete = current_command_state->command_function(in_queue, out_queue, current_command_state->next_tick_count);

				#ifdef DEBUG_MCP_IF
					printf("Command Function Returned: %d\n", command_complete);
				#endif		

				/* if the command is now complete - then free the command_state */
				if(command_complete)
				{
					/* free the command state */
					memset(current_command_state, 0, sizeof(Command_State));
				}
				else /* its not complete so will need to tick again in the future */
				{
					current_command_state->next_tick_count++;	
					current_command_state->next_exec_time = get_next_exec_time(current_command_state->command_id);
				}
				
				/* break out of for loop as we dont want to execute another command tick */
				break;
			}
		}
	}
}

/* find an empty command state to use */
Command_State *get_empty_command_state(void)
{
	Command_State *empty_state = NULL;
	
	for(int index = 0; index < MAX_COMMAND_STATES; index++ )
	{
		if(command_states[index].command_id == Unknown)
		{
			/* found an empty state to use */
			empty_state = &command_states[index];
		}
	}
	
	return( empty_state );
}
		
/* get the next exec time in microseconds for that command */
long get_next_exec_time(Command commandId)
{
	/* get the command tick time (millseconds) and convert to microseconds */
	long command_tick_time = get_command_tick_period(commandId) * 1000;
	
	return( get_current_exec_time() + command_tick_time );
}
		
/* get the current time in microseconds */
long get_current_exec_time(void)
{
	struct timeval current;
	
	gettimeofday(&current, NULL);
	
	return(current.tv_usec + (current.tv_sec * 1000000));
}