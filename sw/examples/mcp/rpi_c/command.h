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
    Help     = 6,
    Reboot   = 7,
    Tron     = 8,
    Troff    = 9,
	
	CreateAlloc    = 10,
    FreeAlloc      = 11,
	RetrieveAlloc  = 12,
	StoreAlloc     = 13,
    InsertAlloc    = 14,
    CutAlloc       = 15,
    LoadAlloc      = 16,
    SaveAlloc      = 17,
	
	MaxCommands = 18
	
} Command;

/* define a structure for holding command data in */

typedef struct CommandDef
{
	Command commandID;
	char *command_text;
	BOOL (*return_function)(Queue *, Queue*, int);
	BOOL (*help_function)(Queue *, Queue *);
    long tick_time;
    int out_queue_space;
	
} CommandDef;

extern const CommandDef command_defs[MaxCommands];

/* get the command id from the command_string */                   
Command get_commandID_from_string(char *command_string);

/* this function will return a pointer to a command to run for the command id passed */
void *get_command_function(Command command_id );

/* get the tick_time for the command */
long get_command_tick_period(Command command_id);

/* get the size of the out_queue typically used by the command */
int get_command_out_queue_space_required(Command command_id);

/* get the text string of the command name */
void get_command_string(Queue *in_queue, char *command_string);

/* these are the commands supported by MCP                    */
/* the return value is TRUE if the command has completed      */
/* FALSE if it has not completed                              */

/* the function signature must stay the same for all commands */

/********************************************************************/
/*                                                                  */
/* Functions providing the command services                         */
/*                                                                  */
/********************************************************************/

/* simple built in commands for basic services */
BOOL command_unknown(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_ping(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_shutdown(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_time(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_date(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_reset(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_help(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_reboot(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_tron(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_troff(Queue *in_queue, Queue *out_queue, int tick);

/* these commands control moving memory in/out of the CPC to/from the pi */
BOOL command_create_alloc(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_free_alloc(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_retrieve_alloc(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_store_alloc(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_insert_alloc(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_cut_alloc(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_load_alloc(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_save_alloc(Queue *in_queue, Queue *out_queue, int tick);

/********************************************************************/
/*                                                                  */
/* Help Functions for the command services                          */
/*                                                                  */
/********************************************************************/

/* simple built in commands help for basic services */
BOOL command_unknown_help(Queue *in_queue, Queue *out_queue);
BOOL command_ping_help(Queue *in_queue, Queue *out_queue);
BOOL command_shutdown_help(Queue *in_queue, Queue *out_queue);
BOOL command_time_help(Queue *in_queue, Queue *out_queue);
BOOL command_date_help(Queue *in_queue, Queue *out_queue);
BOOL command_reset_help(Queue *in_queue, Queue *out_queue);
BOOL command_help_help(Queue *in_queue, Queue *out_queue);
BOOL command_reboot_help(Queue *in_queue, Queue *out_queue);
BOOL command_tron_help(Queue *in_queue, Queue *out_queue);
BOOL command_troff_help(Queue *in_queue, Queue *out_queue);

/* help for commands that control moving pages of memory in/out of the CPC to/from the pi */
BOOL command_create_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_free_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_retrieve_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_store_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_insert_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_cut_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_load_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_save_alloc_help(Queue *in_queue, Queue *out_queue);

#endif