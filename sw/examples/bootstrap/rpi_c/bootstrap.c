//
// Simple Bootstrap for Amstrad CPC
//
// (c) 2019 BigEd, Revaldinho
//
// One line bootstrap to run on CPC - 
//
//     FOR I=110 TO 170: POKE I,INP(&FD80): NEXT: CALL 126
//
#include <wiringPi.h>
#include <stdio.h>

// One liner assumes 45 bytes + 16 bytes flushing of FIFO = 61 bytes total
#define LOADER_LEN 45

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

int queue[QUEUE_SZ];
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
  int i;
  for (i=7;i>=0;i--) {
    rval = (rval << 1) + (digitalRead(DATA[i]) & 0x1);
  }
  digitalWrite(PIN_SOB, HIGH);
  digitalWrite(PIN_SOB, LOW);
  return(rval);
}

void fifo_send_byte( char c ) {
  // Blocking write to FIFO waiting on DIR 'til ready to send
  while ( !digitalRead(PIN_DIR) ){}
  write_fifo_byte( c ) ;
}

void main( void ) {
  setup_pins() ;

  unsigned char binary[20000];
  unsigned char loader[LOADER_LEN] ;  
  FILE *ptr;
  int i;

  puts("Reading boot loader from disk");  
  ptr = fopen( "bootloader.bin" ,"rb");  
  fread(loader,LOADER_LEN,1,ptr);
  fclose(ptr);

  #define BIN_LOAD_ADDR  0x4000
  #define BIN_ENTRY_ADDR 0x4025
  #define BIN_LEN        0x4380
  #define FILENAME       "DEF2_NO_HDR.bin"

  puts("Reading payload from disk");  
  ptr = fopen(FILENAME ,"rb");  
  fread(binary,sizeof(binary),1,ptr);
  fclose(ptr);

  // Pi starts up first with FIFO in unknown state
  puts("Flushing the input buffer");  
  while ( digitalRead(PIN_DOR) ) {
    read_fifo_byte();
  }
  puts("Topping up the output buffer");  
  while ( digitalRead(PIN_DIR) ) {
    write_fifo_byte(0xE5);
  }

  puts("Waiting to upload first boot loader");
  while ( !digitalRead(PIN_DIR)){}
  puts("Uploading first boot loader");
  for (i=0; i<LOADER_LEN; i++ ) {
    fifo_send_byte( loader[i] );
  }
  puts("Sending Header Data");
  // length of loader (litte endian)
  // load address (little endian)
  // loader
  // run address (little endian)  
  char header [] = {
    BIN_LEN & 0xFF,
    (BIN_LEN >> 8 ) & 0xFF,    
    BIN_LOAD_ADDR & 0xFF,
    (BIN_LOAD_ADDR >> 8 ) & 0xFF,
  };

  for (i=0; i<4; i++) {
    fifo_send_byte( header[i] );
  }

  puts("Sending the final payload");
  for (i=0; i<BIN_LEN; i++ ) {
    fifo_send_byte( binary[i] );
  }

  puts("Sending the Entry Address");
  fifo_send_byte( (char) BIN_ENTRY_ADDR & 0xFF );
  fifo_send_byte( (char) (BIN_ENTRY_ADDR>>8) & 0xFF );  
}


