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
#include "fifolib.h"
#include "packet.h"
#include "command.h"
#include "command_processor.h"
#include "memstore_manager.h"

/*****************/
/*               */
/* define queues */
/*               */
/*****************/

Queue in_queue;
Queue out_queue;

BOOL TRON = FALSE;

/**************************************************************/
/*                                                            */
/* The main loop which:                                       */
/*                                                            */
/*   Reads bytes from the interface                           */
/*   Checks if complete command/data to process               */
/*     Process Command                                        */
/*   Checks if any data response queued to write to interface */
/*     Write data to interface                                */
/*   Tick any pending commands that are waiting to execute    */
/*                                                            */
/**************************************************************/

int main( void ) 
{
  Command commandID = Unknown;
  int tick_check_counter = 0;
  int packet_check_counter= 0;
  
  char command_string[MAX_COMMAND_NAME_LENGTH];
  
  printf("MCP v%s\n", VERSION);
  
  printf("Init ...");
  
  #ifndef WIRINGPI
    printf("\n\nTHIS IS THE PC BUILD\n\n");
  #endif
  
  /* setup interface pins */
  setup_pins();
  
  /* init the queues */
  queues_reset(&in_queue, &out_queue);
  
 	/* init the memstore */
	memstore_init();

  /* init the command processor */
  command_processor_init();
  
  printf("Complete\n");
  printf("Starting Command Processing Loop\n");

  #ifdef DEBUG_MCP
    TRON = TRUE;
  
    printf("TRON ON\n");
  #endif
  
  #ifdef DEBUG_MCP_IF
    inject_test_messages();
  #endif

  /* loop for ever */
  for(;;) 
  {
    /* RUN EVERY LOOP */
    
    /* read data from the interface into the inqueue if data available */
    /* but make sure there is space to store that byte! i.e. > 0       */
    if((number_free_bytes_in_queue_to_write_too(&in_queue) > 0) && GET_DOR) 
    {
      /* read a byte from the interface and store in incoming queue */
      store_to_current_write_byte(&in_queue, read_fifo_byte());
      
      #ifdef DEBUG_MCP_2     
        printf("Read %c\n", in_queue.data[in_queue.store_next_byte]);
      #endif

      /* increment the queue after storing a byte in it */
      increment_store_next_index(&in_queue,1);
    }

    /* RUN EVERY LOOP */
    
    /* if any data waiting in out_queue to be written to the interface */
    if((number_bytes_in_queue_to_read(&out_queue) > 0) && GET_DIR) 
    {
      /* read the byte from the out_queue and write the byte out to the interface */
      write_fifo_byte(get_current_read_byte(&out_queue));
      
      #ifdef DEBUG_MCP_2
        printf("Wrote %c\n", out_queue.data[out_queue.read_next_byte]);
      #endif
      
      /* move out_queue read pointer on having written it to the interface */
      increment_read_next_index(&out_queue, 1);
    }

    /****************************************************************************************/
    /* We only want to run the is_packet_available check every so often i.e. not ever loop */
    /* so that the reading in and writing out loops get more of the processing time         */
    /****************************************************************************************/

    if(packet_check_counter >= MAX_COMMAND_WAIT)
    {
      if(is_packet_available( &in_queue ) == TRUE)
      {
        int hold_read_pointer = in_queue.read_next_byte;
      
        /* process the packet based on type byte */
        switch(get_packet_type(&in_queue))
        {
          case TextCommand:
            
            /****************************************************************************/
            /* A text command is a command (plus options parameters) provided as ASCII  */
            /* e.g. PING\n is a text command                                            */
            /* Text commands are \n terminated (ASCII 0xA)                              */
            /****************************************************************************/
            
            /* get the text string of the command name*/
            get_command_string(&in_queue, command_string);
            
            if(TRON) {printf("Command Received '%s'\n", command_string);}
            
            /* get the command id from the command_string */                   
            commandID = get_commandID_from_string( command_string );
            
            if(TRON) {printf("CommandID %d\n", commandID);}
            
            /* begin execution of its first segment  */        
            if(process_text_command(commandID, &in_queue, &out_queue) == FALSE)
            {
              /* process_command would not process the command as it would have failed  */
              /* so we need to rewind the in_queue.read_next_byte to before the command */
              /* was read from the queue.  This is so we can try the same command again */
              /* later when what ever caused the process_command to return an error has */
              /* cleared (usually a full out_queue!).  If the out_queue is full then    */
              /* we will effectively stop processing from the in_queue as there is no   */
              /* where to store the output from the commands when they are executed     */
              in_queue.read_next_byte = hold_read_pointer;
            }
            else
            {
              /* process command completed - skip the '---' at end of packet */
              queue_skip_terminator_of_packet(&in_queue);
            }
            break;
            
          case BinaryCommand:
            
            /****************************************************************************/
            /* A binary command is a command (plus options parameters) provided as      */
            /* as binary data e.g. PING is command 1 (see CommandIDs in command.h)      */
            /* e.g. 1 is binary command for PING                                        */
            /* Binary commands have no terminators like Text Commands                   */
            /****************************************************************************/
            
            break;
            
          case BinaryData:
            
            /****************************************************************************/
            /* Binary Data should be read by a Text or Binary Command and not directly  */
            /* here i.e. if we got it here there is a problem                           */
            /****************************************************************************/
            
            if(TRON)
            {
              printf("Found BinaryData packet unexpectedly\n");
            }
            
            break;
            
          case GraphicDirectives:
            
            /****************************************************************************/
            /* Binary Graphic Directives to draw to the pi screen - this is here as an   */
            /* example and is not currently implemented YET!!                           */
            /****************************************************************************/
            
            break;
            
          case UnknownPacket:            
            /* Deliberate Fall through to default case */
          case Max_PacketType:
            /* Deliberate Fall through to default case */
            
         default:
            if(TRON)
            {
              printf("Packet Received had invalid PACKET_TYPE %d - packet ignored!!", get_packet_type(&in_queue));
            }
            break;
        }
      }
      
      /* reset the counter */
      packet_check_counter = 0;
    }
    else
    {
      packet_check_counter++;
    }
    
    /************************************************************************************/
    /* we only want to run the tick_command check every so often i.e. not on every loop */
    /* this is so the read in and write out queue commands above run more often than    */
    /* the tick check i.e. we want data going in and out of the queues alot compared    */
    /* to checking if a command needs to be ticked as this is an expensive operation    */
    /************************************************************************************/
    
    /* check to see if its time to check the command ticks */
    if( tick_check_counter >= MAX_TICK_WAIT )
    {
      /* check if any commands need a tick to do their next bit */
      tick_command(&in_queue, &out_queue);
      /* reset the counter */
      tick_check_counter = 0;
    }
    else /* now check the tick */
    {
      /* dont check yet */
      tick_check_counter++;
    }
    
    
  } /* for */
    
  return(0);
  
} /* main */

