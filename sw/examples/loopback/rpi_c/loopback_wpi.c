#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

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


#define QUEUE_SZ  8192
int DATA[] = { PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7 };

char queue [QUEUE_SZ];
int rptr = 0;
int wptr = 0;

void setup_pins() {
  int i;
  wiringPiSetupGpio();
  for (i=0;i<8;i++){
    pinMode(DATA[i],INPUT);
  }
  pinMode(PIN_DIR, INPUT);
  pinMode(PIN_DOR, INPUT);
  pinMode(PIN_SI, OUTPUT);
  digitalWrite(PIN_SI,LOW);
  pinMode(PIN_SOB, OUTPUT);
  digitalWrite(PIN_SOB,LOW);
  pinMode(PIN_WNR, OUTPUT);
  digitalWrite(PIN_WNR,LOW);
}

void write_fifo_byte(int txdata) {
  int i, bit;
  digitalWrite(PIN_WNR,HIGH);
  for (i=0;i<8;i++) {
    bit = (txdata & 0x1)? HIGH: LOW;
    pinMode(DATA[i], OUTPUT);
    digitalWrite(DATA[i],bit);
    txdata = txdata >> 1;
  }  
  digitalWrite(PIN_SI, HIGH);  
  digitalWrite(PIN_SI, LOW);
  for (i=0;i<8;i++) {
    pinMode(DATA[i], INPUT);
  }
  digitalWrite(PIN_WNR, LOW);
}

int read_fifo_byte() {
  int rval = 0;
  int timeout = 3;

  int i;
  for (i=7;i>=0;i--) {
    rval = (rval << 1) + (digitalRead(DATA[i]) & 0x1);
  }
  digitalWrite(PIN_SOB, HIGH);
  digitalWrite(PIN_SOB, LOW);
  // wait for DOR to go low immediately following -ve edge on SOB
  // but slower Pis might miss the event
  while (digitalRead(PIN_DOR) && !timeout) { timeout--; } ;
  return(rval);
}

void hexdump( char data[], int dlen, int grpsz ) {
  int i, max;
  char *astr = (char * ) malloc ( grpsz + 1);

  memset( astr, 0, grpsz+1);
  max = dlen + (dlen%grpsz > 0)?dlen + grpsz - dlen%grpsz: 0;
  
  for (i=0; i< max ; i++ ) {
    int j = i%grpsz;
    if (j == 0) {
      if (i>0) puts(astr);
      printf( "%04X: ", i);
    }
    
    
    if (i>=dlen) {
      printf( "xx ");
      astr[j]= '-';
    } else {
      printf( "%02X ", data[i] ) ;
      astr[j]= ( data[i]>31 && data[i]<128) ? data[i]: '.';
    }
  }
  if (i%grpsz == 0) printf("%s\n", astr);
}

void main( int argc, char *argv[] ) {
  
  int first = 1;
  int f_hexl= 0;
  int c;

  while ( (c=getopt( argc, argv, "x")) != -1) {
    switch(c) {
    case 'x':
      f_hexl = 1;
      break;
    default:
      abort();
    }
  }

  setup_pins() ;

  for (;;) {
    if (digitalRead(PIN_DOR)) {
      queue[rptr] = read_fifo_byte();
      rptr = (rptr+1) % QUEUE_SZ;
    }
    if ( (wptr!=rptr) && digitalRead(PIN_DIR)) {

      // On first FIFO output, dump the data in the queue to screen
      if ( first && f_hexl) {
          first = 0;
          hexdump( queue, rptr, 16 ) ;
      }

      write_fifo_byte(queue[wptr]);
      wptr = (wptr+1) % QUEUE_SZ;
    }
  }
}
