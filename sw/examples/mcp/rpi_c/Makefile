# To be run on Raspberry Pi

CC = gcc
#CFLAGS = -Wall
CFLAGS =

default: mcp_pi

mcp_pi: mcp_pi.o command.o command_processor.o
	$(CC) $(CFLAGS) -o mcp_pi mcp_pi.o command.o command_processor.o -lwiringPi

mcp_pi.o:	mcp_pi.c mcp_pi.h
	$(CC) $(CFLAGS) -c mcp_pi.c

command.o:	command.c command.h
	$(CC) $(CFLAGS) -c command.c

command_processor.o:	command_processor.c command_processor.h
	$(CC) $(CFLAGS) -c command_processor.c

mcp_pc: mcp_pi.o dummy_wiringpi.o command.o command_processor.o
	$(CC) $(CFLAGS) -o mcp_pc mcp_pi.o dummy_wiringpi.o command.o command_processor.o

dummy_wiringpi.o:	dummy_wiringpi.c dummy_wiringpi.h
	$(CC) $(CFLAGS) -c dummy_wiringpi.c


clean:
	rm mcp_pi *~

