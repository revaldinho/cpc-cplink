GET "BCPLLIB.B"

MANIFEST $(
    SZ=3500       // num of ints
    DBL_SZ=SZ<<1  // num of bytes
    QUAD_SZ=SZ<<2 // double num bytes
$)

LET fifo1_in_bytes( rxptr, max) = VALOF $(
  LET f=0
  // Read up to 255 bytes from the FIFO terminating when the
  // FIFO signals EMTPY and returns the actual number read.
  //
  // Entry:
  // max IX+127/6 - Param 2: MAX bytes to write
  // @tr IX+125/4 - Param 1: ptr to first BCPL word
  // ret addr IX+123/2
  // old IX IX+121/0
  // vec ptr IX+119/8
  // f IX+117/6 - local var init to 0 used to return result
  inline #xDD,#x66,#x7D // LD  h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C // LD  l,(ix+124)
  inline #x11,#x00,#x00 // LD  de,00000   ; e = sent byte count
  inline #x01,#x81,#xFD // LD  bc,0xfd81  ; bc point to status reg first
  inline #xDD,#x7E,#x7F // LD  a,(ix+127) ; get high byte of count
  inline #xE6,#xFF      // AND 0xFF       ; check if non-zero
  inline #x28,#x08      // JR  z,next     ; get low byte if zero
  inline #x17           // RLA            ; check sign bit
  inline #x38,#x1B      // JR  c,end      ; exit if sign bit is set
  inline #x16,#xFF      // LD  d,255      ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A           // LD  e,d
  inline #x18,#x09      // JR  top
                        // NEXT:
  inline #xDD,#x7E,#x7E // LD  a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF      // AND 0xff       ; check for zero and exit early
  inline #x28,#x0F      // JR  Z,end
  inline #x57           // LD  d,a        ; d = max bytes
  inline #x5F           // LD  e,a        ; e = max bytes
                        // TOP:
  inline #xED,#x78      // IN  a,(c)      ; get DOR status flag
  inline #x1F           // RRA
  inline #x30,#x08      // JR  nc,end     ; go to end if no data available
  inline #x0D           // DEC c          ; point to data reg
  inline #xED,#xA2      // INI            ; (HL)<-IN(bc), HL++, B--
  inline #x04           // INC b          ; restore B
  inline #x0C           // INC c          ; point to status reg for next check
  inline #x1D           // DEC e
  inline #x20,#xF3      // JR  nz,top     ; if not loop again
                        // END:
  inline #x7A           // LD  a,d        ; get max count
  inline #x93           // SUB e          ; subtract current count
  inline #xDD,#x77,#x74 // LD  (ix+116),a ; write bytes rcvd ready to exit
  RESULTIS f
$)

AND fifo1_out_bytes(txptr, max) = VALOF $(
  LET f=0
  // Write up to 255 bytes to the FIFO terminating when the
  // FIFO signals FULL and returns the actual number written.
  //
  // Entry:
  // max IX+127/6 Param 2: MAX bytes to write
  // @tx IX+125/4 Param 1: pointer to first byte
  // ret addr IX+123/2
  // old IX IX+121/0
  // vec ptr IX+119/8
  // f IX+117/6 local var init to 0 used to return result
  inline #xDD,#x66,#x7D // LD    h,(ix+125) ; HL points to TX buffer
  inline #xDD,#x6E,#x7C // LD    l,(ix+124)
  inline #x11,#x00,#x00 // LD    de,00000   ; e = sent byte count
  inline #x01,#x81,#xFD // LD    bc,0xfd81
  inline #xDD,#x7E,#x7F // LD    a,(ix+127) ; get high byte of count
  inline #xE6,#xFF      // AND   0xFF       ; check if non-zero
  inline #x28,#x08      // JR    z,next     ; get low byte if zero
  inline #x17           // RLA   ; check sign bit
  inline #x38,#x1C      // JR    c,end      ; exit if sign bit is set
  inline #x16,#xFF      // LD    d,255      ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A           // LD    e,d
  inline #x18,#x09      // JR    top
                        // NEXT: ;
  inline #xDD,#x7E,#x7E // LD    a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF      // AND   0xff       ; check for zero and exit early
  inline #x28,#x10      // JR    Z,end
  inline #x57           // LD    d,a        ; d = max bytes
  inline #x5F           // LD    e,a        ; e = max bytes
                        // TOP:  ;
  inline #xED,#x78      // IN    a,(c)      ; get DIR status flag
  inline #xE6,#x02      // AND   0x2
  inline #x28,#x08      // JR    z,end      ; go to end if no data available
  inline #x0D           // DEC   c          ; point to data reg
  inline #x04           // INC   b          ; pre-incr b
  inline #xED,#xA3      // OUTI  ; b--, OUT(bc) <- hl, hl++
  inline #x0C           // INC   c          ; point to status reg for next check
  inline #x1D           // DEC   e
  inline #x20,#xF2      // JR    nz,top     ; if not loop again
                        // END:  ;
  inline #x7A           // LD    a,d        ; get max count
  inline #x93           // SUB   e          ; subtract current count
  inline #xDD,#x77,#x74 // LD    (ix+116),a ; write bytes rcvd ready to exit

  RESULTIS f
$)

