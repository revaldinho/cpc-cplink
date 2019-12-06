# To be run on Raspberry Pi
VPATH=../../../commonlib/rpi_c

default:	loopback

fifolib.o: fifolib.c fifolib.h
	(cd ${VPATH} ; make ) 

loopback: loopback.c fifolib.o 
	cc loopback.c ${VPATH}/fifolib.o -o loopback -I${VPATH}

clean:
	(cd ${VPATH} ; make clean)
	rm -f loopback *~ *.o >& /dev/null

