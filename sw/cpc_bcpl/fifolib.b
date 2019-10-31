// FIFOLIB.B
//
// A library of functions for the CPC-CPlink project.
// 
// Copyright (C) 2019 Revaldinho
LET fifo_reset() BE $(
    inline #x3E,#xFD             // LD   A,0xFD   
    inline #xD3,#x81             // OUT  (0x81),a   
$)


AND  fifo1_in_bytes (ptr, max) = VALOF $(
  LET f=0
  // Listing source: fifo1_in_bytes.lst
                            //  ; fifo_in_bytes
                            //  ; 
                            //  ; Read up to 255 bytes from the FIFO terminating when the
                            //  ; FIFO signals EMTPY and returns the actual number read.
                            //  ; 
                            //  ; Entry:
                            //  ; max IX+127/6 - Param 2: MAX bytes to write
                            //  ; @tr IX+125/4 - Param 1: ptr to first BCPL word
                            //  ; ret addr IX+123/2
                            //  ; old IX IX+121/0
                            //  ; vec ptr IX+119/8
                            //  ; f IX+117/6 - local var init to 0 used to return result
  inline #xDD,#x66,#x7D     // LD       h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C     // LD       l,(ix+124) 
  inline #x11,#x00,#x00     // LD       de,00000 ; e = sent byte count
  inline #x01,#x81,#xFD     // LD       bc,0xfd81 ; bc point to status reg first
  inline #xDD,#x7E,#x7F     // LD       a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND      0xFF     ; check if non-zero
  inline #x28,#x08          // JR       z,next   ; get low byte if zero
  inline #x17               // RLA      ; check sign bit
  inline #x38,#x1B          // JR       c,end    ; exit if sign bit is set
  inline #x16,#xFF          // LD       d,255    ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A               // LD       e,d      
  inline #x18,#x09          // JR       top      
                            // NEXT:    ; 
  inline #xDD,#x7E,#x7E     // LD       a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND      0xff     ; check for zero and exit early
  inline #x28,#x0F          // JR       Z,end    
  inline #x57               // LD       d,a      ; d = max bytes
  inline #x5F               // LD       e,a      ; e = max bytes
                            // TOP:     ; 
  inline #xED,#x78          // IN       a,(c)    ; get DOR status flag
  inline #x1F               // RRA      
  inline #x30,#x08          // JR       nc,end   ; go to end if no data available
  inline #x0D               // DEC      c        ; point to data reg
  inline #xED,#xA2          // INI      ; (HL)<-IN(bc), HL++, B--
  inline #x04               // INC      b        ; restore B
  inline #x0C               // INC      c        ; point to status reg for next check
  inline #x1D               // DEC      e        
  inline #x20,#xF3          // JR       nz,top   ; if not loop again
                            // END:     ; 
  inline #x7A               // LD       a,d      ; get max count
  inline #x93               // SUB      e        ; subtract current count
  inline #xDD,#x77,#x74     // LD       (ix+116),a ; write bytes rcvd ready to exit
  RESULTIS f
$)

AND  fifo1_out_bytes (ptr, max) = VALOF $(
  LET f=0
  // Listing source: fifo1_out_bytes.lst
                            //  ; fifo_out_bytes
                            //  ; 
                            //  ; Write up to 255 bytes to the FIFO terminating when the
                            //  ; FIFO signals FULL and returns the actual number written.
                            //  ; 
                            //  ; Entry
                            //  ; max IX+127/6 Param 2: MAX bytes to write
                            //  ; @tx IX+125/4 Param 1: pointer to first byte
                            //  ; ret addr IX+123/2
                            //  ; old IX IX+121/0
                            //  ; vec ptr IX+119/8
                            //  ; f IX+117/6 local var init to 0 used to return result
  inline #xDD,#x66,#x7D     // LD       h,(ix+125) ; HL points to TX buffer
  inline #xDD,#x6E,#x7C     // LD       l,(ix+124) 
  inline #x11,#x00,#x00     // LD       de,00000 ; e = sent byte count
  inline #x01,#x81,#xFD     // LD       bc,0xfd81 
  inline #xDD,#x7E,#x7F     // LD       a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND      0xFF     ; check if non-zero
  inline #x28,#x08          // JR       z,next   ; get low byte if zero
  inline #x17               // RLA      ; check sign bit
  inline #x38,#x1C          // JR       c,end    ; exit if sign bit is set
  inline #x16,#xFF          // LD       d,255    ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A               // LD       e,d      
  inline #x18,#x09          // JR       top      
                            // NEXT:    ; 
  inline #xDD,#x7E,#x7E     // LD       a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND      0xff     ; check for zero and exit early
  inline #x28,#x10          // JR       Z,end    
  inline #x57               // LD       d,a      ; d = max bytes
  inline #x5F               // LD       e,a      ; e = max bytes
                            // TOP:     ; 
  inline #xED,#x78          // IN       a,(c)    ; get DIR status flag
  inline #xE6,#x02          // AND      0x2      
  inline #x28,#x08          // JR       z,end    ; go to end if no data available
  inline #x0D               // DEC      c        ; point to data reg
  inline #x04               // INC      b        ; pre-incr b
  inline #xED,#xA3          // OUTI     ; b--, OUT(bc) <- hl, hl++
  inline #x0C               // INC      c        ; point to status reg for next check
  inline #x1D               // DEC      e        
  inline #x20,#xF2          // JR       nz,top   ; if not loop again
                            // END:     ; 
  inline #x7A               // LD       a,d      ; get max count
  inline #x93               // SUB      e        ; subtract current count
  inline #xDD,#x77,#x74     // LD       (ix+116),a ; write bytes rcvd ready to exit
  RESULTIS f
$)