AND fifo2_in_bytes_nc( rxptr, max) = VALOF $(
  LET f=0
  // Read up to 255 bytes from the FIFO ignoring FIFO status
  //
  // Entry:
  // max      IX+127/6 - Param 2: MAX bytes to write
  // @rxptr   IX+125/4 - Param 1: ptr to first byte in bfer
  // ret addr IX+123/2
  // old IX   IX+121/0
  // vec ptr  IX+119/8
  // f        IX+117/6 - local var init to 0 used to return result
  inline #xDD,#x66,#x7D // LD    h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C // LD    l,(ix+124)
  inline #x11,#x00,#x00 // LD    de,00000   ; e = sent byte count
  inline #x01,#x80,#xFD // LD    bc,0xfd80  ; point to data register directly
  inline #xDD,#x7E,#x7F // LD    a,(ix+127) ; get high byte of count
  inline #xE6,#xFF      // AND   0xFF       ; check if non-zero
  inline #x28,#x08      // JR    z,next     ; get low byte if zero
  inline #x17           // RLA   ; check sign bit
  inline #x38,#x14      // JR    c,end      ; exit if sign bit is set
  inline #x16,#xFF      // LD    d,255      ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A           // LD    e,d
  inline #x18,#x09      // JR    top
                        // NEXT: ;
  inline #xDD,#x7E,#x7E // LD    a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF      // AND   0xff       ; check for zero and exit early
  inline #x28,#x08      // JR    Z,end
  inline #x57           // LD    d,a        ; d = max bytes
  inline #x5F           // LD    e,a        ; e = max bytes
                        // TOP:  ;
  inline #xED,#xA2      // INI   ; (HL)<-IN(bc), HL++, B--
  inline #x04           // INC   b          ; restore B
  inline #x1D           // DEC   e
  inline #x20,#xFA      // JR    nz,top     ; if not loop again
                        // END:  ;
  inline #xDD,#x72,#x74 // LD    (ix+116),d ; write bytes rcvd ready to exit
  RESULTIS f
$)

AND fifo2_out_bytes_nc(txptr, max) = VALOF $(
  LET f=0
  // Write up to 255 bytes from the FIFO ignoring FIFO status
  //
  // Entry:
  // max      IX+127/6 - Param 2: MAX bytes to write
  // @txptr   IX+125/4 - Param 1: ptr to first byte
  // ret addr IX+123/2
  // old IX   IX+121/0
  // vec ptr  IX+119/8
  // f        IX+117/6 - local var init to 0 used to return result
  inline #xDD,#x66,#x7D // LD    h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C // LD    l,(ix+124)
  inline #x11,#x00,#x00 // LD    de,00000   ; e = sent byte count
  inline #x01,#x80,#xFD // LD    bc,0xfd80  ; point to data register directly
  inline #xDD,#x7E,#x7F // LD    a,(ix+127) ; get high byte of count
  inline #xE6,#xFF      // AND   0xFF       ; check if non-zero
  inline #x28,#x08      // JR    z,next     ; get low byte if zero
  inline #x17           // RLA   ; check sign bit
  inline #x38,#x14      // JR    c,end      ; exit if sign bit is set
  inline #x16,#xFF      // LD    d,255      ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A           // LD    e,d
  inline #x18,#x09      // JR    top
                        // NEXT: ;
  inline #xDD,#x7E,#x7E // LD    a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF      // AND   0xff       ; check for zero and exit early
  inline #x28,#x08      // JR    Z,end
  inline #x57           // LD    d,a        ; d = max bytes
  inline #x5F           // LD    e,a        ; e = max bytes
                        // TOP:  ;
  inline #x04           // INC   b          ; pre increment B
  inline #xED,#xA3      // OUTI   ; B--, OUT(bc) <- (hl), HL++
  inline #x1D           // DEC   e
  inline #x20,#xFA      // JR    nz,top     ; if not loop again
                        // END:  ;
  inline #xDD,#x72,#x74 // LD    (ix+116),d ; write bytes rcvd ready to exit
  RESULTIS f
$)

