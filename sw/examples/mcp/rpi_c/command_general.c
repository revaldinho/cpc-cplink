/********************************/
/*                              */
/* General Command Functions     */
/*                              */
/********************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "command_general.h"
#include "packet.h"
#include "memstore_manager.h"
#include "command_processor.h"

/***************************************************************/
/*                                                             */
/* Built in Simple Commands                                    */
/*                                                             */
/***************************************************************/


/* unknown command - tick is ignored as all output in one call */
BOOL command_unknown(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "Unknown Command\n";

	/* write response to cpc */
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function complete */
	return( TRUE );
}

/* ping command - tick is ignored as all output in one call */
BOOL command_ping(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "PONG!\n";

	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function completed */
	return( TRUE );
}

/* this command operates over several ticks - first sending the message to the cpc     */
/* second tick actually running the shutdown command so message has time to get to cpc */
BOOL command_shutdown(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "SHUTDOWN IN 10 SECONDS\n";

	/* if we are at the first tick (tick=0) */
	if(tick == 0)
	{
		/* write the OK response to the out_queue for sending to client */
		write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	}
	else
	{
		#ifdef BUILD_ON_PC
			printf("Running Shutdown(Fake): %d\n", system("/bin/pwd"));
		#else
			printf("Running Shutdown: %d\n", system("sudo /sbin/shutdown -h now"));
		#endif
		
		return( TRUE );
	}
	
	/* return FALSE as command is not complete and we need to do */
	/* the shutdown in the next tick                             */
	return( FALSE );
}

/* time command - tick is ignored as all output in one call */
BOOL command_time(Queue *in_queue, Queue *out_queue, int tick)
{
	char current_time[40];
	
	time_t rawtime;
	struct tm * timeinfo;
	
	memset(current_time, 0, 40);
	
	time( &rawtime );
	timeinfo = localtime( &rawtime );
	
	/* format the time response */
	sprintf(current_time, "%02d:%02d:%02d\n", timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
	
	/* write the current_time response to the out_queue for sending to client */
	write_data_to_packet(out_queue, (BYTE *) current_time, strlen(current_time), TextCommand);
	
	return( TRUE );
}

/* date command - tick is ignored as all output in one call */
BOOL command_date(Queue *in_queue, Queue *out_queue, int tick)
{
	char current_date[40];
	
	time_t rawtime;
	struct tm * timeinfo;
	
	memset(current_date, 0, 40);
	
	time( &rawtime );
	timeinfo = localtime( &rawtime );

	/* format the date response */
	sprintf(current_date, "%02d:%02d:%04d\n", timeinfo->tm_mday, timeinfo->tm_mon+1, timeinfo->tm_year+1900);
	
	/* write the current_date to the out_queue for sending to client */
	write_data_to_packet(out_queue, (BYTE *) current_date, strlen(current_date), TextCommand);
	
	return( TRUE );
}

/* reset command - tick is ignored as all output in one call */
BOOL command_reset(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "OK\n";

	if(TRON)
	{
    	printf("Reset Queues, MemStore and Command Processor\n");
	}
	
	/* reset the queues */
	queues_reset(in_queue, out_queue);
	/* reset the memstore */
	memstore_reset();		
	/* reset the command processor */
	command_processor_init();
	
	/* write the OK response to the out_queue for sending to client */
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	return( TRUE );
}

/* help command - tick is ignored as all output in one call */
BOOL command_help(Queue *in_queue, Queue *out_queue, int tick)
{
	char command_name_text[MAX_COMMAND_NAME_LENGTH];

	BOOL is_command_complete = FALSE;

	#ifdef DEBUG_MCP_IF
		/* if debugging then clear out_queue so we can read output of command clearly */
		init_queue(out_queue);
	#endif
	
	/* if previous byte read was \n then no parameter so just do HELP help text */
	if(get_previous_read_byte(in_queue) == '\n' )
	{
		is_command_complete = command_defs[Help].help_function(in_queue, out_queue);
	}
	else if(get_command_next_param_text(in_queue, command_name_text, MAX_COMMAND_NAME_LENGTH, TRUE))
	{
		/* read the parameter (the command name you want help on) */
		Command commandID = get_commandID_from_string( command_name_text );
	
		/* if the command found was unknown then get the HELP help text */
		if(commandID == Unknown)
		{
			is_command_complete = command_defs[Help].help_function(in_queue, out_queue);
		}
		else /* get the help for the identified command */
		{
			is_command_complete = command_defs[commandID].help_function(in_queue, out_queue);		
		}
	}
	else
	{
		/* failed to read the parameter with the help command (the command name that you want help on) */
		/* so lets return help on the HELP command */
		is_command_complete = command_defs[Help].help_function(in_queue, out_queue);
	}
	
	return(is_command_complete);
}

/* this command operates over several ticks - first sending the message to the cpc     */
/* second tick actually running the reboot command so message has time to get to cpc */
BOOL command_reboot(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "Rebooting PI IN 3 SECONDS\n";

	/* if we are at the first tick (tick=0) */
	if(tick == 0)
	{
		/* write the OK response to the out_queue for sending to client */
		write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	}
	else
	{
		#ifdef BUILD_ON_PC
			printf("Running Reboot(Fake): %d\n", system("/bin/pwd"));
		#else
			printf("Running Reboot: %d\n", system("sudo /sbin/reboot"));
		#endif
		
		return( TRUE );
	}
	
	/* return FALSE as command is not complete and we need to do */
	/* the shutdown in the next tick                             */
	
	return( FALSE );
}

/* tron command - tick is ignored as all output in one call */
BOOL command_tron(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "OK\n";

	TRON = TRUE;
	
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function completed */
	return( TRUE );
}

/* troff command - tick is ignored as all output in one call */
BOOL command_troff(Queue *in_queue, Queue *out_queue, int tick)
{
	char *response = "OK\n";

	TRON = FALSE;
	
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function completed */
	return( TRUE );
}

/* version command - tick is ignored as all output in one call */
BOOL command_version(Queue *in_queue, Queue *out_queue, int tick)
{
	char response[18];

	memset(response, 0, 18);
	
	sprintf(response, "MCP v%s\n", VERSION);
	
	write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);
	
	/* function completed */
	return( TRUE );
}

