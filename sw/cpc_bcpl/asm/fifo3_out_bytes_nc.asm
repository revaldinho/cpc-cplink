                                ; Write up to 255 bytes from the FIFO ignoring FIFO status
                                ; Bytes are transferred in blocks of 4 until fewer than 4 remain.
                                ;
                                ; Entry:
                                ;  max      IX+127/6 - Param 2: MAX bytes to write
                                ;  @txptr   IX+125/4 - Param 1: ptr to first byte
                                ;  ret addr IX+123/2
                                ;  old IX   IX+121/0
                                ;  vec ptr  IX+119/8
                                ;  f        IX+117/6 - local var init to 0 to return result
        LD   h,(ix+125)         ; HL points to TX buffer
        LD   l,(ix+124)
        LD   e,00               ; e = sent byte count
        LD   bc,0xfd80          ; point to data register directly
        LD   a,(ix+127)         ; get high byte of count
        AND   0xFF              ; check if non-zero
        JR   z,NEXT             ; get low byte if zero
        RLA                     ; check sign bit
        JR   c,END              ; exit if -ve
        LD   a,255              ; else set a=max bytes=255
        JR   START
NEXT:   
        LD   a,(ix+126)         ; Max bytes (1..255)
        AND   0xff              ; check for zero and exit early
        JR   Z,END
START:  
        LD   e,a                ; Copy max bytes to e
        SRL   A                 ; turn A into block of 4 counter
        SRL   A
        JR   z,TOP1             ; if <4 jump to single byte loop
TOP4:   
        INC   b                 ; pre incr b
        OUTI                    ; B--, OUT(BC) <- (hl), HL++
        INC   b   
        OUTI
        INC   b
        OUTI
        INC   b
        OUTI
        DEC   a                 ; dec block counter
        JR   nz,TOP4            ; repeat if not done
        LD   a,e                ; get byte count mod 4 - remaining bytes
        AND   0x03
        JR   z,END              ; skip to END if none left
TOP1:                           ; else finish up one byte at a time
        INC   b                 ; pre incr B        
        OUTI                    ; (HL) <-IN(BC), HL++, B--
        DEC   a                 ; dec byte counter
        JR   nz,TOP1
END:
        LD   (ix+116),e         ; write bytes transferred ready to exit
