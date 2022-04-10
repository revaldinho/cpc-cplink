// main.c - an loopback demo for the CPC-CPLink board
// Copyright (C) 2019  Revaldinho
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <firmware.h>
#include <fifolib.h>

#define SZ      8192   // num of bytes
#define DBL_SZ  SZ<<1  // double num of bytes

// Manage memory directly
#define MEMPOOL 0x4000

void fifo_flush();
void show_stats(uint16_t dout,uint16_t din, uint32_t t);
void check_data(uint8_t *v1, uint8_t *v2, uint16_t n);

void main ( void ) {
  uint16_t i,j;
  uint32_t t ;
  uint8_t *tx_p = (uint8_t *)MEMPOOL ;
  uint8_t *rx_p = (uint8_t *)MEMPOOL+SZ;
  uint8_t count ;

  scr_set_mode(2);

  fifo_reset();
  fifo_flush();

  printf("Set up random data\r\n");
  srand(kl_time_please());
  for (i=0; i<SZ; i++) {
    *(tx_p+i) = rand() & 0xFFFF ;
    *(rx_p+i) = 0;
  }

  puts("\nTest 1: Send/Receive 1 byte at a time, check FIFO status per byte\r");
  kl_time_set(0);
  for ( i=0, j=0; (i+j) < DBL_SZ; ) {
    if (i < SZ) {
      i += fifo_out_byte(*(tx_p+i));
    }
    if (j < SZ) {
      j += fifo_in_byte(rx_p+j);
    }
  }
  t = kl_time_please();

  show_stats(i,j,t);
  check_data( tx_p, rx_p, SZ);

  puts("\nTest 2: Send/Receive 1 byte at a time, check FIFO status per byte\r");
  puts("         (Use blocking read/write routines)\r");

  kl_time_set(0);
  for ( i=0 ; i < SZ; i++ ) {
    fifo_put_byte(*(tx_p+i));
    *(rx_p+i) = fifo_get_byte();
  }

  t = kl_time_please();
  show_stats(i,j,t);
  check_data( tx_p, rx_p, SZ);

  puts("\nTest 3: Send/Receive multiple bytes, check FIFO status per byte\r");
  kl_time_set(0);
  for ( i=0, j=0; (i+j) < DBL_SZ; ) {
    if (i < SZ) {
      i+=fifo_out_bytes( tx_p+i, SZ-i);
    }
    if (j < SZ) {
      j += fifo_in_bytes( rx_p+j, SZ-j);
    }
  }
  t = kl_time_please();

  show_stats(i,j,t);
  check_data( tx_p, rx_p, SZ);

  puts("\nTest 4: Send/Receive 255 byte blocks, no status check\r");
  kl_time_set(0);
  for ( i=0, j=0; (i+j) < DBL_SZ; ) {
    while ( i< SZ ) {
      i+=fifo_out_nc_bytes( tx_p+i, SZ-i);
    }
    while (j < SZ) {
      j += fifo_in_nc_bytes( rx_p+j, SZ-j);
    }
  }
  t = kl_time_please();

  show_stats(i,j,t);
  check_data( tx_p, rx_p, SZ);

  puts("Press any key to exit\r");
  km_wait_char();
}

void fifo_flush() {
  int i;
  uint8_t bufbyte;
  for ( i=0;  (i<16384) & (fifo_in_byte(&bufbyte)!=0); i++ ) { }
  if ( i>0 ) printf("Flushed %d bytes from input buffer\r\n", i);
}

void show_stats(uint16_t dout,uint16_t din, uint32_t t){
  uint16_t sec = (uint16_t)(t/300);
  uint16_t frac = (uint16_t)((t % 300)/3);
  printf("Bytes sent: %d\r\n", dout);
  printf("Bytes received: %d\r\n", din);
  printf("Time: %d.%02d s\r\n", sec, frac);
}

void check_data(uint8_t *v1, uint8_t *v2, uint16_t n) {
  uint16_t i,e=0;
  printf("Checking Data...");
  for (i=0;  i< n; i++) {
    //printf ( "Expected %4x Actual %4x\r\n", *(v1+i), *(v2+i));
    e += ( *(v1+i) != *(v2+i))? 1:0;
  }
  if (e==0) {
    puts("No errors\r");
  } else {
    printf("%d errors detected\r\n",e);
  }
}
