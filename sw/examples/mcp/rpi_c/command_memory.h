/********************************/
/*                              */
/* Memory API Command Functions */
/*                              */
/********************************/

#ifndef COMMAND_MEMORY_H
#define COMMAND_MEMORY_H 1

#include "mcp_pi.h"

/********************************************************************/
/*                                                                  */
/* Functions providing the command services                         */
/*                                                                  */
/********************************************************************/

/* these binary commands control moving memory in/out of the CPC to/from the pi */
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

/* help for binary commands that control moving pages of memory in/out of the CPC to/from the pi */
BOOL command_create_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_free_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_retrieve_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_store_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_insert_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_cut_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_load_alloc_help(Queue *in_queue, Queue *out_queue);
BOOL command_save_alloc_help(Queue *in_queue, Queue *out_queue);

#endif
