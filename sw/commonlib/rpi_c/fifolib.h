#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

// Comment next line out for trial compiles on system without wiringPi library
//#define PI 1

#ifdef PI
#include <wiringPi.h>
#define GET_DIR digitalRead(PIN_DIR)
#define GET_DOR digitalRead(PIN_DOR)
#else
#define GET_DIR 1
#define GET_DOR 1
#endif 

#ifndef PI1 
// BCM Pin numbering
#define PIN_D7   11
#define PIN_D6   10
#define PIN_D5    9
#define PIN_D4    8
#define PIN_D3    7
#define PIN_D2    4
#define PIN_D1    3
#define PIN_D0    2
#define PIN_SI   18
#define PIN_DIR  17
#define PIN_SOB  22
#define PIN_DOR  23
#define PIN_WNR  24
#else
// Wiring Pi pin numbering for older PI1
#define PIN_D7   11
#define PIN_D6   10
#define PIN_D5    9
#define PIN_D4    8
#define PIN_D3    7
#define PIN_D2    4
#define PIN_D1    1
#define PIN_D0    0
#define PIN_SI   18
#define PIN_DIR  17
#define PIN_SOB  22
#define PIN_DOR  23
#define PIN_WNR  24
#endif

extern void setup_pins() ;
extern void write_fifo_byte(int txdata) ; 
extern int read_fifo_byte() ;

