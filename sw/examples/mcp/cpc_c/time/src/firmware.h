#include <stdint.h>

extern void gra_line_abs (uint16_t x, uint16_t y)  ;
extern void gra_move_abs (uint16_t x, uint16_t y)  ;
extern void gra_plot_abs (uint16_t x, uint16_t y)  ;
extern void gra_set_pen (uint8_t p) __z88dk_fastcall ;
extern void gra_set_origin (uint16_t x, uint16_t y)  ;



extern uint32_t kl_time_please() ;
extern void kl_time_set(uint32_t t) __z88dk_fastcall ;

extern uint8_t km_wait_char() __z88dk_fastcall ;

extern void scr_set_mode (uint8_t m) __z88dk_fastcall ;

