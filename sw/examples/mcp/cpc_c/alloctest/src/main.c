// main.c - page commands test demo for the CPC-CPLink board
//
// Copyright (C) 2019  Revaldinho/Shifters74
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
// 



#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <firmware.h>
#include <fifolib.h>

#define RESPONSE_LENGTH 2048

/* define functions below as main MUST be first function!! */
void send_command_to_pi(const uint8_t *command);
void read_response_from_pi(uint8_t *response);
uint8_t get_byte_from_pi(void);
void read_binary_data_from_pi(uint8_t *memory_buffer, uint16_t number_bytes_to_read);
void send_binary_data_to_pi(const uint8_t *memory_buffer, uint16_t number_bytes_to_write);
void copy_message_into_buffer(uint8_t *memory_buffer, const uint8_t *change_text, uint16_t position_to_add);
void clear_buffer(uint8_t *memory_buffer);

void main ( void ) 
{
	const uint8_t *command = NULL;
	const uint8_t *change_text = NULL;
	uint8_t *memory_buffer = (uint8_t *) 0x8000;  /* memory area on the CPC we will use */
	
	/* set screen mode 2 */
	scr_set_mode(2);

	/* reset the fifo queue */
	fifo_reset();

	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/***********************************/
	/* Create a memory allocation on   */
	/* the raspberry pi                */
	/***********************************/

	/* format of request:                                  */
	/* CALLOC NUMBER_PAGES SIZE_OF_PAGE_IN_KB\n            */
	/* example:                                            */
	/* CALLOC 5 8\n which requests 5 x 8KB of memory       */
	/* MAX NUMBER_PAGES is 9999                            */
	/* MAX SIZE_OF_PAGE_IN_KB is 32                        */
	/* return values:                                      */
	/* CALLOC ERROR 1\n when parameters are incorrect      */
	/* CALLOC ERROR 2\n when pi can't allocate memory      */
	/* CALLOC ALLOCATION_ID\n when complete successfully   */
	/* The ALLOCATION_ID is a positive int (1..20)         */
	
	command = "CALLOC 8 1\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
    printf("Response: %s\r\n",memory_buffer);
	/* response CALLOC 1\n meaning memory allocation 1 created which consists of 8 pages (starting from 0) of 1K memory on the pi */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/***********************************/
	/* Load in a file on the pi into   */
	/* the memory allocated            */
	/***********************************/
		
	/* format of request:                                     */
	/* PILALOC ALLOCATION_ID /PATH/FILENAME\n                 */
	/* example:                                               */
	/* PILALOC 1 /home/pi/mcp_pi/HowThisWorks.txt\n           */
	/* return values:                                         */
	/* PILALOC ALLOCATION_ID FILE_SIZE\n                      */
	/* PILALOC ERROR 1\n when parameters are incorrect        */
	/* PILALOC ERROR 2\n unable to load file                  */
	/* PILALOC ERROR 3\n file to large to fit in memory       */
	/*                    allocation                          */
	
	command = "PILALLOC 1 /home/pi/mcp_pi/run_mcp_pi.sh\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
    printf("Response: %s\r\n",memory_buffer);
	/* response PILALOC 1 37\n meaning the pi read 37 bytes into the memory allocation 1 (starting at page 0) */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/***********************************/
	/* Retrieve page 0 of the memory   */
	/* allocation we created as it     */
	/* contains the contents of the    */
	/* small file just loaded	       */
	/***********************************/

	/* format of request:                                */
	/* RALLOC ALLOCATION_ID PAGE_NUMBER\n                */
	/* example:                                          */
	/* RALLOC 1 0\n which retrieve the first page(from 0)*/
	/* for memory in allocation_id 1                     */
	/* return values:                                    */
	/* RALLOC OK 1 0\n                                   */
	/* The binary data is the number of KB you specified */
	/* in your CALLOC command and follows in the next    */
    /* packet                                            */
	/* RALLOC ERROR 1\n when parameters are incorrect    */
	
	command = "RALLOC 1 0\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);    
	printf("Response: %s\r\n",memory_buffer);
	
	/* response RALLOC OK 1 0\n meaning we requested memory page 0 from memory allocation 1 and got it (the OK bit ) */	
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* now read the binary data which is 1K in size */
	read_binary_data_from_pi(memory_buffer,1024);
	
	/* response is 1K of binary data (the size of each page specified in the CALLOC command) */
	
	/* when retrieving pages you have to request a whole page and can not specify part of a   */
	/* page even if its only part of a page containing the data - you must get the whole page */
	
	/* print the response as its a text file with null terminated string */
	printf("BinReceive: %s\r\n", memory_buffer);

	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/*********************************************/
	/* lets change the binary data just received */
	/*********************************************/

	change_text = "echo \"We Changed This!\"";
	copy_message_into_buffer(memory_buffer, change_text, 37);
	
	printf("Changed 1K page to: %s\r\n", memory_buffer);
	
	/* we copied the change text into the memory buffer starting at the position 37 */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/*********************************************/
	/* Now lets write the memory page we have    */
	/* updated back to the pi                    */
	/*********************************************/

	/* format of request:                                     */
	/* SALLOC ALLOCATION_ID PAGE_NUMBER\n                     */
	/* example:                                               */
	/* SALLOC 1 0\n                                           */
    /*   BINARY_DATA follows in the next packet where the     */
    /*   length of BINARY_DATA is the page size used in       */       
    /*   CALLOC creation                                      */
	/* return values:                                         */
	/* SALLOC OK ALLOCATION_ID PAGE_NUMBER\n                  */
	/* e.g. SALLOC OK 1 0\n                                   */
	/* SALLOC ERROR 1\n when parameters are incorrect         */
	
	command = "SALLOC 1 0\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);
	printf("Sent: %s\r\n",command);
	/* meaning we are writing page 0 of memory allocation 1 back */
	
	/* send the binary data to the pi */
	send_binary_data_to_pi(memory_buffer, 1024);
	
	printf("BinSent: %s\r\n", memory_buffer);
	
	/* when you send a page of data you must send the whole page of data back to the pi even if only modifying a small part of it */
	/* sending this page has no affect on the other pages in the memory allocation 1 on the pi                                    */
	
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
	
    printf("Response: %s\r\n",memory_buffer);
	/* response SALLOC OK 1 0\n meaning we sent page 0 of memory allocation 1 back successfully (the OK bit) */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/*********************************************/
	/* Now save the memory allocation on the pi  */
	/* to a file on the pi (with our changes)    */
	/*********************************************/
	
	/* format of request:                                       */
	/* PISALLOC ALLOCATION_ID /PATH/FILENAME FILESIZE\n         */
	/* FILESIZE is size you want to save from memory allocation */
	/* example:                                                 */
	/* PISALLOC 1 /home/pi/mcp_pi/HowThisWorks.txt 4086\n       */
	/* return values:                                           */
	/* PISALLOC ALLOCATION_ID OK\n                              */
	/* PILALOC ERROR 1\n parameter error                        */
	/* PILALOC ERROR 2\n unable to write to file                */
	
	command = "PISALLOC 1 /home/pi/mcp_pi/delete_me_save_test.sh 60\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer); 
    printf("Response: %s\r\n",memory_buffer);
	/* response PISALLOC 1 OK\n meaning we saved memory allocation 1 successfully to the file specified - now check on the pi that this is true! */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/***********************************/
	/* Create a memory allocation on   */
	/* the raspberry pi                */
	/***********************************/

	/* format of request:                                  */
	/* CALLOC NUMBER_PAGES SIZE_OF_PAGE_IN_KB\n            */
	/* example:                                            */
	/* CALLOC 5 8\n which requests 5 x 8KB of memory       */
	/* MAX NUMBER_PAGES is 9999                            */
	/* MAX SIZE_OF_PAGE_IN_KB is 32                        */
	/* return values:                                      */
	/* CALLOC ERROR 1\n when parameters are incorrect      */
	/* CALLOC ERROR 2\n when pi can't allocate memory      */
	/* CALLOC ALLOCATION_ID\n when complete successfully   */
	/* The ALLOCATION_ID is a positive int (1..20)         */
	
  	command = "CALLOC 2 16\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
    printf("Response: %s\r\n",memory_buffer);
	/* response CALLOC 2\n meaning memory allocation 2 created which consists of 2 pages (starting from 0) of 16K memory on the pi */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/***********************************/
	/* Load in a file on the pi into   */
	/* the memory allocated            */
	/***********************************/
		
	/* format of request:                                     */
	/* PILALOC ALLOCATION_ID /PATH/FILENAME\n                 */
	/* example:                                               */
	/* PILALOC 1 /home/pi/mcp_pi/HowThisWorks.txt\n           */
	/* return values:                                         */
	/* PILALOC ALLOCATION_ID FILE_SIZE\n                      */
	/* PILALOC ERROR 1\n when parameters are incorrect        */
	/* PILALOC ERROR 2\n unable to load file                  */
	/* PILALOC ERROR 3\n file to large to fit in memory       */
	/*                    allocation                          */
	
	command = "PILALLOC 2 /home/pi/mcp_pi/run_mcp_pi.sh\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
    printf("Response: %s\r\n",memory_buffer);
	/* response PILALOC 2 37\n meaning the pi read 37 bytes into the memory allocation 1 (starting at page 0) */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/***********************************************/
	/* Insert a string into the memory of the file */
    /* on the pi in the memory allocated           */
	/***********************************************/
		
  	/* format of request:                                                          */
	/* IALLOC ALLOCATION_ID INSERT_AT INSERT_LENGTH\n                              */
	/* example:                                                                    */
	/* IALLOC 1 20 50\n                                                            */
    /* Where 1 is ALLOC_ID and 20 is 20 bytes form start of ALLOC_ID memory        */
    /* The BinaryData packet follows the sending of IALLOC containing the data to insert */
	/* return values:                                                              */
	/* IALLOC OK ALLOCATION_ID\n                                                   */
	/* IALLOC ERROR 1\n when parameters are incorrect                              */
	/* IALLOC ERROR 2\n unable to allocate memory on pi                            */
	/* IALLOC ERROR 3\n insert_at beyond end of allocation memory                  */

    command = "IALLOC 2 12 12\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);
	printf("Sent: %s\r\n",command);
	/* meaning we are writing page 0 of memory allocation 1 back */
	
	/* send the binary data to the pi */
	send_binary_data_to_pi("Hello World\n", 12);
	
	printf("BinSent: Hello World\\n\r\n");
	
	/* when you send a page of data you must send the whole page of data back to the pi even if only modifying a small part of it */
	/* sending this page has no affect on the other pages in the memory allocation 1 on the pi                                    */
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
	
    printf("Response: %s\r\n",memory_buffer);
	/* response IALLOC OK 2\n meaning we sent page 0 of memory allocation 1 back successfully (the OK bit) */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/*********************************************/
	/* Now save the memory allocation on the pi  */
	/* to a file on the pi (with our changes)    */
	/*********************************************/
	
	/* format of request:                                       */
	/* PISALLOC ALLOCATION_ID /PATH/FILENAME FILESIZE\n         */
	/* FILESIZE is size you want to save from memory allocation */
	/* example:                                                 */
	/* PISALLOC 1 /home/pi/mcp_pi/HowThisWorks.txt 4086\n       */
	/* return values:                                           */
	/* PISALLOC ALLOCATION_ID OK\n                              */
	/* PILALOC ERROR 1\n parameter error                        */
	/* PILALOC ERROR 2\n unable to write to file                */
	
	command = "PISALLOC 2 /home/pi/mcp_pi/delete_me_save_test2.sh 49\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer); 
    printf("Response: %s\r\n",memory_buffer);
	/* response PISALLOC 2 OK\n meaning we saved memory allocation 1 successfully to the file specified - now check on the pi that this is true! */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();


	/***********************************/
	/* Create a memory allocation on   */
	/* the raspberry pi                */
	/***********************************/

	/* format of request:                                  */
	/* CALLOC NUMBER_PAGES SIZE_OF_PAGE_IN_KB\n            */
	/* example:                                            */
	/* CALLOC 5 8\n which requests 5 x 8KB of memory       */
	/* MAX NUMBER_PAGES is 9999                            */
	/* MAX SIZE_OF_PAGE_IN_KB is 32                        */
	/* return values:                                      */
	/* CALLOC ERROR 1\n when parameters are incorrect      */
	/* CALLOC ERROR 2\n when pi can't allocate memory      */
	/* CALLOC ALLOCATION_ID\n when complete successfully   */
	/* The ALLOCATION_ID is a positive int (1..20)         */
	
  	command = "CALLOC 2 16\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
    printf("Response: %s\r\n",memory_buffer);
	/* response CALLOC 3\n meaning memory allocation 3 created which consists of 2 pages (starting from 0) of 16K memory on the pi */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/***********************************/
	/* Load in a file on the pi into   */
	/* the memory allocated            */
	/***********************************/
		
	/* format of request:                                     */
	/* PILALOC ALLOCATION_ID /PATH/FILENAME\n                 */
	/* example:                                               */
	/* PILALOC 1 /home/pi/mcp_pi/HowThisWorks.txt\n           */
	/* return values:                                         */
	/* PILALOC ALLOCATION_ID FILE_SIZE\n                      */
	/* PILALOC ERROR 1\n when parameters are incorrect        */
	/* PILALOC ERROR 2\n unable to load file                  */
	/* PILALOC ERROR 3\n file to large to fit in memory       */
	/*                    allocation                          */
	
	command = "PILALLOC 3 /home/pi/mcp_pi/run_mcp_pi.sh\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
    printf("Response: %s\r\n",memory_buffer);
	/* response PILALOC 3 37\n meaning the pi read 37 bytes into the memory allocation 1 (starting at page 0) */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/*********************************************/
	/* Now cut memory from the allocation on the */
	/* file on the pi                            */
	/*********************************************/
	
 	/* format of request:                                                            */
	/* XALLOC ALLOCATION_ID START_CUT LENGTH_OF_CUT\n                                */
	/* example:                                                                      */
	/* XALLOC 1 20 50\n Will cut out 50_bytes of data from position 20 in allocation */
	/* return values:                                                                */
	/* XALLOC OK ALLOCATION_ID\n                                                     */
	/* XALLOC ERROR 1\n when parameters are incorrect                                */
	/* XALLOC ERROR 2\n position+cut_length go beyond end of allocation memory       */
	

	command = "XALLOC 3 7 4\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer);
    printf("Response: %s\r\n",memory_buffer);
	/* response XALLOC 1 37\n meaning the pi read 37 bytes into the memory allocation 1 (starting at page 0) */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/*********************************************/
	/* Now save the memory allocation on the pi  */
	/* to a file on the pi (with our changes)    */
	/*********************************************/
	
	/* format of request:                                       */
	/* PISALLOC ALLOCATION_ID /PATH/FILENAME FILESIZE\n         */
	/* FILESIZE is size you want to save from memory allocation */
	/* example:                                                 */
	/* PISALLOC 1 /home/pi/mcp_pi/HowThisWorks.txt 4086\n       */
	/* return values:                                           */
	/* PISALLOC ALLOCATION_ID OK\n                              */
	/* PILALOC ERROR 1\n parameter error                        */
	/* PILALOC ERROR 2\n unable to write to file                */
	
	command = "PISALLOC 3 /home/pi/mcp_pi/delete_me_save_test3.sh 45\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer); 
    printf("Response: %s\r\n",memory_buffer);
	/* response PISALLOC 3 OK\n meaning we saved memory allocation 3 successfully to the file specified - now check on the pi that this is true! */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();


	/*****************************************************************/
	/* As we have finished lets free the memory allocation on the pi */
	/*****************************************************************/
	
	/* format of request:                                */
	/* FALLOC ALLOCATION_ID\n                            */
	/* example:                                          */
	/* FALLOC 1\n which releases all memory associated   */
	/* with allocation_id 1.  The allocation id is what  */
	/* what was returned after doing a CALLOC request    */
	/* NOTE: When you have allocated memory with CALLOC  */
	/* and are now using FALLOC to release it - you are  */
	/* releaseing all the memory for that allocation_ID  */
	/* i.e. you can't release just part of the memory.   */
	/* return values:                                    */
	/* FALLOC ERROR 1\n when parameters are incorrect    */
    /* FALLOC FREE\n when all memory for that allocation */
	/* id has been released in the pi                    */
	
	command = "FALLOC 1\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer); 
    printf("Response: %s\r\n",memory_buffer);
	/* response FALLOC FREE\n meaning we released memory allocation 1 successfully hence it can no longer be used */
    
	command = "FALLOC 2\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer); 
    printf("Response: %s\r\n",memory_buffer);
	/* response FALLOC FREE\n meaning we released memory allocation 2 successfully hence it can no longer be used */
    
	command = "FALLOC 3\n";
	
	/* send the command to the pi */
	send_command_to_pi(command);	
	printf("Sent: %s\r\n",command);
	
	/* clear the buffer */
	clear_buffer( memory_buffer );
	
	/* read the response from the pi */
	read_response_from_pi(memory_buffer); 
    printf("Response: %s\r\n",memory_buffer);
	/* response FALLOC FREE\n meaning we released memory allocation 3 successfully hence it can no longer be used */
	
	
	/************************************/
	/* Wait for a key press as the CPC  */
	/* will hard reset when the program */
	/* exits - so wait and give time to */
	/* read the send/respones above     */
	/************************************/
	
	printf("Finished.\r");
	printf("Press any key to exit\r");
	km_wait_char();
}

