/************************************************************************/
/*                                                                      */
/* This can only be compiled on a raspberry pi due to the wiringPi.h    */
/* being a raspberry pi only header and library.                        */
/*                                                                      */
/* To compile enter "make" at the command line                          */
/*                                                                      */
/* If you want to test compile on a PC, uncomment #define BUILD_ON_PC 1 */
/* in mcp_pi.h and it will include dummy headers for wiringPi.  This is */
/* only a test compilation IT WILL NOT EXECUTE on a raspberry pi        */
/*                                                                      */
/* To compile enter "make mcp_pc" at the command line                   */
/*                                                                      */
/************************************************************************/
#include <string.h>
#include <stdio.h>
#include <unistd.h>

#include "mcp_pi.h"
#include "command.h"
#include "command_processor.h"

BYTE DATA[] = { PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7 };

/*****************/
/*               */
/* define queues */
/*               */
/*****************/

Queue in_queue;
Queue out_queue;

/***********************************/
/*                                 */
/* Interface functions             */
/*                                 */
/***********************************/

/* setup pins on the GPIO for the interface */
void setup_pins() 
{
  /* init wiring pi library */
  wiringPiSetupGpio();

  /* set up the pins for input mod */
  for(int i=0; i<8; i++)
  {
    pinMode(DATA[i],INPUT);
  }

  pinMode(PIN_DIR, INPUT);
  pinMode(PIN_DOR, INPUT);
  pinMode(PIN_SI, OUTPUT);
  digitalWrite(PIN_SI,LOW);
  pinMode(PIN_SOB, OUTPUT);
  digitalWrite(PIN_SOB,LOW);
  pinMode(PIN_WNR, OUTPUT);
  digitalWrite(PIN_WNR,LOW);
}

/* write a byte to the interface */
void write_fifo_byte(BYTE txdata) 
{
  int bit;

  digitalWrite(PIN_WNR,HIGH);

  for(int i=0; i<8; i++) 
  {
    bit = (txdata & 0x1)? HIGH: LOW;
    pinMode(DATA[i], OUTPUT);
    digitalWrite(DATA[i],bit);
    txdata = txdata >> 1;
  }
  
  digitalWrite(PIN_SI, HIGH);  
  digitalWrite(PIN_SI, LOW);

  for(int i=0; i<8; i++) 
  {
    pinMode(DATA[i], INPUT);
  }

  digitalWrite(PIN_WNR, LOW);
}

/* read a byte from the interface */
int read_fifo_byte() 
{
  int rval = 0;

  for(int i=7; i>=0; i--) 
  {
    rval = (rval << 1) + (digitalRead(DATA[i]) & 0x1);
  }

  digitalWrite(PIN_SOB, HIGH);
  digitalWrite(PIN_SOB, LOW);

  return(rval);
}

/**************************************************************/
/*                                                            */
/* Functions to work with the queue                           */
/*                                                            */
/**************************************************************/

/* init_queue */
void init_queue(Queue *queue)
{
  /* this will clear the buffers and set mode to 'command' */
  memset(queue, 0, sizeof(Queue));
}

/* is a complete command in in_queue */
BOOL is_command_available(Queue *in_queue)
{
  int index = 0;
  int read_index = 0;
  
  BOOL command_found = FALSE;
  
  /* search the queue for a \n terminated string from the current read point */
  /* but not past the store point (and remember to wrap around in circular queue*/
  
  read_index = (in_queue->read_next_byte + index) % QUEUE_SIZE;
  
  while((read_index != in_queue->store_next_byte ) && (command_found == FALSE))
  {
    if((in_queue->data[read_index] == '\n') && (index > 0))
    {
      /* found a \n at the end of a command */
      command_found = TRUE;
    }

    index++;
    
    read_index = (in_queue->read_next_byte + index) % QUEUE_SIZE;

  } /* while */

  return( command_found );
}

/* get the text string of the command name */
void get_command_string(Queue *in_queue, BYTE *command_string)
{
  int index = 0;
  int read_index = (in_queue->read_next_byte + index) % QUEUE_SIZE;
  BOOL command_name_complete = FALSE;
  
  /* clear the command string */
  memset(command_string, 0, COMMAND_NAME_LENGTH);
  
  /* search the queue for a space from the current read point as the command format */
  /* is COMMANDNAME OPTIONALPARAM1 ETC\n so we will look from the current read point to */
  /* the space to get the COMMANDNAME but only for a MAXIMUM size of the COMMANDNAME */
  
  while((index < COMMAND_NAME_LENGTH) && (command_name_complete == FALSE))
  {
    /* did we find a space to terminate the command name text */
    /* or a \n indicating end of command i.e. no params       */
    if(((in_queue->data[read_index] == ' ') || (in_queue->data[read_index] == '\n')) && (index > 0))
    {
      /* found a space (or \n) at the end of a command name */
      command_name_complete = TRUE;
      /* increment the read_next_byte pointer as we have read the command name from the queue*/
      in_queue->read_next_byte = (read_index + 1) % QUEUE_SIZE;
    }
    else /* copy the byte to the command name string */
    {
      command_string[index] = in_queue->data[read_index];
    }

    index++;
    
    read_index = (in_queue->read_next_byte + index) % QUEUE_SIZE;

  } /* while */  
}

BOOL get_data_from_queue(Queue *queue, BYTE *data)
{
  return( FALSE );
}

