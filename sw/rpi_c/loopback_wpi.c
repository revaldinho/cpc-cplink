#include <wiringPi.h>
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

int DATA[] = { PIN_D0, PIN_D1, PIN_D2, PIN_D3, PIN_D4, PIN_D5, PIN_D6, PIN_D7 };

int queue[1024];
int qptr = 0;


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
  digitalWrite(PIN_WNR,1);
  for (i=0;i<8;i++) {
    bit = (txdata & 0x1)? HIGH: LOW;
    pinMode(DATA[i], OUTPUT);
    digitalWrite(DATA[i],bit);
    txdata = txdata >> 1;
  }
  digitalWrite(PIN_SI, HIGH);
  delayMicroseconds(60);
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
  delayMicroseconds(60);
  digitalWrite(PIN_SOB, LOW);
  return(rval);
}

void main( void ) {
  wiringPiSetupGpio () ;
  for (;;) {
    if (digitalRead(PIN_DOR)) {
      queue[qptr++] = read_fifo_byte();
    }
    if ( (qptr>0) && digitalRead(PIN_DIR)) {
      write_fifo_byte(queue[--qptr]);
    }
  }
}

