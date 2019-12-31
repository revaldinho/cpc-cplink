/********************************/
/*                              */
/* File Command Functions       */
/*                              */
/********************************/

#ifndef COMMAND_FILE_H
#define COMMAND_FILE_H 1

#include "mcp_pi.h"

/********************************************************************/
/*                                                                  */
/* Functions providing the command services                         */
/*                                                                  */
/********************************************************************/

/* get a file from the pi's storage to write to cpc storage */
BOOL command_get_file(Queue *in_queue, Queue *out_queue, int tick);
/* put a file on cpc storage on to the pi's storage */
BOOL command_put_file(Queue *in_queue, Queue *out_queue, int tick);
/* get a file from the pi storage into cpc memory */
BOOL command_get_mem_file(Queue *in_queue, Queue *out_queue, int tick);
/* put a file in cpc memory into pi storage */
BOOL command_put_mem_file(Queue *in_queue, Queue *out_queue, int tick);

/********************************************************************/
/*                                                                  */
/* Help Functions for the command services                          */
/*                                                                  */
/********************************************************************/

BOOL command_get_file_help(Queue *in_queue, Queue *out_queue);
BOOL command_put_file_help(Queue *in_queue, Queue *out_queue);
BOOL command_get_mem_file_help(Queue *in_queue, Queue *out_queue);
BOOL command_put_mem_file_help(Queue *in_queue, Queue *out_queue);

#endif
