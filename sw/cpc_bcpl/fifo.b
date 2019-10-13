// Straight port of fifo.bas into BCPL

GET "BCPLLIB.B"

MANIFEST $(
    SZ=4096
$)

LET fifo_dor() = VALOF $(
    LET f=0
    inline #x3E,#xFD            // LD  A,0xFD   
    inline #xDB,#x81            // IN  A,(0x81)   
    inline #xE6,#x01            // AND 0x01   
    inline #xDD,#x77,#x78       // LD  (IX+120),A   # store result in f
    inline #xDD,#x36,#x79,#x00  // LD  (IX+121),0   # zero upper byte
    RESULTIS f
$)

AND fifo_dir() = VALOF $(
    LET f=0
    inline #x3E,#xFD            // LD  A,0xFD   
    inline #xDB,#x81            // IN  A,(0x81)
    inline #xCB,#x2F            // SRA   A       
    inline #xE6,#x01            // AND 0x01
    inline #xDD,#x77,#x78       // LD  (IX+120),A   # store result in f
    inline #xDD,#x36,#x79,#x00  // LD  (IX+121),0   # zero upper byte
    RESULTIS f
$)

AND fifo_out(c) BE $(
    inline #xDD,#x7E,#x7E       // LD   A,(ix+126) - get low byte of param
    inline #x06,#xFD            // LD   B,0xFD
    inline #x0E,#x80            // LD   C,0x80 
    inline #xED,#x79            // OUT  (C),a   
$)

AND fifo_in() = VALOF $(
    LET f=99		
    inline #x3E,#xFD            // LD  A,0xFD   
    inline #xDB,#x80            // IN  A,(0x80)
    inline #xDD,#x77,#x78       // LD  (IX+120),A   # store result in f
    inline #xDD,#x36,#x79,#x00  // LD  (IX+121),0   # zero upper byte
    RESULTIS f
$)

AND fifo_reset() BE $(
    inline #x3E,#xFD             // LD   A,0xFD   
    inline #xD3,#x81             // OUT  (0x81),a   
$)

AND start() = VALOF $(
  LET rx = VEC SZ
  LET tx = VEC SZ
  LET p,i,j,t,e = 0,0,0,0,0
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
  WHILE (i+j) NE SZ*2 DO $(
    IF ((i NE SZ) & (fifo_dir()=1)) DO $(
       fifo_out(tx!i)
       i := i+1
    $)
    IF ((j NE SZ) & (fifo_dor()=1)) DO $(
       rx!j:=fifo_in()
       j := j+1
    $)
  $)
  t := scaledtime(2)         // ask for time in 75th of sec.

  writef("Bytes sent: %n*n", i)
  writef("Bytes received: %n*n", j)
  sec := (t)/75             // seconds
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


