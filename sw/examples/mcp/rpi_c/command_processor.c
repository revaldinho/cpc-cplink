/*********************************************************/
/*                                                       */
/* The definition of a command processor that the pi     */
/* uses to process the service requests from the CPC     */
/*                                                       */
/*********************************************************/

#include <string.h>
#include <stdio.h>

#include "command_processor.h"

CommandProcessorState command_processor;

void command_processor_init(void)
{
	memset( &command_processor, 0, sizeof(CommandProcessorState));
}

void process_command(Command command, Queue *in_queue, Queue *out_queue)
{
	BOOL (*return_function)(Queue *, Queue *, int) = get_command_function( command );
	
	#ifdef DEBUG_MCP
		printf("Func: %p\n", return_function);
	#endif
	
	#ifdef DEBUG_MCP_IF
		printf("Command Function Returned: %d\n",return_function(in_queue, out_queue, 0));
	#else
	    return_function(in_queue, out_queue, 0);
	#endif
	
	/* store the command function in the state */
}

void tick_command( void )
{
	/* get the next function that needs ticking */
}

