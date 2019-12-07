# use null setting for USERDEFINES for trial compile on system without wiringPi 
#   make [ USERDEFINES= ] 

USERDEFINES?=-DWIRINGPI=1

fifolib.o : fifolib.c fifolib.h
	cc -c fifolib.c -o fifolib.o ${USERDEFINES} -lwiringPi

clean:
	rm -f *.o *~ >& /dev/null