#ifdef DEBUG_MCP_IF
void inject_test_messages(void)
{
  /* ASSUMES STORING DATA AT START OF QUEUE */
  
  /* const char *test_message = "HELP\nHGHGG\nHELP PING\nHELP SHUTDOWN\nHELP TIME\nHELP DATE\nHELP RESET\nHELP HELP\nHELP REBOOT\n"; */
  /* const char *test_message = "CALLOC 5 1\nPILALLOC 1 /home/twri/Dev/retro/Amstrad/mcp/rpi_c/run_mcp_pi.sh\nRALLOC 1 0\n\nPISALLOC 1 /home/twri/Dev/retro/Amstrad/mcp/rpi_c/delete_me_save_test.sh 37\n"; */
  /* const char *test_message = "HELP\nHELP CALLOC\nHELP FALLOC\nHELP RALLOC\nHELP SALLOC\nHELP IALLOC\nHELP XALLOC\nHELP PILALLOC\nHELP PISALLOC\n"; */
  
  /* turn on debug by default */
  TRON = TRUE;
  
  
  /* QUEUE_SIZE 131070 */
  /*
  in_queue.read_next_byte   = 131068;
  in_queue.store_next_byte  = 131068;
  out_queue.read_next_byte  = 131068;
  out_queue.store_next_byte = 131068;
  */
  
  const char *test_message  = "PING\n";
  const char *test_message2 = "TIME\n";
  const char *test_message3 = "DATE\n";
  const char *test_message4 = "SHUTDOWN\n";
  const char *test_message5 = "REBOOT\n";

  inject_command(&in_queue, test_message,  TextCommand); 
  inject_command(&in_queue, test_message2, TextCommand);  
  inject_command(&in_queue, test_message3, TextCommand);  
  inject_command(&in_queue, test_message4, TextCommand);  
  inject_command(&in_queue, test_message5, TextCommand);  
  
  /*
  const char *test_message   = "CALLOC 5 1\n";
  const char *test_message2  = "SALLOC 1 0\n";
  const char *test_message3  = "RALLOC 1 0\n";

  printf("Injecting Message into in_queue\n");
  inject_command(&in_queue, test_message,   TextCommand); 
  inject_command(&in_queue, test_message2,  TextCommand); 
  inject_binary_data(&in_queue, "Some Test Data\n",  BinaryData, 1024); 
  inject_command(&in_queue, test_message3,  TextCommand); 
  */
  
  print_in_queue(0, 1200);
  
  printf("\n\n");
}

