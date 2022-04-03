#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <fifolib.h>
#include <firmware.h>

#define MOVE(x,y) plot(4,x,y)
#define DRAW(x,y) plot(5,x,y)
#define VDU18(a,b) fifo_put_byte(18) ; fifo_put_byte(a) ; fifo_put_byte(b)
#define NUMLINES 8
#define ITERATIONS 50
#define MX 3200
#define MY 2400

void vduinit();
void vdustring( char *s );
void plot(uint8_t type, int16_t x, int16_t y) ;
void delay_300th(uint32_t d);

void main (int argc, char *argv[] ) {
  int16_t X=MX>>1;
  int16_t Y=MY>>1;
  int16_t A=2*MX/3;
  int16_t B=2*MY/3;
  int16_t V=32;
  int16_t W=24;
  int16_t C=16;
  int16_t D=-28;
  int16_t E[NUMLINES+1];
  int16_t F[NUMLINES+1];
  int16_t G[NUMLINES+1];
  int16_t H[NUMLINES+1];
  char strbuf[128];
  int16_t  k;
  int8_t   i;
  uint32_t t;

  printf("Walking Lines Demo running on Pi GPU\r\n");

  vduinit();
  // Dummy initialisation for very first pass
  E[1]=0 ; F[1]=0 ; G[1]=0; H[1]=0;

  kl_time_set(0);

  for ( k=0 ; k< ITERATIONS ; k++ ) {
    for ( i=1 ; i < (NUMLINES+1) ; i++) {
      VDU18(0,0);
      MOVE(E[i],F[i]);
      DRAW(G[i],H[i]);
      E[i]=X;
      F[i]=Y;
      G[i]=A;
      H[i]=B;
      VDU18(0,i);
      MOVE(X,Y);
      DRAW(A,B);
      X=X+V ;
      Y=Y+W ;
      if (X>MX || X<0) V=-V;
      if (Y>MY || Y<0) W=-W;
      A=A+C;
      B=B+D;
      if (A>MX || A<0) C=-C;
      if (B>MY || B<0) D=-D;
    }
    delay_300th(30);
  }

  t = kl_time_please();

  sprintf(strbuf,"Num Lines: %d\r\n", NUMLINES); vdustring(strbuf);
  sprintf(strbuf,"Iterations: %d\r\n", ITERATIONS); vdustring(strbuf);
  sprintf(strbuf,"Coordinate Area: %d x %d\r\n", MX, MY); vdustring(strbuf);
  sprintf(strbuf,"Time: %d.%02d s\r\n", (uint16_t)(t/300), (uint16_t)(t%300)/3); vdustring(strbuf);
  printf("Press any key to exit\r");
  sprintf(strbuf,"Press any key to exit\r"); vdustring(strbuf);
  km_wait_char();
}


void vduinit() {
  int8_t init_seq[] = {20,16,26,23,1,0,0,0,0,0,0,0,0,-1};
  int8_t *p = init_seq;

  fifo_reset();
  while (*p != -1 ){
    fifo_put_byte((uint8_t) *p++) ;
  }
}

void vdustring( char *s ) {
  while (*s !=0) {
    fifo_put_byte(*s++);
  }
}

void plot(uint8_t type, int16_t x, int16_t y) {
  fifo_put_byte(25);
  fifo_put_byte(type);
  fifo_put_byte((uint8_t) (x & 0xFF));
  fifo_put_byte((uint8_t) (x >> 8));
  fifo_put_byte((uint8_t) (y & 0xFF));
  fifo_put_byte((uint8_t) (y >> 8));
}

void delay_300th(uint32_t d) {
  uint32_t x = kl_time_please() + d;
  while ( kl_time_please() < x ) {}
}
