/* only used for testing compilation process on PC */

#ifndef DUMMY_WIRINGPI_H
#define DUMMY_WIRINGPI_H 1

#define INPUT 0
#define OUTPUT 0

#define LOW 0
#define HIGH 1
	
void wiringPiSetupGpio(void);

void pinMode(unsigned char pin, unsigned char direction);

void digitalWrite(unsigned char pin, unsigned char bit_to_write);

unsigned char digitalRead(unsigned char pin);

#endif