void send_command_to_pi(const uint8_t *command)
{
	uint16_t counter = 0;
	
	/* count to the end of the null terminated string */
	while(command[counter] != 0)
	{
		counter++;
	}
	
	/* send the +++ delimiter */
	fifo_out_byte('+');
	fifo_out_byte('+');
	fifo_out_byte('+');
	
	/* send the size of the command - low byte (+3 is packet over head) */
	fifo_out_byte(counter + 3);

	/* send the size of the command - high byte */
	fifo_out_byte(0);
	
	/* send the packet_type */
	fifo_out_byte(1);
	
	/* send the command to the pi */
	for(uint8_t index = 0; index < counter; index++)
	{
		fifo_out_byte(command[ index ]);
	}
	
	/* send the --- delimiter */
	fifo_out_byte('-');
	fifo_out_byte('-');
	fifo_out_byte('-');	
}

void read_response_from_pi(uint8_t *response)
{
	uint16_t index = 0;
	uint8_t packet_header[6];
	
	/* read the first 6 bytes */
	while(index < 6)
	{
		packet_header[index] = get_byte_from_pi();
		index++;
	}
	
	/* if we got a packet header delimiter '+++' as expected and packet type is TextCommand (1) */
	if(packet_header[0] == '+' && packet_header[1] == '+' && packet_header[2] == '+' && packet_header[5] == 1)
	{
		uint16_t number_bytes = packet_header[3] + (packet_header[4] * 256) - 3; /* -3 as size includes packet header */
		
		index = 0;		
		/* read the message */
		while(index < number_bytes)
		{
			response[index] = get_byte_from_pi();
			index++;
		}
		
		/* read the trailing delimiters '---' */
		index = 0;
		while(index < 3)
		{
			get_byte_from_pi();
			index++;
		}		
	}
	else
	{
		printf("Received malformed packet\r\n");
	}	
}

