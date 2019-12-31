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

#define OUTPUT_BUFFER_LENGTH 255
#define RESPONSE_BUFFER_LENGTH 2048

/* define functions below as main MUST be first function!! */
void send_bytes_to_pi(uint8_t *data, uint8_t size_to_send);
void read_response_from_pi(uint8_t *response);
uint8_t get_byte_from_pi(void);
void read_binary_data_from_pi(uint8_t *response_buffer, uint16_t number_bytes_to_read);
void send_binary_data_to_pi(const uint8_t *response_buffer, uint16_t number_bytes_to_write);
void copy_message_into_buffer(uint8_t *response_buffer, const uint8_t *change_text, uint16_t position_to_add);
void clear_buffer(uint8_t *response_buffer, uint16_t size_of_buffer);

void main ( void ) 
{
	const uint8_t *command = NULL;
	const uint8_t *change_text = NULL;
	uint8_t *output_buffer = (uint8_t *) 0x8000;  /* memory area to build message */
	uint8_t *response_buffer = (uint8_t *) 0x8100;  /* memory area tp hold response */
	const uint8_t *file_name = "/home/pi/mcp_pi/run_mcp_pi.sh\n";
	const uint8_t *new_file_name = "/home/pi/mcp_pi/delete_me_save_test.sh\n";
	const uint8_t *new_file_name2 = "/home/pi/mcp_pi/delete_me_save_test2.sh\n";
	const uint8_t *new_file_name3 = "/home/pi/mcp_pi/delete_me_save_test3.sh\n";

	/* set screen mode 2 */
	scr_set_mode(2);

	/* reset the fifo queue */
	fifo_reset();

	/***********************************/
	/* Create a memory allocation on   */
	/* the raspberry pi                */
	/***********************************/

	/* format of request:                                   */
	/* CALLOC NUM_PAGES SIZE_OF_PAGES                       */
	/* CALLOC (1 byte) value 21                             */
    /* NUM_PAGEs (2 bytes) range 1 to 9999                  */
    /* SIZE_OF_PAGES_IN_KB (1 byte) range 1 to 32           */
	/* example:                                             */
    /* 21 05 00 01                                          */
    /* where 05 + (00 << 8) is 5 pages of 8KB               */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("CAllOC \r\n");

	/* command id */
	output_buffer[0] = 21;
	/* number of pages lo/high */
	output_buffer[1] = 5;
	output_buffer[2] = 0;
	/* size of page to create (in KB) */
	output_buffer[3] = 1;

	/* send the command to the pi */
	send_bytes_to_pi(output_buffer, 4);

	/* format of response:                                  */
	/* CALLOC ALLOC_ID ERROR_ID                             */ 
    /* CALLOC 21 (1 byte)                                   */
    /* ALLOC_ID (1 byte) range 0 to 20                      */
    /* ALLOC_ID value 0 means error occured                 */
    /* ERROR_ID (1 byte) range 0 to 2                       */
	/* ERROR_ID value 0 no error occured                    */
	/* ERROR_ID value 1 when parameters are incorrect       */
	/* ERROR_ID value 2 when pi can't allocate memory       */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);

	/***********************************/
	/* Load in a file on the pi into   */
	/* the memory allocated            */
	/***********************************/
		
	/* format of request:                                  */
	/* PILALLOC ALLOC_ID /PATH/TO/FILENAME\n               */
	/* PILALLOC (1 byte) value 27                          */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* /PATH/TO/FILENAME is a string filename path  with   */
	/* \n termination                                      */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("PILAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 27;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	/* copy the path to file into the memory buffer */
    copy_message_into_buffer(&output_buffer[2], file_name, 0);

	/* send the command to the pi (30 is size of path and filename) */
	send_bytes_to_pi(output_buffer, 2 + 30);

	/* format of response:                                 */
	/* PILALLOC ALLOC_ID FILE_SIZE ERROR_ID                */
	/* PILALLOC (1 byte) value 27                          */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be modified      */
	/* FILE_SIZE (4 byte) range 0 to sizeof(int)           */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
    /* ERROR_ID (1 byte) range 0 to 3                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 unable to load file                */
	/* ERROR_ID value 3 file to large to fit allocation    */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);

	/***********************************/
	/* Retrieve page 0 of the memory   */
	/* allocation we created as it     */
	/* contains the contents of the    */
	/* small file just loaded	       */
	/***********************************/

	/* format of request:                                */
	/* RALLOC ALLOC_ID PAGE_NUMBER                       */
	/* RALLOC (1 byte) value 23                          */
	/* ALLOC_ID (1 byte) range 1 to 20                   */
	/* PAGE_NUMBER (2 bytes lo/high) value 0 to 9999     */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("RAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 23;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	/* retrieve page number (low/high byte) */
	output_buffer[2] = 0; /* page 0 */
	output_buffer[3] = 0;

	/* send the command (of 4 bytes) to the pi */
	send_bytes_to_pi(output_buffer, 4);

	/* format of response:                               */
	/* RALLOC ALLOC_ID PAGE_NUMBER ERROR_ID              */
	/* RALLOC (1 byte) value 23                          */
	/* ALLOC_ID (1 byte) range 0 to 20                   */
    /* ALLOC_ID value 0 means error occured              */
    /* ALLOC_ID value 1 to 20 alloc_id retreiving ok     */
	/*          A BinaryData packet of PAGE_NUMBER data  */
	/*          will follow this response packet.        */
	/* PAGE_NUMBER (2 bytes lo/high) value 0 to 9999     */
	/*          The page number retrieved                */
    /* ERROR_ID (1 byte) range 0 to 1                    */
	/* ERROR_ID value 0 no error occured                 */
	/* ERROR_ID value 1 when parameters are incorrect    */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);

	/* now read the binary data which is 1K in size */
	read_binary_data_from_pi(response_buffer, 1024);
	
	/* response is 1K of binary data (the size of each page specified in the CALLOC command) */
	
	/* when retrieving pages you have to request a whole page and can not specify part of a   */
	/* page even if its only part of a page containing the data - you must get the whole page */
	
	/* print the response as its a text file with null terminated string */
	printf("BinReceive: %s\r\n", response_buffer);

	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/*********************************************/
	/* lets change the binary data just received */
	/*********************************************/

	change_text = "echo \"We Changed This!\"";
	copy_message_into_buffer(response_buffer, change_text, 37);
	
	printf("Changed 1K page to: %s\r\n", response_buffer);
	
	/* we copied the change text into the memory buffer starting at the position 37 */
	
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();

	/*********************************************/
	/* Now lets write the memory page we have    */
	/* updated back to the pi                    */
	/*********************************************/

	/* format of request:                                  */
	/* SALLOC ALLOC_ID PAGE_NUMBER                         */
	/* SALLOC (1 byte) value 24                            */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* PAGE_NUMBER (2 bytes lo/high) value 0 to 9999       */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	
	printf("SAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 24;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	/* store page number (low/high byte) */
	output_buffer[2] = 0; /* page 0 */
	output_buffer[3] = 0;

	/* send the command (of 4 bytes) to the pi */
	send_bytes_to_pi(output_buffer, 4);

	/* when you send a page of data you must send the whole page of data back to the pi even if only modifying a small part of it */
	/* sending this page has no affect on the other pages in the memory allocation 1 on the pi                                    */
	
	/* send the binary data to the pi */
	send_binary_data_to_pi(response_buffer, 1024);
	
	/* format of response:                                 */
	/* SALLOC ALLOC_ID PAGE_NUMBER ERROR_ID                */
	/* SALLOC (1 byte) value 24                            */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be stored        */
	/* PAGE_NUMBER (2 bytes lo/high) value 0 to 9999       */
	/*          The page stored                            */
    /* ERROR_ID (1 byte) range 0 to 1                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */

	/* clear the buffer */
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	/* read the response from the pi */
	read_response_from_pi(response_buffer);
	
	/*********************************************/
	/* Now save the memory allocation on the pi  */
	/* to a file on the pi (with our changes)    */
	/*********************************************/
	

	/* format of request:                                       */
	/* PISALLOC ALLOC_ID FILESIZE /PATH/FILENAME\n              */
	/* PISALLOC (1 byte) value 28                               */
	/* ALLOC_ID (1 byte) range 1 to 20                          */
	/* FILE_SIZE (4 byte) range 0 to sizeof(int)                */
	/*   format high16bit(lo/high) low16bit(lo/high)            */
	/* /PATH/TO/FILENAME is a string filename path  with        */
	/* \n termination                                           */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
		
	printf("PISAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 28;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	/* file size to save format high16bit(lo/high) low16bit(lo/high) */
	output_buffer[2] = 0; 
	output_buffer[3] = 0;
	output_buffer[4] = 60; /* save 60 bytes */
	output_buffer[5] = 0;
	/* copy the path to file into the memory buffer */
    copy_message_into_buffer(&output_buffer[6], new_file_name, 0);

	/* send the command to the pi 6 + size of path/filename */
	send_bytes_to_pi(output_buffer, 6 + 39);
	
	/* format of response:                                      */
	/* PISALLOC ALLOC_ID ERROR_ID                               */
	/* PILALLOC (1 byte) value 28                               */
	/* ALLOC_ID (1 byte) range 0 to 20                          */
    /* ALLOC_ID value 0 means error occured                     */
    /* ALLOC_ID value 1 to 20 alloc_id saved from               */
    /* ERROR_ID (1 byte) range 0 to 2                           */
	/* ERROR_ID value 0 no error occured                        */
	/* ERROR_ID value 1 when parameters are incorrect           */
	/* ERROR_ID value 2 unable to save file                     */

	/* read the response from the pi */
	read_response_from_pi(response_buffer); 
	
	/*****************************************************************/
	/* As we have finished lets free the memory allocation on the pi */
	/*****************************************************************/

	/* format of request:                                */
	/* FALLOC ALLOC_ID                                   */
	/* FALLOC (1 byte) value 22                          */
	/* ALLOC_ID (1 byte) range 1 to 20                   */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
		
	printf("FAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 22;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;

	/* send the command to the pi */
	send_bytes_to_pi(output_buffer, 2);
	
	/* format of response:                               */
	/* FALLOC ALLOC_ID ERROR_ID                          */ 
	/* FALLOC (1 byte) value 22                          */
	/* ALLOC_ID (1 byte) range 0 to 20                   */
    /* ALLOC_ID value 0 means error occured              */
    /* ALLOC_ID value 1 to 20 alloc_id freed ok          */
    /* ERROR_ID (1 byte) range 0 to 1                    */
	/* ERROR_ID value 0 no error occured                 */
	/* ERROR_ID value 1 when parameters are incorrect    */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);     

	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();






	/***********************************/
	/* Create a memory allocation on   */
	/* the raspberry pi                */
	/***********************************/

	/* format of request:                                   */
	/* CALLOC NUM_PAGES SIZE_OF_PAGES                       */
	/* CALLOC (1 byte) value 21                             */
    /* NUM_PAGEs (2 bytes) range 1 to 9999                  */
    /* SIZE_OF_PAGES_IN_KB (1 byte) range 1 to 32           */
	/* example:                                             */
    /* 21 06 00 01                                          */
    /* where 06 + (00 << 8) is 6 pages of 8KB               */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("CAllOC \r\n");

	/* command id */
	output_buffer[0] = 21;
	/* number of pages lo/high */
	output_buffer[1] = 6;
	output_buffer[2] = 0;
	/* size of page to create (in KB) */
	output_buffer[3] = 1;

	/* send the command to the pi */
	send_bytes_to_pi(output_buffer, 4);

	/* format of response:                                  */
	/* CALLOC ALLOC_ID ERROR_ID                             */ 
    /* CALLOC 21 (1 byte)                                   */
    /* ALLOC_ID (1 byte) range 0 to 20                      */
    /* ALLOC_ID value 0 means error occured                 */
    /* ERROR_ID (1 byte) range 0 to 2                       */
	/* ERROR_ID value 0 no error occured                    */
	/* ERROR_ID value 1 when parameters are incorrect       */
	/* ERROR_ID value 2 when pi can't allocate memory       */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);

	/***********************************/
	/* Load in a file on the pi into   */
	/* the memory allocated            */
	/***********************************/
		
	/* format of request:                                  */
	/* PILALLOC ALLOC_ID /PATH/TO/FILENAME\n               */
	/* PILALLOC (1 byte) value 27                          */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* /PATH/TO/FILENAME is a string filename path  with   */
	/* \n termination                                      */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("PILAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 27;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	/* copy the path to file into the memory buffer */
    copy_message_into_buffer(&output_buffer[2], file_name, 0);

	/* send the command to the pi (30 is size of path and filename) */
	send_bytes_to_pi(output_buffer, 2 + 30);

	/* format of response:                                 */
	/* PILALLOC ALLOC_ID FILE_SIZE ERROR_ID                */
	/* PILALLOC (1 byte) value 27                          */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be modified      */
	/* FILE_SIZE (4 byte) range 0 to sizeof(int)           */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
    /* ERROR_ID (1 byte) range 0 to 3                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 unable to load file                */
	/* ERROR_ID value 3 file to large to fit allocation    */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);

	/**********************************************/
	/* Now lets insert memory into the allocation */
	/**********************************************/

	/* format of request:                                  */
	/* IALLOC ALLOC_ID INSERT_AT INSERT_LENGTH             */
	/* IALLOC (1 byte) value 25                            */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* INSERT_AT (4 byte) range 0 to sizeof(int)           */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */
	/* INSERT_LENGTH (4 byte) range 0 to sizeof(int)       */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("IAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 25;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	output_buffer[2] = 0;
	output_buffer[3] = 0;
	output_buffer[4] = 12;
	output_buffer[5] = 0;
	output_buffer[6] = 0;
	output_buffer[7] = 0;
	output_buffer[8] = 12;
	output_buffer[9] = 0;

	/* send the command (of 10 bytes) to the pi */
	send_bytes_to_pi(output_buffer, 10);

	/* send the binary data to the pi to insert into the allocation */
	send_binary_data_to_pi("Hello World\n", 12);
	
	/* format of response:                                 */
	/* IALLOC ALLOC_ID ERROR_ID                            */
	/* IALLOC (1 byte) value 25                            */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be modified      */
    /* ERROR_ID (1 byte) range 0 to 3                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 unable to allocate memory on pi    */
	/* ERROR_ID value 3 insert_at beyond end of allocation memory */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);
	
	/*********************************************/
	/* Now save the memory allocation on the pi  */
	/* to a file on the pi (with our changes)    */
	/*********************************************/
	

	/* format of request:                                       */
	/* PISALLOC ALLOC_ID FILESIZE /PATH/FILENAME\n              */
	/* PISALLOC (1 byte) value 28                               */
	/* ALLOC_ID (1 byte) range 1 to 20                          */
	/* FILE_SIZE (4 byte) range 0 to sizeof(int)                */
	/*   format high16bit(lo/high) low16bit(lo/high)            */
	/* /PATH/TO/FILENAME is a string filename path  with        */
	/* \n termination                                           */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
		
	printf("PISAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 28;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	/* file size to save format high16bit(lo/high) low16bit(lo/high) */
	output_buffer[2] = 0; 
	output_buffer[3] = 0;
	output_buffer[4] = 49; /* save 49 bytes */
	output_buffer[5] = 0;
	/* copy the path to file into the memory buffer */
    copy_message_into_buffer(&output_buffer[6], new_file_name2, 0);

	/* send the command to the pi 6 + size of path/filename */
	send_bytes_to_pi(output_buffer, 6 + 40);
	
	/* format of response:                                      */
	/* PISALLOC ALLOC_ID ERROR_ID                               */
	/* PILALLOC (1 byte) value 28                               */
	/* ALLOC_ID (1 byte) range 0 to 20                          */
    /* ALLOC_ID value 0 means error occured                     */
    /* ALLOC_ID value 1 to 20 alloc_id saved from               */
    /* ERROR_ID (1 byte) range 0 to 2                           */
	/* ERROR_ID value 0 no error occured                        */
	/* ERROR_ID value 1 when parameters are incorrect           */
	/* ERROR_ID value 2 unable to save file                     */

	/* read the response from the pi */
	read_response_from_pi(response_buffer); 
	
	/*****************************************************************/
	/* As we have finished lets free the memory allocation on the pi */
	/*****************************************************************/

	/* format of request:                                */
	/* FALLOC ALLOC_ID                                   */
	/* FALLOC (1 byte) value 22                          */
	/* ALLOC_ID (1 byte) range 1 to 20                   */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
		
	printf("FAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 22;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;

	/* send the command to the pi */
	send_bytes_to_pi(output_buffer, 2);
	
	/* format of response:                               */
	/* FALLOC ALLOC_ID ERROR_ID                          */ 
	/* FALLOC (1 byte) value 22                          */
	/* ALLOC_ID (1 byte) range 0 to 20                   */
    /* ALLOC_ID value 0 means error occured              */
    /* ALLOC_ID value 1 to 20 alloc_id freed ok          */
    /* ERROR_ID (1 byte) range 0 to 1                    */
	/* ERROR_ID value 0 no error occured                 */
	/* ERROR_ID value 1 when parameters are incorrect    */

	/* read the response from the pi */
	read_response_from_pi(response_buffer); 
    
	printf("\r\nPress any key to continue\r");
	printf("\r\n");
	printf("\r\n");	
	
	km_wait_char();




	/***********************************/
	/* Create a memory allocation on   */
	/* the raspberry pi                */
	/***********************************/

	/* format of request:                                   */
	/* CALLOC NUM_PAGES SIZE_OF_PAGES                       */
	/* CALLOC (1 byte) value 21                             */
    /* NUM_PAGEs (2 bytes) range 1 to 9999                  */
    /* SIZE_OF_PAGES_IN_KB (1 byte) range 1 to 32           */
	/* example:                                             */
    /* 21 06 00 01                                          */
    /* where 06 + (00 << 8) is 6 pages of 8KB               */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("CAllOC \r\n");

	/* command id */
	output_buffer[0] = 21;
	/* number of pages lo/high */
	output_buffer[1] = 6;
	output_buffer[2] = 0;
	/* size of page to create (in KB) */
	output_buffer[3] = 1;

	/* send the command to the pi */
	send_bytes_to_pi(output_buffer, 4);

	/* format of response:                                  */
	/* CALLOC ALLOC_ID ERROR_ID                             */ 
    /* CALLOC 21 (1 byte)                                   */
    /* ALLOC_ID (1 byte) range 0 to 20                      */
    /* ALLOC_ID value 0 means error occured                 */
    /* ERROR_ID (1 byte) range 0 to 2                       */
	/* ERROR_ID value 0 no error occured                    */
	/* ERROR_ID value 1 when parameters are incorrect       */
	/* ERROR_ID value 2 when pi can't allocate memory       */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);

	/***********************************/
	/* Load in a file on the pi into   */
	/* the memory allocated            */
	/***********************************/
		
	/* format of request:                                  */
	/* PILALLOC ALLOC_ID /PATH/TO/FILENAME\n               */
	/* PILALLOC (1 byte) value 27                          */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* /PATH/TO/FILENAME is a string filename path  with   */
	/* \n termination                                      */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("PILAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 27;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	/* copy the path to file into the memory buffer */
    copy_message_into_buffer(&output_buffer[2], file_name, 0);

	/* send the command to the pi (30 is size of path and filename) */
	send_bytes_to_pi(output_buffer, 2 + 30);

	/* format of response:                                 */
	/* PILALLOC ALLOC_ID FILE_SIZE ERROR_ID                */
	/* PILALLOC (1 byte) value 27                          */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be modified      */
	/* FILE_SIZE (4 byte) range 0 to sizeof(int)           */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
    /* ERROR_ID (1 byte) range 0 to 3                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 unable to load file                */
	/* ERROR_ID value 3 file to large to fit allocation    */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);

	/**********************************************************/
	/* Now lets cut out a section of memory in the allocation */
	/**********************************************************/

	/* format of request:                                  */
	/* XALLOC ALLOC_ID CUT_FROM CUT_LENGTH                 */
	/* XALLOC (1 byte) value 26                            */
	/* ALLOC_ID (1 byte) range 1 to 20                     */
	/* CUT_FROM (4 byte) range 0 to sizeof(int)            */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */
	/* CUT_LENGTH (4 byte) range 0 to sizeof(int)          */
	/*   format high16bit(lo/high) low16bit(lo/high)       */
	/*   Max size is the allocation size                   */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
	
	printf("XAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 26;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	output_buffer[2] = 0;
	output_buffer[3] = 0;
	output_buffer[4] = 7; /* from byte 7 of allocation */
	output_buffer[5] = 0;
	output_buffer[6] = 0;
	output_buffer[7] = 0;
	output_buffer[8] = 4; /* cut out 4 bytes (bash) */
	output_buffer[9] = 0;

	/* send the command (of 10 bytes) to the pi */
	send_bytes_to_pi(output_buffer, 10);
	
	/* format of response:                                 */
	/* XALLOC ALLOC_ID ERROR_ID                            */
	/* XALLOC (1 byte) value 26                            */
	/* ALLOC_ID (1 byte) range 0 to 20                     */
    /* ALLOC_ID value 0 means error occured                */
    /* ALLOC_ID value 1 to 20 alloc_id to be modified      */
    /* ERROR_ID (1 byte) range 0 to 2                      */
	/* ERROR_ID value 0 no error occured                   */
	/* ERROR_ID value 1 when parameters are incorrect      */
	/* ERROR_ID value 2 position+cut_length go beyond end  */
	/*         of allocation memory                        */

	/* read the response from the pi */
	read_response_from_pi(response_buffer);
	
	/*********************************************/
	/* Now save the memory allocation on the pi  */
	/* to a file on the pi (with our changes)    */
	/*********************************************/
	
	/* format of request:                                       */
	/* PISALLOC ALLOC_ID FILESIZE /PATH/FILENAME\n              */
	/* PISALLOC (1 byte) value 28                               */
	/* ALLOC_ID (1 byte) range 1 to 20                          */
	/* FILE_SIZE (4 byte) range 0 to sizeof(int)                */
	/*   format high16bit(lo/high) low16bit(lo/high)            */
	/* /PATH/TO/FILENAME is a string filename path  with        */
	/* \n termination                                           */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
		
	printf("PISAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 28;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;
	/* file size to save format high16bit(lo/high) low16bit(lo/high) */
	output_buffer[2] = 0; 
	output_buffer[3] = 0;
	output_buffer[4] = 45; /* save 45 bytes */
	output_buffer[5] = 0;
	/* copy the path to file into the memory buffer */
    copy_message_into_buffer(&output_buffer[6], new_file_name3, 0);

	/* send the command to the pi 6 + size of path/filename */
	send_bytes_to_pi(output_buffer, 6 + 40);
	
	/* format of response:                                      */
	/* PISALLOC ALLOC_ID ERROR_ID                               */
	/* PILALLOC (1 byte) value 28                               */
	/* ALLOC_ID (1 byte) range 0 to 20                          */
    /* ALLOC_ID value 0 means error occured                     */
    /* ALLOC_ID value 1 to 20 alloc_id saved from               */
    /* ERROR_ID (1 byte) range 0 to 2                           */
	/* ERROR_ID value 0 no error occured                        */
	/* ERROR_ID value 1 when parameters are incorrect           */
	/* ERROR_ID value 2 unable to save file                     */

	/* read the response from the pi */
	read_response_from_pi(response_buffer); 
	
	/*****************************************************************/
	/* As we have finished lets free the memory allocation on the pi */
	/*****************************************************************/

	/* format of request:                                */
	/* FALLOC ALLOC_ID                                   */
	/* FALLOC (1 byte) value 22                          */
	/* ALLOC_ID (1 byte) range 1 to 20                   */

	/* clear the buffer */
	clear_buffer(output_buffer, OUTPUT_BUFFER_LENGTH);
	clear_buffer(response_buffer, RESPONSE_BUFFER_LENGTH);
		
	printf("FAllOC \r\n");
	
	/* command id */
	output_buffer[0] = 22;
	/* use memory allocation 1 (created above) */
	output_buffer[1] = 1;

	/* send the command to the pi */
	send_bytes_to_pi(output_buffer, 2);
	
	/* format of response:                               */
	/* FALLOC ALLOC_ID ERROR_ID                          */ 
	/* FALLOC (1 byte) value 22                          */
	/* ALLOC_ID (1 byte) range 0 to 20                   */
    /* ALLOC_ID value 0 means error occured              */
    /* ALLOC_ID value 1 to 20 alloc_id freed ok          */
    /* ERROR_ID (1 byte) range 0 to 1                    */
	/* ERROR_ID value 0 no error occured                 */
	/* ERROR_ID value 1 when parameters are incorrect    */

	/* read the response from the pi */
	read_response_from_pi(response_buffer); 
    
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

void send_bytes_to_pi(uint8_t *data, uint8_t size_to_send)
{
	/*
	printf("Size %d\r\n", size_to_send);
	printf("Data 0 - %d\r\n", data[0]);
	printf("Data 1 - %d\r\n", data[1]);
	printf("Data 2 - %d\r\n", data[2]);
	printf("Data 3 - %d\r\n", data[3]);
    */

	printf("Sending: ");
	/* send the +++ delimiter */
	fifo_out_byte('+'); printf("%02d ", '+');
	fifo_out_byte('+'); printf("%02d ", '+');
	fifo_out_byte('+'); printf("%02d ", '+');
	
	/* send the size of the command - low byte (+3 is packet over head) */
	fifo_out_byte((size_to_send + 3) % 256);
	printf("%02d ", (size_to_send + 3) % 256);

	/* send the size of the command - high byte */
	fifo_out_byte((size_to_send + 3) / 256);
	printf("%02d ", (size_to_send + 3) / 256);
	
	/* send the BinaryComand type*/
	fifo_out_byte(2);
	printf("02 ");
	
	/* send the command to the pi */
	for(uint8_t index = 0; index < size_to_send; index++)
	{
		fifo_out_byte(data[index]);
		printf("%02d ", data[index]);
	}
	
	/* send the --- delimiter */
	fifo_out_byte('-');  printf("%02d ", '-');
	fifo_out_byte('-');  printf("%02d ", '-');
	fifo_out_byte('-');	 printf("%02d ", '-');

	printf("\r\r\n");
}

void read_response_from_pi(uint8_t *response)
{
	uint16_t index = 0;
	uint8_t packet_header[6];
	
	printf("Reading: ");

	/* read the first 6 bytes */
	while(index < 6)
	{
		packet_header[index] = get_byte_from_pi();
		printf("%02d ", packet_header[index]);
		index++;
	}
	
	/* if we got a packet header delimiter '+++' as expected and packet type is BinaryCommand (2) */
	if(packet_header[0] == '+' && packet_header[1] == '+' && packet_header[2] == '+' && packet_header[5] == 2)
	{
		uint16_t number_bytes = packet_header[3] + (packet_header[4] * 256) - 3; /* -3 as size includes packet header */
		
		index = 0;		
		/* read the message */
		while(index < number_bytes)
		{
			response[index] = get_byte_from_pi();
			printf("%02d ", response[index]);
			index++;
		}
		
		/* read the trailing delimiters '---' */
		index = 0;
		while(index < 3)
		{
			printf("%02d ", get_byte_from_pi());
			index++;
		}		

		printf("\r\r\n");
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

void send_binary_data_to_pi(const uint8_t *response_buffer, uint16_t number_bytes_to_write)
{
	uint16_t index = 0;
	uint16_t total_bytes_to_send = number_bytes_to_write + 3;
	
	printf("Sending BinaryData ... ");

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
		fifo_out_byte(response_buffer[ index ]);
	}
	
	/* send the --- delimiter */
	fifo_out_byte('-');
	fifo_out_byte('-');
	fifo_out_byte('-');

	printf("done\r\n");
}

void read_binary_data_from_pi(uint8_t *response_buffer, uint16_t number_bytes_to_read)
{
	uint16_t index = 0;
	uint8_t packet_header[6];
	
	printf("Reading Binary ... ");

	/* read the first 6 bytes */
	while(index < 6)
	{
		packet_header[index] = get_byte_from_pi();
		/* printf("%02d ", packet_header[index]); */
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
			response_buffer[index] = get_byte_from_pi();
			/* printf("%02d ", response_buffer[index]); */
			index++;
		}
		
		/* read the trailing delimiters '---' */
		index = 0;
		while(index < 3)
		{
			/* printf("%02d ", get_byte_from_pi()); */
			get_byte_from_pi();
			index++;
		}		

		printf("done.\r\r\n");
	}
	else
	{
		printf("Received malformed packet\r\n");
	}	
}

/* this function just copies the message into the response buffer provided           */
/* it makes no attempt to check the copy fits etc as this is a simple test program!! */
void copy_message_into_buffer(uint8_t *response_buffer, const uint8_t *change_text, uint16_t position_to_add)
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
		response_buffer[index + position_to_add] = change_text[index];
	}
	
	response_buffer[position_to_add + counter + 1] = 0;
}

void clear_buffer(uint8_t *response_buffer, uint16_t size_of_buffer)
{
	/* clear every byte in the buffer */
	for(uint16_t index = 0; index < size_of_buffer; index++)
	{
		response_buffer[index] = 0;
	}
}

