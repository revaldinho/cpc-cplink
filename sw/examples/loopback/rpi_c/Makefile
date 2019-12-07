# To be run on Raspberry Pi
#
# Use:
#      make [ USERDEFINES=  ] 
#

VPATH=../../../commonlib/rpi_c

USERDEFINES?=-DWIRINGPI=1

default:	loopback

fifolib.o: fifolib.c fifolib.h
	(cd ${VPATH} ; make USERDEFINES=${USERDEFINES} ) 

loopback: loopback.c fifolib.o 
	cc loopback.c ${VPATH}/fifolib.o -o loopback -I${VPATH} -lwiringPi

clean:
	(cd ${VPATH} ; make clean)
	rm -f loopback *~ *.o >& /dev/null

