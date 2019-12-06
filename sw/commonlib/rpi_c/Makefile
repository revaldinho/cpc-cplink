# use null setting for USERDEFINES for trial compile on system without wiringPi 
#   make [ USERDEFINES= ] 

USERDEFINES?=-DWIRINGPI=1

fifolib.o : fifolib.c fifolib.h
	cc -c fifolib.c -O2 -o fifolib.o ${USERDEFINES} 

clean:
	rm -f *.o *~ >& /dev/null
