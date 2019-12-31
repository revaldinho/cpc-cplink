/********************************/
/*                              */
/* General Command Functions     */
/*                              */
/********************************/

#ifndef COMMAND_GENERAL_H
#define COMMAND_GENERAL_H 1

#include "mcp_pi.h"

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
BOOL command_version(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_wifi_ip(Queue *in_queue, Queue *out_queue, int tick);
BOOL command_shellexec(Queue *in_queue, Queue *out_queue, int tick);

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
BOOL command_version_help(Queue *in_queue, Queue *out_queue);
BOOL command_wifi_ip_help(Queue *in_queue, Queue *out_queue);
BOOL command_shellexec_help(Queue *in_queue, Queue *out_queue);

#endif