void inject_command(Queue *in_queue, const char *message, PacketType packet_type)
{
  /* packet start delimiters */
  for(int x = 0; x < 3; x++)
  {
    store_to_current_write_byte(in_queue, '+');
    increment_store_next_index(in_queue, 1);
  }
  /* low byte packet_size */
  store_to_current_write_byte(in_queue, strlen(message) + FRONT_PACKET_OVERHEAD);
  increment_store_next_index(in_queue, 1);
  /* high byte packet_size */
  store_to_current_write_byte(in_queue, 0);
  increment_store_next_index(in_queue, 1);
  /* command packet type */  
  store_to_current_write_byte(in_queue, packet_type);
  increment_store_next_index(in_queue, 1);
  /* store the packet body */
  for(int x = 0; x < strlen(message); x++)
  {
    store_to_current_write_byte(in_queue, message[x]);
    increment_store_next_index(in_queue, 1);
  }
  /* packet terminator delimiters */
  for(int x = 0; x < 3; x++)
  {
    store_to_current_write_byte(in_queue, '-');
    increment_store_next_index(in_queue, 1);
  }
}

void inject_binary_data(Queue *in_queue, const char *message, PacketType packet_type, int page_size)
{
  /* packet start delimiters */
  for(int x = 0; x < 3; x++)
  {
    store_to_current_write_byte(in_queue, '+');
    increment_store_next_index(in_queue, 1);
  }
  /* low byte packet_size */
  store_to_current_write_byte(in_queue, (page_size + FRONT_PACKET_OVERHEAD) % 256);
  increment_store_next_index(in_queue, 1);
  /* high byte packet_size */
  store_to_current_write_byte(in_queue, (page_size + FRONT_PACKET_OVERHEAD) / 256);
  increment_store_next_index(in_queue, 1);
  /* command packet type */  
  store_to_current_write_byte(in_queue, packet_type);
  increment_store_next_index(in_queue, 1);
  /* store the packet body */
  for(int x = 0; x < strlen(message); x++)
  {
    store_to_current_write_byte(in_queue, message[x]);
    increment_store_next_index(in_queue, 1);
  }
  for(int x = 0; x < (page_size - strlen(message)); x++)
  {
    store_to_current_write_byte(in_queue, 0);
    increment_store_next_index(in_queue, 1);
  }
  /* packet terminator delimiters */
  for(int x = 0; x < 3; x++)
  {
    store_to_current_write_byte(in_queue, '-');
    increment_store_next_index(in_queue, 1);
  }
}

#endif
