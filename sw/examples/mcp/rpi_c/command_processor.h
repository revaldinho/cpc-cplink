/*********************************************************/
/*                                                       */
/* The definition of a command processor that the pi     */
/* uses to process the service requests from the CPC     */
/*                                                       */
/*********************************************************/

#ifndef COMMAND_PROCESSOR_H
#define COMMAND_PROCESSOR_H 1

#include "mcp_pi.h"
#include "command.h"

#define MAX_COMMAND_STATES 20

typedef struct Command_State 
{
	/* the id of the command */
	Command command_id;
	/* the function to execute */
	BOOL (*command_function)(Queue *, Queue*, int);
	
	/* how many times it ticked */
	int next_tick_count;
	/* when to tick again (microseconds) */
	long next_exec_time;
	
} Command_State;

/* initialise the command processor data structures */
void command_processor_init(void);

/* process the specified text command */
BOOL process_text_command(Command command_id, Queue *in_queue, Queue *out_queue);

/* check to see if any command has another step to perform */
void tick_command( Queue *in_queue, Queue *out_queue );

/* find an empty command state to use */
Command_State *get_empty_command_state(void);

	/* get the next exec time in microseconds for that command */
long get_next_exec_time(Command command_id);
	
/* get the current time in microseconds */
long get_current_exec_time(void);

	
#endif