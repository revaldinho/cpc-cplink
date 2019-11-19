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

typedef struct CommandState 
{
	int test;
	
} CommandState;

typedef struct CommandProcessorState
{
	CommandState states[20];
	
} CommandProcessorState;

void command_processor_init(void);

void process_command(Command command, Queue *in_queue, Queue *out_queue);

void tick_command( void );
	
#endif