BOOL write_data_to_queue(Queue *queue, BYTE *data, int size_of_data)
{
  #ifdef DEBUG_MCP
    printf("Write Start OUT Queue %d: read: %d write %d\n", QUEUE_SIZE, queue->read_next_byte, queue->store_next_byte);
  #endif
  
  /* if there is space between current store point and top of queue */
  if(QUEUE_SIZE - queue->store_next_byte >= size_of_data )
  {
    /* just store the data */
    memcpy(&queue->data[queue->store_next_byte], data, size_of_data);
    /* increment the store pointer as we have stored data */
    queue->store_next_byte = queue->store_next_byte + size_of_data;
  }
  else /* not enough space between where we are and top of queue or queue is nearly full */
  {
    /* enough of space to store data but split across top of queue and wrap around to bottom of queue */
    if((QUEUE_SIZE - queue->store_next_byte) + (queue->read_next_byte - 1) >= size_of_data )
    {
      /* copy data in two sections */
      /* from store_next_byte to top of queue */
      memcpy(&queue->data[queue->store_next_byte], data, QUEUE_SIZE - queue->store_next_byte);
      /* from bottom of queue store the remaining */
      memcpy(&queue->data[0], data + (QUEUE_SIZE - queue->store_next_byte), size_of_data - (QUEUE_SIZE - queue->store_next_byte));
      /* set where to store next bytes */
      queue->store_next_byte = size_of_data - (QUEUE_SIZE - queue->store_next_byte) + 1;
    }
    else /* not enough space to store data - queue is full so return false to command function - it will try again */
    {
      return( FALSE );
    }
  }
  
  #ifdef DEBUG_MCP
    printf("Write End OUT Queue %d: read: %d write %d\n", QUEUE_SIZE, queue->read_next_byte, queue->store_next_byte);
    printf("OutQueue: '%s'\n", (char *)&queue->data[queue->read_next_byte]);
  #endif
  

  return( TRUE );
}

/* reset the in_queue and out_queue and clear all data   */
void reset_queues(void)
{
  init_queue(&in_queue);
  init_queue(&out_queue);  
}

/**************************************************************/
/*                                                            */
/* The main loop which:                                       */
/*                                                            */
/*   Reads bytes from the interface                           */
/*   Checks if complete command/data to process               */
/*     Process Command                                        */
/*   Checks if any data response queued to write to interface */
/*     Write data to interface                                */
/*                                                            */
/**************************************************************/

void main( void ) 
{
  Command commandID = Unknown;
  BYTE command_string[COMMAND_NAME_LENGTH];
  
  printf("MCP v%s\n", VERSION);
  
  /* setup interface pins */
  setup_pins();
  
  /* init the queues */
  reset_queues();
  
  printf("Init ...");
  
  /* init the command processor */
  command_processor_init();
  
  printf("Complete\n");
  printf("Starting Command Processing Loop\n");

  #ifdef DEBUG_MCP_IF
    /* pretend we received messages by inserting into queue and processing them */
    inject_test_messages();
  #endif
  
  /* loop for ever */
  for(;;) 
  {
    /* read data from the interface into the inqueue if data available */
    if(digitalRead(PIN_DOR)) 
    {
      /* read a byte from the interface and store in incoming queue */
      in_queue.data[in_queue.store_next_byte] = read_fifo_byte();
      
      #ifdef DEBUG_MCP
        printf("Read %c %d\n",in_queue.data[in_queue.store_next_byte], in_queue.data[in_queue.store_next_byte]);
      #endif
      
      /* use the queue in a loop so ensure the in_queue.store_next_byte wraps around */
      in_queue.store_next_byte = (in_queue.store_next_byte+1) % QUEUE_SIZE;
    }

    #ifdef DEBUG_MCP_IF
      /* process injected commands during dev */
    
      process_any_injected_commands();
    
    #else 
      /* process commands in production */
    
      /* is a complete command in in_queue */
      if( is_command_available( &in_queue ) == TRUE )
      {
        /* get the text string of the command name*/
        get_command_string(&in_queue, command_string);
        /* get the command id from the command_string */                   
        commandID = get_commandID_from_string( command_string );
        /* begin execution of its first segment  */        
        process_command(commandID, &in_queue, &out_queue);
      }
    #endif
    
    /* if any data waiting to be written to the interface */
    if((out_queue.read_next_byte != out_queue.store_next_byte) && digitalRead(PIN_DIR)) 
    {
      /* write the byte out */
      write_fifo_byte(out_queue.data[out_queue.read_next_byte]);
      /* move write pointer on, looping around queue when it gets to end */
      out_queue.read_next_byte = (out_queue.read_next_byte+1) % QUEUE_SIZE;
    }
    
    /* check if any commands need a tick to do their next bit */
    tick_command();
    
  } /* for */
    
} /* main */

#ifdef DEBUG_MCP_IF
void inject_test_messages(void)
{
  const char *test_message = "PING\nSHUTDOWN\nTIME\nDATE\nCPAGE 5 8\nRESET\n";
  
  /* push in message to test command processor */
  memcpy(in_queue.data, test_message, strlen(test_message));
  in_queue.store_next_byte = strlen(test_message);
}

void process_any_injected_commands(void)
{
  Command commandID = Unknown;
  BYTE command_string[COMMAND_NAME_LENGTH];
  
  /* sleep(1); */
  
  if( is_command_available( &in_queue ) == TRUE )
  {
    printf("Command Received\n");
    /* get the text string of the command name*/
    get_command_string(&in_queue, command_string);
    printf("Got command string '%s'\n", command_string);
    /* get the command id from the command_string */                   
    commandID = get_commandID_from_string( command_string );
    printf("Got command ID %d\n",commandID);
    /* begin execution of its first segment  */        
    process_command(commandID, &in_queue, &out_queue);
  }
}
#endif