/* wifi command - tick is ignored as all output in one call */
BOOL command_wifi_ip(Queue *in_queue, Queue *out_queue, int tick)
{
	int shell_return = 0;

	char buffer[MAX_WIFI_LINE_LENGTH * 2];

	/* run checks on WIFI_DEVICE device id */

	/* clear down the buffer */
	memset(buffer, 0, MAX_WIFI_LINE_LENGTH * 2);

	/*int return_value1 = system("iwconfig 2>&1 | grep ESSID > /tmp/wifi.tmp"); */
	
	sprintf(buffer, "(ifconfig %s | awk \'/inet /{print substr($2,0)}\') > /tmp/wifi.tmp 2> /tmp/wifi.tmp", WIFI_DEVICE);

	shell_return = system(buffer);
	
	/* if we got a positive value */
	if(shell_return)
	{
		char *response = "WIFI Unable to execute command.\n";

		/* we failed to execute command - so return an error */
		write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);	
	}
	else /* shell command executed ok */
	{
	    FILE *file = fopen("/tmp/wifi.tmp", "r");

		/* read the tmp file for the output of the shell command */

		/* if we failed to open the tmp file */
		if (file == NULL)
		{
			char *response = "WIFI Unable to retrieve result.\n";

			/* we failed to execute command - so return an error */
			write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);	
		}
		else /* we openend the file to read */
		{
			/* if we failed to seek to the end of the file */
			if(fseek(file, 0, SEEK_END) != 0)
			{
				char *response = "WIFI Unable to retrieve result.\n";

				/* we failed to execute command - so return an error */
				write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);	
			}
			else /* we found the end of the file */
			{
				/* read the file length */
				int file_size = ftell(file);
				int line_index = 0;

				char ch = 0;
				char file_buffer[MAX_WIFI_LINE_LENGTH];

				memset(file_buffer, 0, MAX_WIFI_LINE_LENGTH);

				/* see to the begining of the file so we can read it */
				fseek(file, 0, SEEK_SET);

				for(int index = 0; index < file_size; index++)
				{
					/* read a character from the file */
					ch = fgetc(file);

					/* if its a \n then ignore it */
					if(ch != '\n')
					{
						file_buffer[line_index] = ch;
						line_index++;
					}

				} /* for */
				
				/* terminate the string in the result buffer */
				file_buffer[line_index] = 0;

				if(TRON) {printf("WIFI IP %s", file_buffer);}

				/* clear down the buffer */
				memset(buffer, 0, MAX_WIFI_LINE_LENGTH * 2);

				/* build message to return to client */
				sprintf(buffer, "WIFI IP %s\n", file_buffer);

				/* write the result to the client */
				write_data_to_packet(out_queue, (BYTE *) buffer, strlen(buffer), TextCommand);	
			}

			/* close the file */
			fclose(file);

		} /* we openend the file to read */

	} /* shell command executed ok */
		
	return( TRUE );
}

