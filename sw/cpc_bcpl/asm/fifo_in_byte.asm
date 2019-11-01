                                ; fifo_in_byte
                                ; 
                                ; Read a single byte and store it in the location pointed to by
                                ; a parameter
                                ; 
                                ; Entry:
                                ;  ptr      IX+127/6 - Param: ptr to memory to store result
                                ;  ret addr IX+125/4
                                ;  old IX   IX+123/2
                                ;  vec ptr  IX+121/0
                                ;  f        IX+119/8 - local var init to 0 used to return result
        LD   h,(ix+127)         ; HL points to RX buffer
        LD   l,(ix+126)   
        LD   bc,0xfd81          ; bc points to status register
        IN   a,(c)              ; get DOR status flag
        AND   0x1   
        JR   z,END              ; go to END if no data available
        DEC   c                 ; point to data reg
        INI                     ; (HL)<-IN(bc), HL++, B--
END:                 
        LD   (ix+118),a         ; write status back
