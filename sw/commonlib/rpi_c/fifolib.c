#include "fifolib.h"

int DATA[] = { PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7 };

void setup_pins() {
  int i;
#ifdef PI
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
#endif
}

void write_fifo_byte(int txdata) {
  int i, bit;
#ifdef PI
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
#endif
}

int read_fifo_byte() {
  int rval = 0;
  int timeout = 3;

  int i;
#ifdef PI
  for (i=7;i>=0;i--) {
    rval = (rval << 1) + (digitalRead(DATA[i]) & 0x1);
  }
  digitalWrite(PIN_SOB, HIGH);
  digitalWrite(PIN_SOB, LOW);
  // wait for DOR to go low immediately following -ve edge on SOB
  // but slower Pis might miss the event
  while (digitalRead(PIN_DOR) && !timeout) { timeout--; } ;
#endif
  return(rval);
}