AND  fifo2_in_bytes_nc (ptr, max) = VALOF $(
  LET f=0
  // Listing source: fifo2_in_bytes_nc.lst
                            //  ; fifo_in_bytes_nc
                            //  ; 
                            //  ; Read up to 255 bytes from the FIFO ignoring FIFO status
                            //  ; flag states.
                            //  ; .
                            //  ; Entry:
                            //  ; max IX+127/6 - Param 2: MAX bytes to write
                            //  ; @rxptr IX+125/4 - Param 1: ptr to first byte in buffer
                            //  ; ret addr IX+123/2
                            //  ; old IX IX+121/0
                            //  ; vec ptr IX+119/8
                            //  ; f IX+117/6 - local var init to 0 used to return result
  inline #xDD,#x66,#x7D     // LD       h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C     // LD       l,(ix+124) 
  inline #x11,#x00,#x00     // LD       de,00000 ; e = sent byte count
  inline #x01,#x80,#xFD     // LD       bc,0xfd80 ; point to data register directly
  inline #xDD,#x7E,#x7F     // LD       a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND      0xFF     ; check if non-zero
  inline #x28,#x08          // JR       z,next   ; get low byte if zero
  inline #x17               // RLA      ; check sign bit
  inline #x38,#x14          // JR       c,end    ; exit if sign bit is set
  inline #x16,#xFF          // LD       d,255    ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A               // LD       e,d      
  inline #x18,#x09          // JR       top      
                            // NEXT:    ; 
  inline #xDD,#x7E,#x7E     // LD       a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND      0xff     ; check for zero and exit early
  inline #x28,#x08          // JR       Z,end    
  inline #x57               // LD       d,a      ; d = max bytes
  inline #x5F               // LD       e,a      ; e = max bytes
                            // TOP:     ; 
  inline #xED,#xA2          // INI      ; (HL)<-IN(bc), HL++, B--
  inline #x04               // INC      b        ; restore B
  inline #x1D               // DEC      e        
  inline #x20,#xFA          // JR       nz,top   ; if not loop again
                            // END:     ; 
  inline #xDD,#x72,#x74     // LD       (ix+116),d ; write num bytes ready to exit
  RESULTIS f
$)

AND  fifo2_out_bytes_nc (ptr, max) = VALOF $(
  LET f=0
  // Listing source: fifo2_out_bytes_nc.lst
                            //  ; fifo_out_bytes_nc
                            //  ; 
                            //  ; Write up to 255 bytes from the FIFO ignoring FIFO status
                            //  ; flag states.
                            //  ; .
                            //  ; Entry:
                            //  ; max IX+127/6 - Param 2: MAX bytes to write
                            //  ; @txptr IX+125/4 - Param 1: ptr to first byte
                            //  ; ret addr IX+123/2
                            //  ; old IX IX+121/0
                            //  ; vec ptr IX+119/8
                            //  ; f IX+117/6 - local var init to 0 used to return result
  inline #xDD,#x66,#x7D     // LD       h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C     // LD       l,(ix+124) 
  inline #x11,#x00,#x00     // LD       de,00000 ; e = sent byte count
  inline #x01,#x80,#xFD     // LD       bc,0xfd80 ; point to data register directly
  inline #xDD,#x7E,#x7F     // LD       a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND      0xFF     ; check if non-zero
  inline #x28,#x08          // JR       z,next   ; get low byte if zero
  inline #x17               // RLA      ; check sign bit
  inline #x38,#x14          // JR       c,end    ; exit if sign bit is set
  inline #x16,#xFF          // LD       d,255    ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A               // LD       e,d      
  inline #x18,#x09          // JR       top      
                            // NEXT:    ; 
  inline #xDD,#x7E,#x7E     // LD       a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND      0xff     ; check for zero and exit early
  inline #x28,#x08          // JR       Z,end    
  inline #x57               // LD       d,a      ; d = max bytes
  inline #x5F               // LD       e,a      ; e = max bytes
                            // TOP:     ; 
  inline #x04               // INC      b        ; pre increment B
  inline #xED,#xA3          // OUTI     ; B--, OUT(bc) <- (hl), HL++
  inline #x1D               // DEC      e        
  inline #x20,#xFA          // JR       nz,top   ; if not loop again
                            // END:     ; 
  inline #xDD,#x72,#x74     // LD       (ix+116),d ; write bytes rcvd ready to exit
  RESULTIS f
$)