/* termcomm command - tick is ignored as all output in one call */
BOOL command_shellexec(Queue *in_queue, Queue *out_queue, int tick)
{
	int shell_return = 0;

	char buffer[MAX_SHELL_LINE_LENGTH];
	char shell_command[MAX_SHELL_LINE_LENGTH * 2];

	char *output_directive = "> /tmp/shellexec.tmp 2> /tmp/shellexec.tmp";

	/* clear down the buffers */
	memset(buffer, 0, MAX_SHELL_LINE_LENGTH);
	memset(shell_command, 0, MAX_SHELL_LINE_LENGTH * 2);
	
	/* if we failed to read the shell command from the params of the ShellExec command */
	if(get_all_command_params_text(in_queue, buffer, MAX_SHELL_LINE_LENGTH) == FALSE)
	{
		char *response = "SHELLEXEC No shell command specified to run.\n";

		/* we failed to get shell command to execute - so return an error */
		write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);	
	}
	else /* read the shell command from ShellExec parameters */
	{
		/* build the shell execution string */
		sprintf(shell_command, "%s %s", buffer, output_directive);

		if(TRON) {printf("Executing Shell Command: %s\n", shell_command);}

		/* execute the command */
		shell_return = system(shell_command);
		
		/* if we got a positive value */
		if(shell_return)
		{
			char *response = "SHELLEXEC Unable to execute command.\n";

			/* we failed to execute command - so return an error */
			write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);	
		}
		else /* shell command executed ok */
		{
			FILE *file = fopen("/tmp/shellexec.tmp", "r");

			/* read the tmp file for the output of the shell command */

			/* if we failed to open the tmp file */
			if (file == NULL)
			{
				char *response = "SHELLEXEC Unable to retrieve result.\n";

				/* we failed to execute command - so return an error */
				write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);	
			}
			else /* we opened the tmp file to read */
			{
				/* seek to the end of the file*/

				/* if we failed to seek to the end of the file */
				if(fseek(file, 0, SEEK_END) != 0)
				{
					char *response = "SHELLEXEC Unable to retrieve result.\n";

					/* we failed to execute command - so return an error */
					write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);	
				}
				else /* we found the end of the file */
				{
					char *result_header_text = "SHELLEXEC Shell Result\r\r";

					/* read the file length */
					int file_size = ftell(file);

					/* if the result file is over half the queue size */
					if(file_size > (QUEUE_SIZE >> 2))
					{
						/* return an error - we can't return such a big result */
						char *response = "SHELLEXEC Result to large to return.\n";

						/* we failed to execute command - so return an error */
						write_data_to_packet(out_queue, (BYTE *) response, strlen(response), TextCommand);	
					}
					/* is there space in the out_queue for the output  (+1 for terminating \n) */
					else if(is_queue_space_available(out_queue, file_size + strlen(result_header_text) + 1) == FALSE)
					{
						/* we don't have space in the out_queue for the result so return FALSE and try again later */
						return( FALSE );					
					}
					else /* there is space in the out_queue for the result packet */
					{				
						char ch = 0;
						int line_index = 0;

						char result_buffer[MAX_SHELL_LINE_LENGTH];

						/* clear the result_buffer */
						memset(result_buffer, 0, MAX_SHELL_LINE_LENGTH);

						/* see to the begining of the file so we can read it */
						fseek(file, 0, SEEK_SET);

						/* start to build the packet for output (+1 for termianting \n) */
						start_build_data_packet(out_queue, file_size + strlen(result_header_text) + 1, TextCommand);

						/* output the start of the response */
						add_data_to_packet(out_queue, (BYTE *) result_header_text, strlen(result_header_text));

						/* read from the file a line at a time to EOF */
						for(int index = 0; index < file_size; index++)
						{
							/* read a character from the file */
							ch = fgetc(file);

							/* if we find a newline */
							if((ch == '\n') || (ch == '\r'))
							{
								/* replace with a \r for the cpc */
								result_buffer[line_index] = '\r';
								/* terminate the string */
								result_buffer[line_index + 1] = 0;
								line_index = 0;

								/* send the line to the output */
								add_data_to_packet(out_queue, (BYTE *) result_buffer, strlen(result_buffer));

								/* clear the result_buffer */
								memset(result_buffer, 0, MAX_SHELL_LINE_LENGTH);
							}
							else /* didn't find a new line */
							{
								/* copy the charactert into the result_buffer */
								result_buffer[line_index] = ch;
								line_index++;
							}		

						} /* for */

						/* if we have a line with no terminating \n left */			
						if(line_index > 0)
						{
							result_buffer[line_index] = 0;
							line_index = 0;

							/* send the line to the output */
							add_data_to_packet(out_queue, (BYTE *) result_buffer, strlen(result_buffer));
						}

						/* add the final \n to ensure its readable by the client */
						add_data_to_packet(out_queue, (BYTE *) "\n", 1);

						/* finish the packet build and send the packet out */
						finish_build_data_packet_and_send(out_queue);

					} /* there is space in the out_queue for the result packet */

				} /* we found the end of the file */

				/* close the file */
				fclose(file);

			} /* we opened the tmp file to read */

		} /* shell command executed ok */

	} /* read the shell command from ShellExec parameters */
			
	return( TRUE );
}

