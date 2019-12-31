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

/*************************************************/
/* DO NOT CHANGE THE NUMBERING OR ORDERING BELOW */
/* YOU MAY ADD ABOVE MaxCommands ONLY            */
/*************************************************/

typedef enum CommandIDs 
{
    Unknown  = 0,
    
    /* General TextCommand Functions */
	Ping      = 1,
	Shutdown  = 2,
	Time      = 3,
	Date      = 4,
	Reset     = 5,
    Help      = 6,
    Reboot    = 7,
    Tron      = 8,
    Troff     = 9,
    Version   = 10,
	WifiIP    = 11,
    ShellExec = 12,
    Spare2    = 13,
    Spare3    = 14,
    Spare4    = 15,
    Spare5    = 16,
    Spare6    = 17,
    Spare7    = 18,
    Spare8    = 19,
    Spare9    = 20,

    /* Memory BinaryCommand Functions */
	CreateAlloc   = 21,
    FreeAlloc     = 22,
	RetrieveAlloc = 23,
	StoreAlloc    = 24,
    InsertAlloc   = 25,
    CutAlloc      = 26,
    LoadAlloc     = 27,
    SaveAlloc     = 28,

    /* File TextCommand Functions */
    GetFile          = 29,
    PutFile          = 30,
    
    /* File BinaryCommand Functions */
    GetMemFile       = 31,
    PutMemFile       = 32,

    /* MUST always be last */
	MaxCommands = 33
	
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

/* this function will return a pointer to a command to run for the command id passed */
void *get_command_function(Command command_id);

/* get the tick_time for the command */
long get_command_tick_period(Command command_id);

/* get the size of the out_queue typically used by the command */
int get_command_out_queue_space_required(Command command_id);

/* get the command string name from the command id */
const char *get_command_name_from_id(Command command_id);

/************************************************************/
/*                                                          */
/* Functions for working with Text Commands                 */
/*                                                          */
/************************************************************/

/* get the command id from the command_string */                   
Command get_commandID_from_string(char *command_string);

/* get the text string of the command name */
BOOL get_command_text(Queue *in_queue, char *command_string);

/* this function reads the queue for a parameter (space seperated) */	
BOOL get_command_next_param_text(Queue *in_queue, char *param_found_text, int max_size_of_param, BOOL nl_terminated);

/* this function reads the queue for all parameters in the command e.g. for shell usage */	
BOOL get_all_command_params_text(Queue *in_queue, char *param_found_text, int max_size_of_param);

/************************************************************/
/*                                                          */
/* Functions for working with Binary Commands               */
/*                                                          */
/************************************************************/

/* get the commandID binary from the packet body */
Command get_command_bin(Queue *in_queue);

/* this function reads the queue for the next binary parameter */	
int get_command_next_8bit_param_bin(Queue *in_queue);

/* this function reads the queue for the next binary parameter */	
int get_command_next_16bit_param_bin(Queue *in_queue);

/* this function reads the queue for the next binary parameter */	
int get_command_next_32bit_param_bin(Queue *in_queue);

/************************************************************/
/*                                                          */
/* Functions for working with Help Commands                 */
/*                                                          */
/************************************************************/

/* add up the length of the array of strings */
int size_of_string_array(char **text_array);

/* write out an array of text for help commands */
BOOL write_out_help_text_array(Queue *queue, char **help_text_array);

#endif