AND  fifo3_in_bytes_nc (ptr, max) = VALOF $(
  LET f=0
  // Listing source: fifo3_in_bytes_nc.lst
                            //  ; 
                            //  ; Write up to 255 bytes from the FIFO ignoring FIFO status
                            //  ; flag states. To improve speed, bytes are transferred in
                            //  ; blocks of 4 until fewer than 4 remain.
                            //  ; 
                            //  ; Entry:
                            //  ; max IX+127/6 - Param 2: MAX bytes to write
                            //  ; @rxptr IX+125/4 - Param 1: ptr to first byte
                            //  ; ret addr IX+123/2
                            //  ; old IX IX+121/0
                            //  ; vec ptr IX+119/8
                            //  ; f IX+117/6 - local var init to 0 to return result
  inline #xDD,#x66,#x7D     // LD       h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C     // LD       l,(ix+124) 
  inline #x1E,#x00          // LD       e,00     ; e = sent byte count
  inline #x01,#x80,#xFD     // LD       bc,0xfd80 ; point to data register directly
  inline #xDD,#x7E,#x7F     // LD       a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND      0xFF     ; check if non-zero
  inline #x28,#x07          // JR       z,next   ; get low byte if zero
  inline #x17               // RLA      ; check sign bit
  inline #x38,#x2C          // JR       c,end    ; exit if -ve
  inline #x3E,#xFF          // LD       a,255    ; else set a=max bytes=255
  inline #x18,#x07          // JR       start    
                            // NEXT:    ; 
  inline #xDD,#x7E,#x7E     // LD       a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND      0xff     ; check for zero and exit early
  inline #x28,#x21          // JR       Z,end    
                            // START:   ; 
  inline #x5F               // LD       e,a      ; Copy max bytes to e
  inline #xCB,#x3F          // SRL      A        ; turn A into block of 4 counter
  inline #xCB,#x3F          // SRL      A        
  inline #x28,#x14          // JR       z,TOP1   ; if <8 jump to single byte loop
                            // TOP8:    ; 
  inline #xED,#xA2          // INI      ; (HL) <-IN(BC), HL++, B--
  inline #x04               // INC      b        ; restore B
  inline #xED,#xA2          // INI      
  inline #x04               // INC      b        
  inline #xED,#xA2          // INI      
  inline #x04               // INC      b        
  inline #xED,#xA2          // INI      
  inline #x04               // INC      b        
  inline #x3D               // DEC      a        ; dec block counter
  inline #x20,#xF1          // JR       nz,top8  ; repeat if not done
  inline #x7B               // LD       a,e      ; get byte count mod 4 - remaining bytes
  inline #xE6,#x03          // AND      0x03     
  inline #x28,#x06          // JR       z,end    ; skip to end if none left
                            // TOP1:    ; else finish up one byte at a time
  inline #xED,#xA2          // INI      ; (HL) <-IN(BC), HL++, B--
  inline #x04               // INC      b        ; restore B
  inline #x3D               // DEC      a        ; dec byte counter
  inline #x20,#xFA          // JR       nz,top1  
                            // END:     ; 
  inline #xDD,#x73,#x74     // LD       (ix+116),e ; write bytes transferred ready to exit
  RESULTIS f
$)