/********************************************************************/
/*                                                                  */
/* Help Functions for the command services                          */
/*                                                                  */
/********************************************************************/

/* simple built in commands help for basic services */
BOOL command_unknown_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rAn UNKNOWN command was entered\r\rTry entering HELP\\n.\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_ping_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rPING\r\rFunction: Used to test pi is responding to CPC commands.\r\rFormat: PING\\n\r\rReturn: PONG!\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}
	
BOOL command_shutdown_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rSHUTDOWN\r\rFunction: Command to shutdown the raspberry pi before powering off CPC.\rYou need to wait the number of seconds specified then check the pi is inactive    i.e. red led only on.\r\rFormat: SHUTDOWN\\n\r\rReturn: SHUTDOWN in x seconds\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_time_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rTIME\r\rFunction: Returns pi current time.  Will need time on pi to be set e.g. via ntp.\r\rFormat: TIME\\n\r\rReturn: HH:MM:SS\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
		
	/* command comepleted */
	return(TRUE);
}

BOOL command_date_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rDATE\r\rFunction: Returns pi current date.  Will need date on pi to be set e.g. via ntp.\r\rFormat: DATE\\n\r\rReturn: DD:MM:YYYY\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_reset_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response_text[] = 
		{
			"\rRESET\r\r",
			"Function: Reset MCP data structures, flushes queues, closes files, frees        allocated memory etc.\r\rUse this to bring the MCP back to a clean known state.\r\r",
			"Format: RESET\\n\r\r",
			"Return: OK\\n\r\n",
			NULL 
		};
	
	/* write response to out_queue */
	if(write_out_help_text_array(out_queue, response_text))
	{
		function_complete = TRUE;
	}
	   
	/* was command completed */
	return(function_complete);
}