uint8_t get_byte_from_pi(void)
{
	uint8_t did_we_get_byte = 0;
	uint8_t byte_received = 0;
	
	while(did_we_get_byte == 0)
	{
		/* try and get a byte from the pi */
		did_we_get_byte = fifo_in_byte(&byte_received);

	} /* while */
	
	return(byte_received);
}

void send_binary_data_to_pi(const uint8_t *memory_buffer, uint16_t number_bytes_to_write)
{
	uint16_t index = 0;
	uint16_t total_bytes_to_send = number_bytes_to_write + 3;
	
	/* send the +++ delimiter */
	fifo_out_byte('+');
	fifo_out_byte('+');
	fifo_out_byte('+');
	
	/* send the size of the command - low byte (+3 is packet over head) */
	fifo_out_byte(total_bytes_to_send % 256);

	/* send the size of the command - high byte */
	fifo_out_byte(total_bytes_to_send / 256);
	
	/* send the packet_type */
	fifo_out_byte(3);
	
	/* send the command to the pi */
	for(index = 0; index < number_bytes_to_write; index++)
	{
		fifo_out_byte(memory_buffer[ index ]);
	}
	
	/* send the --- delimiter */
	fifo_out_byte('-');
	fifo_out_byte('-');
	fifo_out_byte('-');	
}

