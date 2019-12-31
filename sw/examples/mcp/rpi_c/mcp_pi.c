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
#include "mcp_pi.h"
#include "fifolib.h"
#include "packet.h"
#include "queue.h"
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

int binary_data_receive_mode = 0;
int binary_data_send_mode = 0;

/* set the binary data receive mode */
void set_binary_data_receive_lock_mode(int bytes_to_receive)
{
    /* this mode focuses on receiving BinaryData from the cpc */
    /* as quick as possible rather than patiently waiting for */
    /* data to arrive. This is to reduce the extra processing */
    /* over head of continual rechecking commands and packets */
    /* etc waiting for the binary to completely arrive        */

    #ifndef DEBUG_MCP_IF
        #ifdef OPTIMISE_IO
            /* set the receive mode to the number of bytes we want before */
            /* processing command packets again                           */
            binary_data_receive_mode = bytes_to_receive - number_bytes_in_queue_to_read(&in_queue);
        #endif
    #endif
}

/* set the binary data send mode */
void set_binary_data_send_lock_mode(int bytes_to_send)
{
    /* this mode focuses on sending BinaryData to the cpc      */
    /* as quick as possible rather than patiently waiting for  */
    /* data to be sent around the main loop. This is to reduce */
    /* the time for the main loop by avoiding the  over head   */
    /* of continual checking for commands and packets incoming */

    #ifndef DEBUG_MCP_IF
        #ifdef OPTIMISE_IO
            /* set the send mode to the number of bytes that need to be sent */
            binary_data_send_mode = bytes_to_send + number_bytes_in_queue_to_read(&in_queue);
        #endif
    #endif
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

            /* if binary_data_receive_mode > 0 */
            if(binary_data_receive_mode > 0)
            {
                /* decrement it as we have a byte */
                binary_data_receive_mode--;
            }
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

            /* sent a byte and we are tracking */
            if(binary_data_send_mode > 0)
            {
                /* decrement count */
                binary_data_send_mode--;
            }
        }

        /****************************************************************************************/
        /* We only want to run the is_complete_packet_available check every so often i.e. not   */
        /* every loop so that the reading in and writing out loops get more of the processing   */
        /* time.  It will also can not run if a receive or send lock has been enabled which is  */
        /* to optimise the sending/receiving of BinaryData by processing no commands while they */
        /* are active.  Once complete then commands will be processed again                     */ 
        /****************************************************************************************/

        if((packet_check_counter >= MAX_COMMAND_WAIT) && (binary_data_receive_mode == 0) && (binary_data_send_mode == 0))
        {
            if(is_complete_packet_available( &in_queue ) == TRUE)
            {
                int hold_read_pointer = in_queue.read_next_byte;
              
                /* process the packet based on packet type byte */
                switch(get_packet_type(&in_queue))
                {
                    case TextCommand:
                        
                        /****************************************************************************/
                        /* A text command is a command (plus options parameters) provided as ASCII  */
                        /* e.g. PING\n is a text command                                            */
                        /* Text commands are \n terminated (ASCII 0xA)                              */
                        /****************************************************************************/
                        
                        /* if we can get the text string of the command name */
                        if(get_command_text(&in_queue, command_string))
                        {                            
                            if(TRON) {printf("Text Command Received '%s'\n", command_string);}
                            
                            /* get the command id from the command_string */                   
                            commandID = get_commandID_from_string( command_string );
                            
                            if(TRON) {printf("CommandID %d\n", commandID);}
                            
                            /* if we completed processing the command */        
                            if(process_command(commandID, &in_queue, &out_queue))
                            {
                                /* process command completed - skip the '---' at end of packet */
                                queue_skip_terminator_of_packet(&in_queue);
                            }
                            else /* command did not complete */
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
                        }
                        else /* we did not get the command name */
                        {
                            char *response = "Bad Text Command\n";

                            /* command name was too long or too short e.g. \n only */
                            if(TRON) {printf("Bad Text Command Received - DISCARDING\n");}

                            /* write Unknown Command response to cpc */
                            write_data_to_packet(&out_queue, (BYTE *) response, strlen(response), TextCommand);

                            /* skip the '---' at end of packet */
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
                        
                        /* get the command id from the packet header */                   
                        commandID = get_command_bin(&in_queue);
                        
                        if(TRON) {printf("Binary Command Received %d (%s)\n", commandID, get_command_name_from_id(commandID));}
                        
                        /* begin execution of its first segment  */        
                        if(process_command(commandID, &in_queue, &out_queue) == FALSE)
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

                }  /* switch */

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
        if((tick_check_counter >= MAX_TICK_WAIT) && (binary_data_receive_mode == 0) && (binary_data_send_mode == 0))
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

    
    const char *test_message9  = "\n";
    const char *test_message10  = "";
    const char *test_message11  = "PANG\n";
    const char *test_message12  = "WHATALOADOFNONSENSEHASBEENENTEREDHERETOCHECKWECANERRORHANDLEIT\n";
    const char *test_message13  = "PING";
    const char *test_message  = "PING\n";

    inject_command(&in_queue, test_message9,  TextCommand); 
    inject_command(&in_queue, test_message10,  TextCommand); 
    inject_command(&in_queue, test_message11,  TextCommand); 
    inject_command(&in_queue, test_message12,  TextCommand); 
    inject_command(&in_queue, test_message13,  TextCommand); 
    inject_command(&in_queue, test_message,  TextCommand); 

    /*
    const char *test_message  = "PING\n";
    const char *test_message2 = "TIME\n";
    const char *test_message3 = "DATE\n";
    const char *test_message4 = "SHUTDOWN\n";
    const char *test_message5 = "REBOOT\n";
    const char *test_message6 = "VERSION\n";
    const char *test_message7 = "WIFI\n";
    const char *test_message8 = "SHELLEXEC ls -al /home\n";

    inject_command(&in_queue, test_message,  TextCommand); 
    inject_command(&in_queue, test_message2, TextCommand);  
    inject_command(&in_queue, test_message3, TextCommand);  
    inject_command(&in_queue, test_message4, TextCommand);  
    inject_command(&in_queue, test_message5, TextCommand);  
    inject_command(&in_queue, test_message6, TextCommand);  
    inject_command(&in_queue, test_message7, TextCommand);  
    inject_command(&in_queue, test_message8, TextCommand);  
    */

    printf("Injecting Message into in_queue\n");
    /* turn tron off as output for injection is nonsense as its aimed at out_queue not in_queue as we are using */
    TRON = FALSE;

    /* inject BinCommand CreateAlloc = 21, 5 pages of 1KB */
    start_build_data_packet(&in_queue, 4, BinaryCommand);
    add_4_bytes_of_data_to_packet(&in_queue, CreateAlloc, 5, 0, 1);
    finish_build_data_packet_and_send(&in_queue);

    /* inject BinCommand LoadAlloc = 27, alloc 1, FILE_NAME */
    char *file_name = "./run_mcp_pi.sh\n";
    start_build_data_packet(&in_queue, 2 + strlen(file_name), BinaryCommand);
    add_byte_of_data_to_packet(&in_queue, LoadAlloc);
    add_byte_of_data_to_packet(&in_queue, 1);
    add_data_to_packet(&in_queue, (BYTE *) file_name, strlen(file_name));
    finish_build_data_packet_and_send(&in_queue);

    /* inject BinCommand RetrieveAlloc = 23, page 0 (0..4 pages) for alloc_id 1 */
    start_build_data_packet(&in_queue, 4, BinaryCommand);
    add_4_bytes_of_data_to_packet(&in_queue, RetrieveAlloc, 1, 0, 0);
    finish_build_data_packet_and_send(&in_queue);

    /* inject BinCommand StoreAlloc = 24, alloc 1, page 0 */
    start_build_data_packet(&in_queue, 4, BinaryCommand);
    add_4_bytes_of_data_to_packet(&in_queue, StoreAlloc, 1, 0, 0);
    finish_build_data_packet_and_send(&in_queue);
    inject_binary_data(&in_queue, "ABCDEFGH\n", BinaryData, 1024);

    /* inject BinCommand RetrieveAlloc = 23, page 0 (0..4 pages) for alloc_id 1 */
    start_build_data_packet(&in_queue, 4, BinaryCommand);
    add_4_bytes_of_data_to_packet(&in_queue, RetrieveAlloc, 1, 0, 0);
    finish_build_data_packet_and_send(&in_queue);

    /* inject BinCommand LoadAlloc = 27, alloc 1, FILE_NAME */
    start_build_data_packet(&in_queue, 2 + strlen(file_name), BinaryCommand);
    add_byte_of_data_to_packet(&in_queue, LoadAlloc);
    add_byte_of_data_to_packet(&in_queue, 1);
    add_data_to_packet(&in_queue, (BYTE *) file_name, strlen(file_name));
    finish_build_data_packet_and_send(&in_queue);

    /* inject BinCommand InsertAlloc = 25, alloc 1 */
    start_build_data_packet(&in_queue, 10, BinaryCommand);
    add_byte_of_data_to_packet(&in_queue, InsertAlloc);
    add_byte_of_data_to_packet(&in_queue, 1);
    add_int_to_packet(&in_queue, 12); /* insert at */
    add_int_to_packet(&in_queue, 12); /* insert length */
    finish_build_data_packet_and_send(&in_queue);
    inject_binary_data(&in_queue, "Hello World\n", BinaryData, 12);
    
    /* inject BinCommand SaveAlloc = 28, alloc 1 */
    char *file_name2 = "./delete_me_save_test.sh\n";
    start_build_data_packet(&in_queue, 6 + strlen(file_name2), BinaryCommand);
    add_byte_of_data_to_packet(&in_queue, SaveAlloc);
    add_byte_of_data_to_packet(&in_queue, 1); /* alloc id 1 */
    add_int_to_packet(&in_queue, 49); /* size to save to file */
    add_data_to_packet(&in_queue, (BYTE *) file_name2, strlen(file_name2));
    finish_build_data_packet_and_send(&in_queue);
    
    /* inject BinCommand LoadAlloc = 27, alloc 1, FILE_NAME */
    start_build_data_packet(&in_queue, 2 + strlen(file_name), BinaryCommand);
    add_byte_of_data_to_packet(&in_queue, LoadAlloc);
    add_byte_of_data_to_packet(&in_queue, 1);
    add_data_to_packet(&in_queue, (BYTE *) file_name, strlen(file_name));
    finish_build_data_packet_and_send(&in_queue);

    /* inject BinCommand CutAlloc = 26, alloc 1 */
    start_build_data_packet(&in_queue, 10, BinaryCommand);
    add_byte_of_data_to_packet(&in_queue, CutAlloc);
    add_byte_of_data_to_packet(&in_queue, 1);
    add_int_to_packet(&in_queue, 7); /* cut from */
    add_int_to_packet(&in_queue, 4); /* cut length */
    finish_build_data_packet_and_send(&in_queue);
        
    /* inject BinCommand SaveAlloc = 28, alloc 1 */
    char *file_name3 = "./delete_me_save_test2.sh\n";
    start_build_data_packet(&in_queue, 6 + strlen(file_name3), BinaryCommand);
    add_byte_of_data_to_packet(&in_queue, SaveAlloc);
    add_byte_of_data_to_packet(&in_queue, 1); /* alloc id 1 */
    add_int_to_packet(&in_queue, 33); /* size to save to file */
    add_data_to_packet(&in_queue, (BYTE *) file_name3, strlen(file_name3));
    finish_build_data_packet_and_send(&in_queue);
    
    /* inject BinCommand FreeAlloc = 22, allocation 1 */
    start_build_data_packet(&in_queue, 2, BinaryCommand);
    add_byte_of_data_to_packet(&in_queue, FreeAlloc);
    add_byte_of_data_to_packet(&in_queue, 1);
    finish_build_data_packet_and_send(&in_queue);

    /* turn TRON back following injection */
    TRON = TRUE;

    printf("Injecting Complete\n");

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