BOOL command_help_help(Queue *in_queue, Queue *out_queue)
{
	BOOL function_complete = FALSE;
	
	char *response1_text = "\rHELP\r\r";
	char *response2_text = "Function: Provides help on the COMMAND_NAME provided.\r\r";
	char *response3_text = "Help is available for the following commands:\r\r";
	char *response4_text = "\r\rFormat: HELP COMMAND_NAME\\n\r\r";
	char *response5_text = "Return: String of text terminated with \\n\r\n";
	
	int command_text_size = 0;

	/* get the command string text so we can determine the size */
	for(int index = 1; index < MaxCommands; index++ )
	{
		/* +1 for space after the command text */
		command_text_size = command_text_size + strlen(command_defs[index].command_text) + 1;
	}

	command_text_size = command_text_size + 
	                    strlen(response1_text) + 
						strlen(response2_text) + 
						strlen(response3_text) + 
						strlen(response4_text) + 
						strlen(response5_text) +
						(MaxCommands - 1) / 7; /* to allow for \r for each line of 7 commands below */
	
	/* ensure space in out_queue */
	if(start_build_data_packet(out_queue, command_text_size, TextCommand))
	{
		add_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text));
		add_data_to_packet(out_queue, (BYTE *) response2_text, strlen(response2_text));
		add_data_to_packet(out_queue, (BYTE *) response3_text, strlen(response3_text));
		
		for(int index = 1; index < MaxCommands; index++ )
		{
			add_data_to_packet(out_queue, (BYTE *) command_defs[index].command_text, strlen(command_defs[index].command_text));
			add_data_to_packet(out_queue, (BYTE *) " ", 1);

			/* if we have output 7 commands */
			if((index % 7) == 0)
			{
				/* start a new line */
				add_data_to_packet(out_queue, (BYTE *) "\r", 1);				
			}
		}

		add_data_to_packet(out_queue, (BYTE *) response4_text, strlen(response4_text));
		add_data_to_packet(out_queue, (BYTE *) response5_text, strlen(response5_text));
		finish_build_data_packet_and_send(out_queue);
		
		function_complete = TRUE;
	}
	
	/* was command completed */
	return(function_complete);
}

BOOL command_reboot_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rREBOOT\r\rFunction: Command to reboot the raspberry pi.\rYou need to wait the number of seconds before the reboot starts.\r\rFormat: REBOOT\\n\r\rReturn: REBOOT PI in x seconds\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_tron_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rTRON\r\rFunction: Turns on debugging information sent to pi console.\r\rFormat: TRON\\n\r\rReturn: OK\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_troff_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rTROFF\r\rFunction: Turns off debugging information sent to pi console.\r\rFormat: TROFF\\n\r\rReturn: OK\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_version_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rVERSION\r\rFunction: Returns MCP current build version.\r\rFormat: VERSION\\n\r\rReturn: MCP v0.3\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_wifi_ip_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rWIFIIP\r\rFunction: Returns the ip address of the pi's wifi.\r\rFormat: WIFIIP\\n\r\rReturn: WIFI IP_ADDRESS\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}

BOOL command_shellexec_help(Queue *in_queue, Queue *out_queue)
{
	char *response1_text = "\rSHELLEXEC\r\rFunction: Calls a command as if run from a shell and returns the shell output.\r\rFormat: SHELLEXEC SHELL_COMMAND\\n\r\rReturn: Text from the shell\\n\r\n";
	
	/* write response to out_queue */
	write_data_to_packet(out_queue, (BYTE *) response1_text, strlen(response1_text), TextCommand);
	
	/* command comepleted */
	return(TRUE);
}