AND fifo3_in_bytes_nc( rxptr, max) = VALOF $(
  LET f=0
  // Write up to 255 bytes from the FIFO ignoring FIFO status
  // Bytes are transferred in blocks of 4 until fewer than 4 remain.
  //
  // Entry:
  // max     IX+127/6 - Param 2: MAX bytes to write
  // @rxptr  IX+125/4 - Param 1: ptr to first byte
  // ret addrIX+123/2
  // old IX  IX+121/0
  // vec ptr IX+119/8
  // f        IX+117/6 - local var init to 0 to return result
  inline #xDD,#x66,#x7D // LD     h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C // LD     l,(ix+124)
  inline #x1E,#x00      // LD     e,00       ; e = sent byte count
  inline #x01,#x80,#xFD // LD     bc,0xfd80  ; point to data register directly
  inline #xDD,#x7E,#x7F // LD     a,(ix+127) ; get high byte of count
  inline #xE6,#xFF      // AND    0xFF       ; check if non-zero
  inline #x28,#x07      // JR     z,next     ; get low byte if zero
  inline #x17           // RLA    ; check sign bit
  inline #x38,#x2C      // JR     c,end      ; exit if -ve
  inline #x3E,#xFF      // LD     a,255      ; else set a=max bytes=255
  inline #x18,#x07      // JR     nz, start
                        // NEXT:  ;
  inline #xDD,#x7E,#x7E // LD     a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF      // AND    0xff       ; check for zero and exit early
  inline #x28,#x21      // JR     Z,end
                        // START: ;
  inline #x5F           // LD     e,a        ; Copy max bytes to e
  inline #xCB,#x3F      // SRL    A          ; turn A into block of 4 counter
  inline #xCB,#x3F      // SRL    A
  inline #x28,#x14      // JR     z,TOP1     ; if <8 jump to single byte loop
                        // TOP8:  ;
  inline #xED,#xA2      // INI    ; (HL) <-IN(BC), HL++, B--
  inline #x04           // INC    b          ; restore B
  inline #xED,#xA2      // INI
  inline #x04           // INC    b
  inline #xED,#xA2      // INI
  inline #x04           // INC    b
  inline #xED,#xA2      // INI
  inline #x04           // INC    b
  inline #x3D           // DEC    a          ; dec block counter
  inline #x20,#xF1      // JR     nz,top8    ; repeat if not done
  inline #x7B           // LD     a,e        ; get byte count mod 4 - remaining bytes
  inline #xE6,#x03      // AND    0x03
  inline #x28,#x06      // JR     z,end      ; skip to end if none left
                        // TOP1:  ; else finish up one byte at a time
  inline #xED,#xA2      // INI    ; (HL) <-IN(BC), HL++, B--
  inline #x04           // INC    b          ; restore B
  inline #x3D           // DEC    a          ; dec byte counter
  inline #x20,#xFA      // JR     nz,top1
                        // END:   ;
  inline #xDD,#x73,#x74 // LD     (ix+116),e ; write bytes transferred ready to exit
  RESULTIS f
$)

AND fifo3_out_bytes_nc(txptr, max) = VALOF $(
  LET f=0
  // Write up to 255 bytes from the FIFO ignoring FIFO status.
  // Bytes are transferred in blocks of 4 until fewer than 4 remain.
  //
  // Entry:
  // max      IX+127/6 - Param 2: MAX bytes to write
  // @txptr   IX+125/4 - Param 1: ptr to first byte
  // ret addr IX+123/2
  // old IX   IX+121/0
  // vec ptr  IX+119/8
  // f        IX+117/6 - local var init to 0 to return result
  inline #xDD,#x66,#x7D // LD     h,(ix+125) ; HL points to TX buffer
  inline #xDD,#x6E,#x7C // LD     l,(ix+124)
  inline #x1E,#x00      // LD     e,00       ; e = sent byte count
  inline #x01,#x80,#xFD // LD     bc,0xfd80  ; point to data register directly
  inline #xDD,#x7E,#x7F // LD     a,(ix+127) ; get high byte of count
  inline #xE6,#xFF      // AND    0xFF       ; check if non-zero
  inline #x28,#x07      // JR     z,next     ; get low byte if zero
  inline #x17           // RLA    ; check sign bit
  inline #x38,#x2C      // JR     c,end      ; exit if sign bit is set
  inline #x3E,#xFF      // LD     a,255      ; else set a=max bytes=255
  inline #x18,#x07      // JR     start
                        // NEXT:  ;
  inline #xDD,#x7E,#x7E // LD     a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF      // AND    0xff       ; check for zero and exit early
  inline #x28,#x21      // JR     Z,end
                        // START: ;
  inline #x5F           // LD     e,a        ; Copy max bytes to e
  inline #xCB,#x3F      // SRL    A          ; turn A into block of 4 counter
  inline #xCB,#x3F      // SRL    A
  inline #x28,#x14      // JR     z,TOP1     ; if <8 jump to single byte loop
                        // TOP8:  ;
  inline #x04           // INC    b          ; pre incr B
  inline #xED,#xA3      // OUTI   ; B--, OUT(bc) <- (hl), HL++
  inline #x04           // INC    b
  inline #xED,#xA3      // OUTI
  inline #x04           // INC    b
  inline #xED,#xA3      // OUTI
  inline #x04           // INC    b
  inline #xED,#xA3      // OUTI
  inline #x3D           // DEC    a          ; dec block counter
  inline #x20,#xF1      // JR     nz,top8    ; repeat if not done
  inline #x7B           // LD     a,e        ; get byte count mod 4 - remaining bytes
  inline #xE6,#x03      // AND    0x03
  inline #x28,#x06      // JR     z,end      ; skip to end if none left
                        // TOP1:  ; else finish up one byte at a time
  inline #x04           // INC    b          ; pre incr B
  inline #xED,#xA3      // OUTI   ; B--, OUT(bc) <- (hl), HL++
  inline #x3D           // DEC    a
  inline #x20,#xFA      // JR     nz,top1    ; if not loop again
                        // END:   ;
  inline #xDD,#x73,#x74 // LD     (ix+116),e ; write bytes transferred ready to exit
  RESULTIS f
$)

