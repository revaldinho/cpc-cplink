GET "BCPLLIB.B"

MANIFEST $(
    SZ=4096      // num of ints
    DBL_SZ=SZ<<1 // num of bytes
$)

LET fifo_in_bytes( rxptr, max) = VALOF $(
  LET f=0

  // Listing source: fifo_in_bytes.lst
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
  inline #xDD,#x66,#x7D     // LD         h,(ix+125) ; HL points to RX buffer
  inline #xDD,#x6E,#x7C     // LD         l,(ix+124) 
  inline #x11,#x00,#x00     // LD         de,00000   ; e = sent byte count
  inline #x01,#x81,#xFD     // LD         bc,0xfd81  ; bc point to status reg first
  inline #xDD,#x7E,#x7F     // LD         a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND        0xFF       ; check if non-zero
  inline #x28,#x08          // JR         z,next     ; get low byte if zero
  inline #x17               // RLA        ; check sign bit
  inline #x38,#x1B          // JR         c,end      ; exit if sign bit is set
  inline #x16,#xFF          // LD         d,255      ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A               // LD         e,d        
  inline #x18,#x09          // JR         top        
                            // NEXT:      ; 
  inline #xDD,#x7E,#x7E     // LD         a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND        0xff       ; check for zero and exit early
  inline #x28,#x0F          // JR         Z,end      
  inline #x57               // LD         d,a        ; d = max bytes
  inline #x5F               // LD         e,a        ; e = max bytes
                            // TOP:       ; 
  inline #xED,#x78          // IN         a,(c)      ; get DOR status flag
  inline #x1F               // RRA        
  inline #x30,#x08          // JR         nc,end     ; go to end if no data available
  inline #x0D               // DEC        c          ; point to data reg
  inline #xED,#xA2          // INI        ; (HL)<-IN(bc), HL++, B--
  inline #x04               // INC        b          ; restore B
  inline #x0C               // INC        c          ; point to status reg for next check
  inline #x1D               // DEC        e          
  inline #x20,#xF3          // JR         nz,top     ; if not loop again
                            // END:       ; 
  inline #x7A               // LD         a,d        ; get max count
  inline #x93               // SUB        e          ; subtract current count
  inline #xDD,#x77,#x74     // LD         (ix+116),a ; write bytes rcvd ready to exit
  RESULTIS f                               
$)


AND fifo_out_bytes(txptr, max) = VALOF $(
  LET f=0

  // Listing source: fifo_out_bytes.lst
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
  inline #xDD,#x66,#x7D     // LD         h,(ix+125) ; HL points to TX buffer
  inline #xDD,#x6E,#x7C     // LD         l,(ix+124) 
  inline #x11,#x00,#x00     // LD         de,00000   ; e = sent byte count
  inline #x01,#x81,#xFD     // LD         bc,0xfd81  
  inline #xDD,#x7E,#x7F     // LD         a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND        0xFF       ; check if non-zero
  inline #x28,#x08          // JR         z,next     ; get low byte if zero
  inline #x17               // RLA        ; check sign bit
  inline #x38,#x1C          // JR         c,end      ; exit if sign bit is set
  inline #x16,#xFF          // LD         d,255      ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x5A               // LD         e,d        
  inline #x18,#x09          // JR         top        
                            // NEXT:      ; 
  inline #xDD,#x7E,#x7E     // LD         a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND        0xff       ; check for zero and exit early
  inline #x28,#x10          // JR         Z,end      
  inline #x57               // LD         d,a        ; d = max bytes
  inline #x5F               // LD         e,a        ; e = max bytes
                            // TOP:       ; 
  inline #xED,#x78          // IN         a,(c)      ; get DIR status flag
  inline #xE6,#x02          // AND        0x2        
  inline #x28,#x08          // JR         z,end      ; go to end if no data available
  inline #x0D               // DEC        c          ; point to data reg
  inline #x04               // INC        b          ; pre-incr b
  inline #xED,#xA3          // OUTI       ; b--, OUT(bc) <- hl, hl++
  inline #x0C               // INC        c          ; point to status reg for next check
  inline #x1D               // DEC        e          
  inline #x20,#xF2          // JR         nz,top     ; if not loop again
                            // END:       ; 
  inline #x7A               // LD         a,d        ; get max count
  inline #x93               // SUB        e          ; subtract current count
  inline #xDD,#x77,#x74     // LD         (ix+116),a ; write bytes rcvd ready to exit
  
  RESULTIS f
$)

AND fifo_reset() BE $(
    inline #x3E,#xFD             // LD   A,0xFD   
    inline #xD3,#x81             // OUT  (0x81),a
$)

AND start() = VALOF $(
  LET rx = VEC SZ
  LET tx = VEC SZ
  LET i,j,t,e = 0,0,0,0
  LET frac,sec = 0,0
  LET txbyteptr = 0  
  LET rxbyteptr = 0

  mode(2)
  fifo_reset()
  WHILE (i<SZ) & (fifo_in_bytes( @(rx!0), SZ-1))>0 DO $( i:=i+1 $)
  IF ( i>0 ) $(
     writef("Flushed %n bytes from input buffer*n", i)
  $)

  writef("Setting up random data*n")
  setseed(scaledtime(1))
  FOR k=0 TO SZ-1 DO $(
    tx!k := randno(31767)-1
    rx!k := 0
  $)

  writef("Test 1: Sending and Receiving, checking status byte by byte*n")
  i:=0
  j:=0
  // Get pointer to absolute byte for transfer
  txbyteptr := (@(tx!0))<<1
  rxbyteptr := (@(rx!0))<<1
  writef("Sending/Receiving Data*n")
  resettime()
  WHILE (i+j) < (DBL_SZ+DBL_SZ) DO $(
    IF i NE DBL_SZ DO i:= i+ (fifo_out_bytes( txbyteptr+i, DBL_SZ-i) )
    IF j NE DBL_SZ DO j:= j+ (fifo_in_bytes( rxbyteptr+j, DBL_SZ-j) )
  $)
  t := scaledtime(2)         // ask for time in 75th of sec.

  writef("Bytes sent: %n*n", i)
  writef("Bytes received: %n*n", j)
  sec := (t)/75              // seconds
  frac := (t - sec*75)*4/3   // convert remainder to 100ths 
  writef("Time: %N.", sec)
  writez(frac,2)
  writes("s*n")
  
  writef("Checking Data...*n")
  FOR i=0 TO SZ-1 DO $(
      //writef("%n  %n *n", tx!i, rx!i)
      IF rx!i NE tx!i DO e:=e+1
  $) 
  TEST e=0 THEN writef("No errors*n") ELSE writef("%n errors detected*n",e)

  presskeytoexit()
  RESULTIS 0
$)