AND  fifo3_out_bytes_nc (ptr, max) = VALOF $(
  LET f=0
  // Listing source: fifo3_out_bytes_nc.lst
                            //  ; 
                            //  ; Write up to 255 bytes from the FIFO ignoring FIFO status
                            //  ; Bytes are transferred in blocks of 4 until fewer than 4 remain.
                            //  ; 
                            //  ; Entry:
                            //  ; max IX+127/6 - Param 2: MAX bytes to write
                            //  ; @txptr IX+125/4 - Param 1: ptr to first byte
                            //  ; ret addr IX+123/2
                            //  ; old IX IX+121/0
                            //  ; vec ptr IX+119/8
                            //  ; f IX+117/6 - local var init to 0 to return result
  inline #xDD,#x66,#x7D     // LD       h,(ix+125) ; HL points to TX buffer
  inline #xDD,#x6E,#x7C     // LD       l,(ix+124) 
  inline #x1E,#x00          // LD       e,00     ; e = sent byte count
  inline #x01,#x80,#xFD     // LD       bc,0xfd80 ; point to data register directly
  inline #xDD,#x7E,#x7F     // LD       a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND      0xFF     ; check if non-zero
  inline #x28,#x07          // JR       z,next   ; get low byte if zero
  inline #x17               // RLA      ; check sign bit
  inline #x38,#x2C          // JR       c,end    ; exit if sign bit is set
  inline #x3E,#xFF          // LD       a,255    ; else set a=max bytes=255
  inline #x18,#x07          // JR       start    
                            // NEXT:    ; 
  inline #xDD,#x7E,#x7E     // LD       a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND      0xff     ; check for zero and exit early
  inline #x28,#x21          // JR       Z,end    
                            // START:   ; 
  inline #x5F               // LD       e,a      ; Copy max bytes to e
  inline #xCB,#x3F          // SRL      A        ; turn A into block of 4 counter
  inline #xCB,#x3F          // SRL      A        
  inline #x28,#x14          // JR       z,TOP1   ; if <4 jump to single byte loop
                            // TOP4:    ; 
  inline #x04               // INC      b        ; pre incr B
  inline #xED,#xA3          // OUTI     ; B--, OUT(bc) <- (hl), HL++
  inline #x04               // INC      b        
  inline #xED,#xA3          // OUTI     
  inline #x04               // INC      b        
  inline #xED,#xA3          // OUTI     
  inline #x04               // INC      b        
  inline #xED,#xA3          // OUTI     
  inline #x3D               // DEC      a        ; dec block counter
  inline #x20,#xF1          // JR       nz,top4  ; repeat if not done
  inline #x7B               // LD       a,e      ; get byte count mod 4 - remaining bytes
  inline #xE6,#x03          // AND      0x03     
  inline #x28,#x06          // JR       z,end    ; skip to end if none left
                            // TOP1:    ; else finish up one byte at a time
  inline #x04               // INC      b        ; pre incr B
  inline #xED,#xA3          // OUTI     ; B--, OUT(bc) <- (hl), HL++
  inline #x3D               // DEC      a        
  inline #x20,#xFA          // JR       nz,top1  ; if not loop again
                            // END:     ; 
  inline #xDD,#x73,#x74     // LD       (ix+116),e ; write bytes transferred ready to exit
  RESULTIS f
$)

AND  fifo_in_byte (ptr) = VALOF $(
  LET f=0
  // Listing source: fifo_in_byte.lst
                            //  ; fifo_in_byte
                            //  ; 
                            //  ; Read a single byte and store it in the location pointed to by
                            //  ; a parameter
                            //  ; 
                            //  ; Entry:
                            //  ; ptr IX+127/6 - Param: ptr to memory to store result
                            //  ; ret addr IX+125/4
                            //  ; old IX IX+123/2
                            //  ; vec ptr IX+121/0
                            //  ; f IX+119/8 - local var init to 0 used to return result
  inline #xDD,#x66,#x7F     // LD       h,(ix+127) ; HL points to RX buffer
  inline #xDD,#x6E,#x7E     // LD       l,(ix+126) 
  inline #x01,#x81,#xFD     // LD       bc,0xfd81 ; bc points to status register
  inline #xED,#x78          // IN       a,(c)    ; get DOR status flag
  inline #xE6,#x01          // AND      0x1      
  inline #x28,#x03          // JR       z,end    ; go to end if no data available
  inline #x0D               // DEC      c        ; point to data reg
  inline #xED,#xA2          // INI      ; (HL)<-IN(bc), HL++, B--
                            // END:     ; 
  inline #xDD,#x77,#x76     // LD       (ix+118),a ; write status back
  RESULTIS f
$)

AND  fifo_out_byte (ptr) = VALOF $(
  LET f=0
  // Listing source: fifo_out_byte.lst
                            //  ; fifo_out_byte
                            //  ; 
                            //  ; Write a single byte to a FIFO
                            //  ; 
                            //  ; Entry:
                            //  ; data IX+127/6 - Byte to be written stored in low byte
                            //  ; ret addr IX+125/4
                            //  ; old IX IX+123/2
                            //  ; vec ptr IX+121/0
                            //  ; f IX+119/8 - local var init to 0 used to return result
  inline #x01,#x81,#xFD     // LD       bc,0xfd81 ; bc points to status register
  inline #xED,#x78          // IN       a,(c)    ; get status flag
  inline #xE6,#x02          // AND      0x2      
  inline #x28,#x08          // JR       z,end    ; go to end if FIFO not ready
  inline #x0D               // DEC      c        ; point to data reg
  inline #xDD,#x7E,#x7E     // LD       a,(IX+126) 
  inline #xED,#x79          // OUT      (c),a    
  inline #x3E,#x01          // LD       a,0x1    ; successful write
                            // END:     ; 
  inline #xDD,#x77,#x76     // LD       (ix+118),a ; write status back
  RESULTIS f
$)

