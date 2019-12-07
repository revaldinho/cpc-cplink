#include "fifolib.h"

#define QUEUE_SZ  8192

uint8_t queue [QUEUE_SZ];
int rptr = 0;
int wptr = 0;

void hexdump( uint8_t data[], int dlen, int grpsz ) {
  int i, max;
  char *astr = (char * ) malloc ( grpsz + 1);

  memset( astr, 0, grpsz+1);
  max = dlen + (((dlen%grpsz) > 0)?dlen + grpsz - (dlen%grpsz): 0);
  
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
      astr[j]= ((data[i]>31) && (data[i]<128)) ? data[i]: '.';
    }
  }
  if (i%grpsz == 0) printf("%s\n", astr);
}


int main( int argc, char *argv[] ) {
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
    if (GET_DOR) {
      queue[rptr] = read_fifo_byte();
      rptr = (rptr+1) % QUEUE_SZ;
    }
    if ( (wptr!=rptr) && GET_DIR) {
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
