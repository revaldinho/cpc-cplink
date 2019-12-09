// main.c - date command demo for the CPC-CPLink board
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
#include <stdint.h>
#include <stdlib.h>
#include <firmware.h>
#include <fifolib.h>

#define RESPONSE_LENGTH 30

/* define functions below as main MUST be first function!! */
void send_command_to_pi(uint8_t *command);
void read_response_from_pi(uint8_t *response);
uint8_t get_byte_from_pi(void);

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
	uint8_t index = 0;
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
