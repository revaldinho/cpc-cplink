// main.c - date command demo for the CPC-CPLink board
//
// Copyright (C) 2019  Revaldinho
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
#include <stdint.h>
#include <stdlib.h>
#include <firmware.h>
#include <fifolib.h>

#define RESPONSE_LENGTH 30

/* define functions below as main MUST be first function!! */
void send_command_to_pi(uint8_t *command);
void read_response_from_pi(uint8_t *response);

void main ( void ) 
{
	uint8_t *command = "DATE\n";
	
	uint8_t response[RESPONSE_LENGTH];
	
	/* clear the response memory where we will build up the string response */
	for(uint8_t index = 0; index < RESPONSE_LENGTH; index++)
	{
		response[index] = 0;
	}
	
	/* set screen mode 2 */
	scr_set_mode(2);

	/* reset the fifo queue */
	fifo_reset();

	/* send the command to the pi */
	send_command_to_pi(command);
	
	/* print what we sent */
    printf("\r%s\r", command);
	
	/* read the response from the pi */
	read_response_from_pi(response);
	
	/* print the response string */
	printf("%s\r\r", response);
	
	printf("Press any key to exit\r");
	km_wait_char();
}

void send_command_to_pi(uint8_t *command)
{
	uint8_t counter = 0;
	
	/* count to the end of the null terminated string */
	while(command[counter] != 0)
	{
		counter++;
	}
	
	/* send the command to the pi */
	for(uint8_t index = 0; index < counter; index++)
	{
		fifo_out_byte(command[ index ]);
	}
}

void read_response_from_pi(uint8_t *response)
{
	uint8_t receive_byte_index = 0;
	uint8_t did_we_get_byte = 0;
	
	while((receive_byte_index == 0) || ((receive_byte_index > 0) && (response[receive_byte_index - 1] != '\n')))
	{
		while(did_we_get_byte == 0)
		{
			/* didn't get one so try again */
			did_we_get_byte = fifo_in_byte(&response[receive_byte_index]);
			
		} /* while */

		/* reset for potential next loop */
		did_we_get_byte = 0;

		/* increment the index into the response ready for next receive byte */
		receive_byte_index++;
		
	} /* while */	
}

