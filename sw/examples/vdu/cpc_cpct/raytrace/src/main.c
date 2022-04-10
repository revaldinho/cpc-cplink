#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fifolib.h>
#include <string.h>
#include <math.h>
#include <fifolib.h>
#include <firmware.h>

void init();
void plot(int16_t x, int16_t y, int8_t col);
void vdustring( char *s ) ;

#define BALLS 4
#define SQRT(a) sqrtf(a)

typedef struct {
    float x,y,z,r,q,v;
} Ball;

#define BALL(i,xf,yf,zf,rad) balls[i].x=xf; balls[i].y=yf; balls[i].z=zf; balls[i].r =rad; balls[i].q=rad*rad; balls[i].v=0;

Ball balls[BALLS];
char strbuf[128];

const int16_t c1[] = {0,6,1,7,5,8,15};

int16_t main(int16_t argc, char *argv[]) {

  uint32_t t;
  uint16_t sec, frac;
  uint8_t  col;
  init();
  kl_time_set(0);
    for(int16_t i = 0; i < 1024; i+=4) {
      for(int16_t j = 0; j < 1280; j+=8) {
            float x = 0.3;
            float y = -0.5;
            float z = 0;
            int8_t ba = 2;
            float dx = j - 640;
            float dy = 512-i;
            float dz = 1300;
            float dd = dx * dx + dy * dy + dz * dz;

            int8_t finish = 0;
            int8_t n;
            float s = 0;
            do {
                n = (y >= 0 || dy <= 0) ? -1 : 0;
                if (n == 0) {
                    s = -y / dy;
                }

                for(int8_t k=0; k < BALLS; ++k ) {
                    Ball *b=&balls[k];
                    float px = b->x - x;
                    float py = b->y - y;
                    float pz = b->z - z;
                    float sc = px * dx + py * dy + pz * dz;
                    if (sc > 0) {
                        float bb = sc * sc / dd;
                        float aa = b->q - (px * px + py * py + pz * pz) + bb;
                        if (aa > 0) {
                            sc = (SQRT(bb) - SQRT(aa)) / SQRT(dd);
                            if (sc < s || n < 0) {
                                n = k + 1;
                                s = sc;
                            }
                        }
                    }
                }

                if (n < 0) {
                  col = 0 + (dy*dy/dd) * 7;
                  n = 0;
                  finish = 1;
                } else {
                  dx = dx * s;
                  dy = dy * s;
                  dz = dz * s;
                  dd = dd * s * s;
                  x = x + dx;
                  y = y + dy;
                  z = z + dz;
                  if (n == 0) {
                    for (int8_t k = 0 ; k < BALLS; ++k) {
                      Ball *b=&balls[k];
                      float u = b->x - x;
                      float v = b->z - z;
                      if (u * u + v * v <= b->q) {
                        //ba = 5;
                        //k = BALLS;
                        ba=1;
                      }
                    }
                    if (((x - (int16_t)floorf(x)) > 0.5) == ((z - (int16_t)floorf(z)) > 0.5)) {
                      col = c1[ba];
                    } else {
                      col = c1[ba + 1];
                    }
                    finish = 1;
                  } else {
                    Ball *b=&balls[n-1];
                    float nx = x - b->x;
                    float ny = y - b->y;
                    float nz = z - b->z;
                    float nn = nx * nx + ny * ny + nz * nz;
                    float l = 2 * (dx * nx + dy * ny + dz * nz) / nn;
                    dx = dx - nx * l;
                    dy = dy - ny * l;
                    dz = dz - nz * l;
                  }
                }
            } while(!finish);
            plot(j, i, col);
        }
    }
    t = kl_time_please();
    sec = (uint16_t)(t/300);
    frac = (uint16_t)((t % 300)/3);
    sprintf(strbuf, "Time: %d.%02d s\r\n", sec, frac);vdustring(strbuf);
    printf("Press any key to exit\r");
    km_wait_char();
    return (0);
}

void init()
{
  const uint8_t d[] ={4,1,5,2,6,3};

  BALL(0, -0.8,-1,3.2, 0.7);
  BALL(1, 0,-0.45,2, 0.3);
  BALL(2, 1.2,-0.7,2.5, 0.5);
  BALL(3, 0.4,-1,4, 0.4);
  //    BALL(4, 0.5,-0.5,1.5, 0.1);

  // |VDU,22,2
  fifo_put_byte(22);
  fifo_put_byte(2);
  // |VDU,23,&A,20,0,0,0,0,0,0,0
  fifo_put_byte(23);
  fifo_put_byte(0x0A);
  fifo_put_byte(20);
  fifo_put_byte(0);
  fifo_put_byte(0);
  fifo_put_byte(0);
  fifo_put_byte(0);
  fifo_put_byte(0);
  fifo_put_byte(0);
  fifo_put_byte(0);

  for (uint8_t n=0; n<6; n++){
    fifo_put_byte(19);
    fifo_put_byte(n+1);
    fifo_put_byte(d[n]);
    fifo_put_byte(0);
    fifo_put_byte(0);
    fifo_put_byte(0);
  }
}

void plot(int16_t x, int16_t y, int8_t col)
{
  // VDU 18,0,col;
  fifo_put_byte(18);
  fifo_put_byte(0);
  fifo_put_byte((uint8_t) col);
  // VDU 25,69,xl,xh,yl,yh
  fifo_put_byte(25);
  fifo_put_byte(69);
  fifo_put_byte((uint8_t) (x&0xFF));
  fifo_put_byte((uint8_t) (x>>8));
  fifo_put_byte((uint8_t) (y&0xFF));
  fifo_put_byte((uint8_t) (y>>8));

}

void vdustring( char *s ) {
  while (*s !=0) {
    fifo_put_byte(*s++);
  }
}
