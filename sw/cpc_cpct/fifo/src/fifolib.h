

extern void    fifo_reset() ;
extern uint8_t fifo_in_byte(uint8_t *buffer) __z88dk_fastcall;
extern uint8_t fifo_out_byte(uint8_t b) __z88dk_fastcall;
extern uint8_t fifo_in_bytes(uint8_t *buffer, uint16_t num_bytes) ;
extern uint8_t fifo_out_bytes(uint8_t *buffer, uint16_t num_bytes) ;
extern uint8_t fifo_in_nc_bytes(uint8_t *buffer, uint16_t num_bytes) ;
extern uint8_t fifo_out_nc_bytes(uint8_t *buffer, uint16_t num_bytes) ;
