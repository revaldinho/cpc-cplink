GET "BCPLLIB.B"

MANIFEST $(
    SZ=4096
$)

LET fifo_in(p) = VALOF $(
    LET f=0
    LET r=#x0A
    
    inline #x01,#x81,#xFD       // LD   bc,0xfd81   # status reg
    inline #xED,#x78            // IN   a,(c)       
    inline #xE6,#x01            // AND  0x1   
    inline #x28,#x09            // JR   z,end2      # exit if zero
    inline #xDD,#x77,#x76       // LD   (ix+118),a  # save DOR state to f low byte 
    inline #x0D                 // DEC  c           # point to data reg
    inline #xED,#x78            // IN   a,(c)       # a <- fifo input
    inline #xDD,#x77,#x74       // LD   (ix+116), a # r (low) <- a
                                // END2:            
    !p:=r
    RESULTIS f                               
$)


AND fifo_out(c) = VALOF $(
    // Check DIR, return 0 if not ready else write byte and return 1
    // Parameters on entry exit
    // c        [hi] <- IX+127 (last parameter @ IX+127)
    // c        [lo] 
    // ret addr [hi] <- IX+125
    // ret addr [lo] <- IX+124
    // old IX   [hi] <- IX+123
    // old IX   [lo] <- IX+122
    // vec ptr  [hi] <- IX+121
    // vec ptr  [lo] <- IX+120
    // f        [hi] <- IX+119, SP points here (first local var)
    // f        [lo] <- IX+118
    LET f=0
    
    inline #x01,#x81,#xFD      //   LD   BC,0xFD81   # status reg
    inline #xED,#x78           //   IN   A,(C)       
    inline #xE6,#x02           //   AND   0x02   
    inline #x28,#x0A           //   JR   Z,END       # skip to end if zero
    inline #xDD,#x36,#x76,#x01 //   LD   (IX+118),1  # save DIR state in f low byte 
    inline #xDD,#x7E,#x7E      //   LD   A,(ix+126)  # get parameter low byte
    inline #x0D                //   DEC   C          # point to data reg
    inline #xED,#x79           //   OUT   (C),a      # write the byte
                               // END:        
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
  FOR k=0 TO SZ-1 DO tx!k := randno(256)-1      

  i:=0
  j:=0

  writef("Sending/Receiving Data*n")
  resettime()
  WHILE (i+j) < SZ+SZ DO $(
    IF (i NE SZ) DO i:= i+ fifo_out(tx!i)
    IF (j NE SZ) DO j:= j+ fifo_in( @(rx!j) )
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

