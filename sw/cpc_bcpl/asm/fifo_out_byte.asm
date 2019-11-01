             ; fifo_out_byte
             ; 
             ; Write a single byte to a FIFO
             ; 
             ; Entry:
             ;  data     IX+127/6 - Byte to be written stored in low byte
             ;  ret addr IX+125/4
             ;  old IX   IX+123/2
             ;  vec ptr  IX+121/0
             ;  f        IX+119/8 - local var init to 0 used to return result
          LD   bc,0xfd81   ; bc points to status register
          IN   a,(c)   ; get status flag
          AND   0x2   
          JR   z,END   ; go to end if FIFO not ready
          DEC   c   ; point to data reg
          LD   a,(IX+126)   
          OUT   (c),a   
          LD   a,0x1   ; successful write
END:         
          LD   (ix+118),a   ; write status back
