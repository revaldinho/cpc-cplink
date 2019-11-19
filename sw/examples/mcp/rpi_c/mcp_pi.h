/********************************************************************/
/* Can only be built on a raspberry pi due to the wiringPi.h        */
/* being a raspberry pi only header and library                     */
/********************************************************************/

#ifndef MCP_PI_H
#define MCP_PI_H 1

#define VERSION "0.1"

/*                                                     */
/* SHOULD BE COMMENTED OUT WHEN RUNNING ON PI */
/*                                                     */
/* only uncomment this for test compiling on PC */
/* MUST be commented out for compiling in pi    */
/*                                                     */
/*#define BUILD_ON_PC 1*/

/*                                                     */
/* BOTH OF THESE SHOULD BE COMMENTED OUT ON PRODUCTION */
/*                                                     */
/* uncomment this to get more MCP command line output for dev testing */
#define DEBUG_MCP 1
/* uncomment to test inject messages into the incoming queue to test processing in dev */
/*#define DEBUG_MCP_IF 1*/

#ifdef BUILD_ON_PC
    #include "dummy_wiringpi.h"
    #define FALSE 0
    #define TRUE 1
#else
	#include <wiringPi.h>
#endif

/************************/
/*                      */
/* Uncomment for pi 1's */
/*                      */
/************************/
/* #define PI1 1 */

/* pin definitions for the interface */
#ifndef PI1 
    // BCM Pin numbering for all other PI's
    #define PIN_D7   11
    #define PIN_D6   10
    #define PIN_D5    9
    #define PIN_D4    8
    #define PIN_D3    7
    #define PIN_D2    4
    #define PIN_D1    3
    #define PIN_D0    2
    #define PIN_SI   18
    #define PIN_DIR  17
    #define PIN_SOB  22
    #define PIN_DOR  23
    #define PIN_WNR  24
#else
    // Wiring PI pin numbering for older PI1
    #define PIN_D7   11
    #define PIN_D6   10
    #define PIN_D5    9
    #define PIN_D4    8
    #define PIN_D3    7
    #define PIN_D2    4
    #define PIN_D1    1
    #define PIN_D0    0
    #define PIN_SI   18
    #define PIN_DIR  17
    #define PIN_SOB  22
    #define PIN_DOR  23
    #define PIN_WNR  24
#endif

/*                                                  */
/* only change these if you know what you are doing */
/*                                                  */

/* size of queues for incoming and outdoing */
#define QUEUE_SIZE  8192
/* maximum length of a command name e.g. PING is 4 */
#define COMMAND_NAME_LENGTH 30
/* maximum number of concurrent processes */
#define NUM_PROCESSES 20

/* typedefs which the compiler does not seem to have for some weird reason */
typedef unsigned char BYTE;
typedef unsigned char BOOL;

typedef enum QueueMode 
{
	command = 0,
	binary = 1,
    
} QMode;

typedef struct Queue 
{
	BYTE data[QUEUE_SIZE];
	
	int store_next_byte;
	int read_next_byte;
	
	QMode mode;
		
} Queue;

/* this function will read available data from the queue */
/* return value is TRUE if data from queue into *data    */
BOOL get_data_from_queue(Queue *queue, BYTE *data);
/* this function will try and write data to the queue    */
/* return value is TRUE if data is written to the queue  */
BOOL write_data_to_queue(Queue *queue, BYTE *data, int size_of_data);
/* reset the in_queue and out_queue and clear all data   */
void reset_queues(void);

#ifdef DEBUG_MCP_IF
    /* inject message into in_queue for dev testing of commands without interface */
    void inject_test_messages(void);
    /* for processing injected queue messages */
    void process_any_injected_commands(void);
#endif

#endif