AND fifo_reset() BE $(
    inline #x3E,#xFD     // LD   A,0xFD
    inline #xD3,#x81     // OUT  (0x81),a
$)

AND show_stats(dout,din, t) BE $(
  LET sec, frac = 0,0
  writef("Bytes sent: %n*n", dout)
  writef("Bytes received: %n*n", din)
  sec := (t)/75
  frac := (t - sec*75)*4/3   // convert remainder to 100ths
  writef("Time: %N.", sec)
  writez(frac,2)
  writes("s*n")
$)

AND check_data(v1,v2,n) BE $(
  LET i,e=0,0
  writef("Checking Data...*n")
  FOR i=0 TO n-1 DO $(
      //IF v1!i NE v2!i DO writef("%n  %n *n", v1!i, v2!i)
      IF v1!i NE v2!i DO e:=e+1
  $)
  TEST e=0 THEN writef("No errors*n") ELSE writef("%n errors detected*n",e)
$)

AND start() = VALOF $(
  LET rx = VEC SZ
  LET tx = VEC SZ
  LET i,j,t = 0,0,0
  LET txbyteptr = 0
  LET rxbyteptr = 0

  mode(2)
  fifo_reset()
  WHILE (i<SZ) & (fifo1_in_bytes( @(rx!0), SZ-1))>0 DO $( i:=i+1 $)
  IF ( i>0 ) $(
     writef("Flushed %n bytes from input buffer*n", i)
  $)

  writef("Set up random data*n")
  setseed(scaledtime(1))
  FOR k=0 TO SZ-1 DO $(
    tx!k := randno(31767)-1
    rx!k := 0
  $)

  // Get pointer to absolute byte for transfer
  txbyteptr := (@(tx!0))<<1
  rxbyteptr := (@(rx!0))<<1

  writef("1: Send and Receive, checking status byte by byte*n")
  i:=0
  j:=0
  resettime()
  WHILE (i+j) < (QUAD_SZ) DO $(
    IF i NE DBL_SZ DO i:= i+ (fifo1_out_bytes( txbyteptr+i, DBL_SZ-i) )
    IF j NE DBL_SZ DO j:= j+ (fifo1_in_bytes( rxbyteptr+j, DBL_SZ-j) )
  $)
  t := scaledtime(2)         // ask for time in 75th of sec.

  show_stats(i,j,t)
  check_data( tx, rx, SZ)

  writef("2: Send and Receive 255 byte blocks, no status checks*n")
  i:=0
  j:=0
  resettime()
  WHILE i < DBL_SZ DO i:= i+ (fifo2_out_bytes_nc( txbyteptr+i, DBL_SZ-i) )
  WHILE j < DBL_SZ DO j:= j+ (fifo2_in_bytes_nc( rxbyteptr+j, DBL_SZ-j) )
  t := scaledtime(2)

  show_stats(i,j,t)
  check_data( tx, rx, SZ)

  writef("3: Send and Receive 255 byte blocks, no status checks, 4 byte unrolled loops*n")
  i:=0
  j:=0
  resettime()
  WHILE i < DBL_SZ DO i:= i+ (fifo3_out_bytes_nc( txbyteptr+i, DBL_SZ-i) )
  WHILE j < DBL_SZ DO j:= j+ (fifo3_in_bytes_nc( rxbyteptr+j, DBL_SZ-j) )
  t := scaledtime(2)

  show_stats(i,j,t)
  check_data( tx, rx, SZ)

  presskeytoexit()
  RESULTIS 0
$)
