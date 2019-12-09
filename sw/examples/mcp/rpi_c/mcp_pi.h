/********************************************************************/
/* Can only be built on a raspberry pi due to the wiringPi.h        */
/* being a raspberry pi only header and library                     */
/********************************************************************/

#ifndef MCP_PI_H
#define MCP_PI_H 1

#define VERSION "0.2"

#include "queue.h"

/*                                                     */
/* All OF THESE SHOULD BE COMMENTED OUT ON PRODUCTION  */
/*                                                     */

/* uncomment this to get more MCP command line output for dev testing */
#define DEBUG_MCP 1
/* uncomment this to get even more MCP command line output for dev testing */
/* #define DEBUG_MCP_2 1 */
/* uncomment to test inject messages into the incoming queue to test processing in dev */
/* #define DEBUG_MCP_IF 1 */

/*                                                  */
/* only change these if you know what you are doing */
/*                                                  */

/* maximum length of a command name e.g. PING is 4 */
#define MAX_COMMAND_NAME_LENGTH 30
/* maximum number of concurrent processes */
#define MAX_NUM_PROCESSES 20
/* maximum file path size for reading files */
#define MAX_PATH_SIZE_TEXT 512
/* maximum file size (text of the filesize) */
#define MAX_FILE_SIZE_TEXT 20
/* define how many potential in and out interface checks are done before each tick is checked */
#define MAX_TICK_WAIT 100
/* define how many potential in and out interface checks are done before we check for a command */
#define MAX_COMMAND_WAIT 20

/* the over head in bytes of of the packet headers and delimiteres */
#define PACKET_OVERHEAD 9
/* the over head in bytes from the start of the packet to the pay load */
#define FRONT_PACKET_OVERHEAD 3
/* the size of the front delimiter */
#define FRONT_DELIMITER_OVERHEAD 3
/* the size of the end delimiter */
#define REAR_DELIMITER_OVERHEAD 3
/* terminator overhead for packet */
#define PACKET_DELIMITER_OVERHEAD 6

/* define packet types that are valid */
typedef enum PacketType 
{
    UnknownPacket     = 0,
	TextCommand       = 1,
	BinaryCommand     = 2,
	BinaryData        = 3,
    GraphicDirectives = 4,
    
    Max_PacketType = 5
    
} PacketType;

extern Queue in_queue;
extern Queue out_queue;
extern BOOL TRON;

#ifdef DEBUG_MCP_IF
    /* inject message into in_queue for dev testing of commands without interface */
    void inject_test_messages(void);
    void inject_command(Queue *in_queue, const char *message, PacketType packet_type);
    void inject_binary_data(Queue *in_queue, const char *message, PacketType packet_type, int page_size);
#endif

#endif
