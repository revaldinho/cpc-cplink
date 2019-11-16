        		; fifo_in_bytes
        		; 
        		; Read up to 255 bytes from the FIFO terminating when the
        		; FIFO signals EMTPY and returns the actual number read.
        		; 
        		; Entry:
        		;  max      IX+127/6 - Param 2: MAX bytes to write
        		;  @tr      IX+125/4 - Param 1: ptr to first BCPL word
        		;  ret addr IX+123/2
        		;  old IX   IX+121/0
        		;  vec ptr  IX+119/8
        		;  f        IX+117/6 - local var init to 0 used to return result
        LD   h,(ix+125) ; HL points to RX buffer
        LD   l,(ix+124)   
        LD   de,00000   ; e = sent byte count
        LD   bc,0xfd81  ; bc point to status reg first
        LD   a,(ix+127) ; get high byte of count
        AND   0xFF      ; check if non-zero
        JR   z,NEXT     ; get low byte if zero
        RLA             ; check sign bit
        JR   c,END      ; exit if sign bit is set
        LD   d,255      ; else, +ve, non-zero hi byte so set size to 255 max
        LD   e,d   
        JR   TOP1       ; count is odd
NEXT:                
        LD   a,(ix+126) ; Max bytes (1..255)
        AND   0xff      ; check for zero and exit early
        JR   Z,END   
        LD   d,a        ; d = max bytes
        LD   e,a        ; e = max bytes
        RRA             ; check if count is odd
        JR   c, TOP1    ; and start at byte 1 if so
TOP2:         
        IN   a,(c)      ; get DOR status flag
        RRA      
        JR   nc,END     ; go to END if no data available
        DEC   c         ; point to data reg
        INI             ; (HL)<-IN(bc), HL++, B--
        INC   b         ; restore B
        INC   c         ; point to status reg for NEXT check
        DEC   e         ; decrement counter but no need to check
TOP1:         
        IN   a,(c)      ; get DOR status flag
        RRA      
        JR   nc,END     ; go to END if no data available
        DEC   c         ; point to data reg
        INI             ; (HL)<-IN(bc), HL++, B--
        INC   b         ; restore B
        INC   c         ; point to status reg for NEXT check
        DEC   e   
        JR   nz,TOP2    ; if not loop again
END:         
        LD   a,d        ; get max count
        SUB   e         ; subtract current count
        LD   (ix+116),a ; write bytes rcvd ready to exit
