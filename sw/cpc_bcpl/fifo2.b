GET "BCPLLIB.B"

MANIFEST $(
    SZ=4096
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
  inline #x29               // ADD        hl,hl      ; double HL to convert BCPL int adr to absolute
  inline #x1E,#x00          // LD         e,0        ; e = sent byte count
  inline #x01,#x81,#xFD     // LD         bc,0xfd81  
  inline #xDD,#x7E,#x7F     // LD         a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND        0xFF       ; check if non-zero
  inline #x28,#x08          // JR         z,next     ; get low byte if zero
  inline #xE6,#x80          // AND        0x80       ; check sign bit
  inline #x20,#x1E          // JR         nz,end     ; exit if sign bit is set
  inline #x16,#xFF          // LD         d,255      ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x18,#x08          // JR         top        
                            // NEXT:      ; 
  inline #xDD,#x7E,#x7E     // LD         a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND        0xff       ; check for zero and exit early
  inline #x28,#x13          // JR         Z,end      
  inline #x57               // LD         d,a        ; d = max bytes
                            // TOP:       ; 
  inline #xED,#x78          // IN         a,(c)      ; get DOR status flag
  inline #xE6,#x01          // AND        0x1        
  inline #x28,#x0C          // JR         z,end      ; go to end if no data available
  inline #x1C               // INC        e          ; inc byte count
  inline #x0D               // DEC        c          ; point to data reg
  inline #xED,#x78          // IN         a,(c)      
  inline #x77               // LD         (hl),a     
  inline #x23               // INC        hl         ; inc twice because data is stored in BCPL ints
  inline #x23               // INC        hl         ; and we are transferring only the low bytes
  inline #x0C               // INC        c          ; point to status reg for next check
  inline #x7B               // LD         a,e        ; transfer byte count to a
  inline #xBA               // CP         d          ; reached the max ?
  inline #x20,#xEE          // JR         nz,top     ; if not loop again
                            // END:       ; 
  inline #xDD,#x73,#x74     // LD         (ix+116),e ; write byte counter ready to exit
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
                            //  ; @tx IX+125/4 Param 1: pointer to first BCPL word
                            //  ; ret addr IX+123/2
                            //  ; old IX IX+121/0
                            //  ; vec ptr IX+119/8
                            //  ; f IX+117/6 local var init to 0 used to return result
  inline #xDD,#x66,#x7D     // LD         h,(ix+125) ; HL is ptr to TX BCPL buffer
  inline #xDD,#x6E,#x7C     // LD         l,(ix+124) 
  inline #x29               // ADD        hl,hl      ; double hl to convert BCPL int ptr to abs address
  inline #x1E,#x00          // LD         e,0        ; e = sent byte count
  inline #x01,#x81,#xFD     // LD         bc,0xfd81  
  inline #xDD,#x7E,#x7F     // LD         a,(ix+127) ; get high byte of count
  inline #xE6,#xFF          // AND        0xFF       ; check if non-zero
  inline #x28,#x08          // JR         z,next     ; get low byte if zero
  inline #xE6,#x80          // AND        0x80       ; check sign bit
  inline #x20,#x1E          // JR         nz,exit    ; exit if sign bit is set
  inline #x16,#xFF          // LD         d,255      ; else, +ve, non-zero hi byte so set size to 255 max
  inline #x18,#x08          // JR         top        
                            // NEXT:      ; 
  inline #xDD,#x7E,#x7E     // LD         a,(ix+126) ; Max bytes (1..255)
  inline #xE6,#xFF          // AND        0xff       ; check for zero and exit early
  inline #x28,#x13          // JR         Z,exit     
  inline #x57               // LD         d,a        ; d = max bytes
                            // TOP:       ; 
  inline #xED,#x78          // IN         a,(c)      ; get DIR status flag
  inline #xE6,#x02          // AND        0x2        
  inline #x28,#x0C          // JR         z,exit     ; go to end if no data available
  inline #x1C               // INC        e          ; inc byte count
  inline #x0D               // DEC        c          ; point to data reg
  inline #x7E               // LD         a,(hl)     
  inline #xED,#x79          // OUT        (c),a      
  inline #x23               // INC        hl         ; inc twice because BCPL data is stored in ints
  inline #x23               // INC        hl         ; and we are transferring only low byte
  inline #x0C               // INC        c          ; point to status reg for next check
  inline #x7B               // LD         a,e        ; transfer byte count to a
  inline #xBA               // CP         d          ; reached the max ?
  inline #x20,#xEE          // JR         nz,top     ; if not loop again
                            // EXIT:      ; 
  inline #xDD,#x73,#x74     // LD         (ix+116),e ; else write return value ready for exit

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
  LET t2,frac,sec = 0,0,0  

  mode(2)
  fifo_reset()
  writef("Setting up random data*n")
  setseed(1243)
  FOR k=0 TO SZ-1 DO $(
    tx!k := randno(256)-1      
    rx!k := 0
  $)

  i:=0
  j:=0
  writef("Sending/Receiving Data*n")
  resettime()
  WHILE (i+j) < SZ+SZ DO $(
    IF (i NE SZ) DO i:= i+ fifo_out_bytes( @(tx!i), SZ-i)
    IF (j NE SZ) DO j:= j+ fifo_in_bytes( @(rx!j), SZ-j )
  $)
  t := scaledtime(2)         // ask for time in 75th of sec.

  writef("Bytes sent: %n*n", i)
  writef("Bytes received: %n*n", j)
  sec := (t)/75              // seconds
  frac := (t - sec*75)*4/3   // convert remainder to 100ths 
  writef("Time: %N.", sec)
  writez(frac,2)
  writes("s*n")
  writef("Data rate: %n Bytes/s*n", 75*((SZ*2)/t))
  
  writef("Checking Data...*n")
  FOR i=0 TO SZ-1 DO $(
      IF rx!i NE tx!i DO e:=e+1
  $) 
  TEST e=0 THEN writef("No errors*n") ELSE writef("%n errors detected*n",e)
  
  presskeytoexit()
  RESULTIS 0
$)