void read_binary_data_from_pi(uint8_t *memory_buffer, uint16_t number_bytes_to_read)
{
	uint16_t index = 0;
	uint8_t packet_header[6];
	
	/* read the first 6 bytes */
	while(index < 6)
	{
		packet_header[index] = get_byte_from_pi();
		index++;
	}
	
	/* if we got a packet header delimiter '+++' as expected and packet type is BinaryData (3) */
	if(packet_header[0] == '+' && packet_header[1] == '+' && packet_header[2] == '+' && packet_header[5] == 3)
	{
		uint16_t number_bytes = packet_header[3] + (packet_header[4] * 256) - 3; /* -3 as size includes packet header */
		
		/* printf("Need To Read: %d\r", number_bytes); */
		
		index = 0;		
		/* read the message */
		while(index < number_bytes_to_read)
		{
			memory_buffer[index] = get_byte_from_pi();
			index++;
			
			/* printf("Got %d\r", index); */
		}
		
		/* read the trailing delimiters '---' */
		index = 0;
		while(index < 3)
		{
			get_byte_from_pi();
			index++;
		}		
	}
	else
	{
		printf("Received malformed packet\r\n");
	}	
}

/* this function just copies the message into the response buffer provided           */
/* it makes no attempt to check the copy fits etc as this is a simple test program!! */
void copy_message_into_buffer(uint8_t *memory_buffer, const uint8_t *change_text, uint16_t position_to_add)
{
	uint16_t counter = 0;
	
	/* count to the end of the null terminated string */
	while(change_text[counter] != 0)
	{
		counter++;
	}
	
	/* copy the change_text a byte at a time into the buffer */
	for(uint16_t index = 0; index < counter; index++)
	{
		memory_buffer[index + position_to_add] = change_text[index];
	}
	
	memory_buffer[position_to_add + counter + 1] = 0;
}

void clear_buffer(uint8_t *memory_buffer)
{
	/* clear every byte in the buffer */
	for(uint16_t index = 0; index < RESPONSE_LENGTH; index++)
	{
		memory_buffer[index] = 0;
	}
}

