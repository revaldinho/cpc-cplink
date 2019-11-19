/*********************************************************/
/*                                                       */
/* The definition of a command(service) that the pi      */
/* provides to the CPC                                   */
/*                                                       */
/*********************************************************/


#ifndef COMMAND_H
#define COMMAND_H 1

#include "mcp_pi.h"

/* there should be an enum here for each command in the system */
/* MaxCommands should ALWAYS be 1 more than the last command   */

typedef enum CommandIDs 
{
    Unknown  = 0,
	Ping     = 1,
	Shutdown = 2,
	Time     = 3,
	Date     = 4,
	Reset    = 5,
	
	CreatePage = 6,
	FetchPage  = 7,
	StorePage  = 8,
    LoadPages  = 9,
    SavePages  = 10,
	
	MaxCommands = 11
	
} Command;

/* define a structure for holding command data in */

typedef struct CommandDef
{
	Command commandID;
	BYTE *command_text;
	BOOL (*return_function)(Queue *, Queue*, int);
	
} CommandDef;

extern const CommandDef command_defs[MaxCommands];

/* get the command id from the command_string */                   
Command get_commandID_from_string(BYTE *command_string);

/* this function will return a pointer to a command to run for the command id passed */
void *get_command_function(Command commandId );

/* these are the commands supported by MCP                    */
/* the return value is TRUE if the command has completed      */
/* false if it has not completed                              */
/* the function signature must stay the same for all commands */

BOOL command_unknown(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_ping(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_shutdown(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_time(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_date(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_reset(Queue *in_queue, Queue *out_queue, int tick);

/* these commands control moving pages of memory in/out of the CPC to/from the pi */
BOOL command_create_page(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_fetch_page(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_store_page(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_load_pages(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_save_pages(Queue *in_queue, Queue *out_queue, int tick);

#endif