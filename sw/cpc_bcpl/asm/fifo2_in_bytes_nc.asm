             ; fifo_in_bytes_nc
             ; 
             ; Read up to 255 bytes from the FIFO ignoring FIFO status
             ; flag states.
             ; .
             ; Entry:
             ;  max      IX+127/6 - Param 2: MAX bytes to write
             ;  @rxptr   IX+125/4 - Param 1: ptr to first byte in buffer
             ;  ret addr IX+123/2
             ;  old IX   IX+121/0
             ;  vec ptr  IX+119/8
             ;  f        IX+117/6 - local var init to 0 used to return result
          LD   h,(ix+125)   ; HL points to RX buffer
          LD   l,(ix+124)   
          LD   de,00000   ; e = sent byte count
          LD   bc,0xfd80   ; point to data register directly
          LD   a,(ix+127)   ; get high byte of count
          AND   0xFF   ; check if non-zero
          JR   z,NEXT   ; get low byte if zero
          RLA      ; check sign bit
          JR   c,END   ; exit if sign bit is set
          LD   d,255   ; else, +ve, non-zero hi byte so set size to 255 max
          LD   e,d   
          JR   TOP   
NEXT:        
          LD   a,(ix+126)   ; Max bytes (1..255)
          AND   0xff   ; check for zero and exit early
          JR   Z,END   
          LD   d,a   ; d = max bytes
          LD   e,a   ; e = max bytes
TOP:         
          INI      ; (HL)<-IN(bc), HL++, B--
          INC   b   ; restore B
          DEC   e   
          JR   nz,TOP   ; if not loop again
END:         
          LD   (ix+116),d   ; write num bytes ready to exit
