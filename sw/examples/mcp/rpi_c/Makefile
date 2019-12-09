# To be run on Raspberry Pi

CC = gcc
CFLAGS = -Wall -O2

VPATH=../../../commonlib/rpi_c

USERDEFINES?=-DWIRINGPI=1

default:	mcp_pi

fifolib.o: fifolib.c fifolib.h
	(cd ${VPATH} ; make USERDEFINES=${USERDEFINES} ) 

mcp_pi.o:	mcp_pi.c mcp_pi.h
	$(CC) $(CFLAGS) -c mcp_pi.c -I${VPATH} ${USERDEFINES}

command.o:	command.c command.h
	$(CC) $(CFLAGS) -c command.c -I${VPATH} ${USERDEFINES}

command_processor.o:	command_processor.c command_processor.h
	$(CC) $(CFLAGS) -c command_processor.c -I${VPATH} ${USERDEFINES}

page_manager.o:	memstore_manager.c memstore_manager.h
	$(CC) $(CFLAGS) -c memstore_manager.c -I${VPATH} ${USERDEFINES}

queue.o:	queue.c queue.h
	$(CC) $(CFLAGS) -c queue.c -I${VPATH} ${USERDEFINES}

packet.o:	packet.c packet.h
	$(CC) $(CFLAGS) -c packet.c -I${VPATH} ${USERDEFINES}

mcp_pi:	mcp_pi.o command.o command_processor.o memstore_manager.o queue.o packet.o fifolib.o 
	$(CC) $(CFLAGS) -o mcp_pi ${VPATH}/fifolib.o mcp_pi.o command.o command_processor.o memstore_manager.o queue.o packet.o -I${VPATH} ${USERDEFINES} -lwiringPi
	cp mcp_pi ~/mcp_pi
	rm *.o

mcp_pc:	mcp_pi.o command.o command_processor.o memstore_manager.o queue.o packet.o fifolib.o 
	$(CC) $(CFLAGS) -o mcp_pc ${VPATH}/fifolib.o mcp_pi.o command.o command_processor.o memstore_manager.o queue.o packet.o -I${VPATH} ${USERDEFINES}
	rm *.o
	(cd ${VPATH} ; rm *.o)

clean:
	rm mcp_pi


