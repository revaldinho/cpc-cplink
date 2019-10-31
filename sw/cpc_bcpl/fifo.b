// CPC-Cplink FIFO Test Program
//
// (C) 2019 Revaldinho

GET "BCPLLIB.B"
GET "FIFOLIB.B"

MANIFEST $(
    SZ=4096       // num of ints
    DBL_SZ=SZ<<1  // num of bytes
    QUAD_SZ=SZ<<2 // double num bytes
$)

LET show_stats(dout,din, t) BE $(
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
  writef("Checking Data...")
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
    tx!k := randno(32767)-1
    rx!k := 0
  $)

  // Get pointer to absolute byte for transfer
  txbyteptr := (@(tx!0))<<1
  rxbyteptr := (@(rx!0))<<1

  writef("*nTest 1: Send/Receive 1 byte at a time, check FIFO status per byte*n")
  i:=0
  j:=0
  resettime()
  WHILE (i+j) < QUAD_SZ DO $(
    IF (i NE DBL_SZ) DO i:= i+ fifo_out_byte(tx%i) // note byte reference for transmission
    IF (j NE DBL_SZ) DO j:= j+ fifo_in_byte(rxbyteptr+j)
  $)
  t := scaledtime(2)         // ask for time in 75th of sec.

  show_stats(i,j,t)
  check_data( tx, rx, SZ)

  writef("*nTest 2: Send/Receive multiple bytes, check FIFO status per byte*n")
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

  writef("*nTest 3: Send/Receive 255 byte blocks, no status checks*n")
  i:=0
  j:=0
  resettime()
  WHILE i < DBL_SZ DO i:= i+ (fifo2_out_bytes_nc( txbyteptr+i, DBL_SZ-i) )
  WHILE j < DBL_SZ DO j:= j+ (fifo2_in_bytes_nc( rxbyteptr+j, DBL_SZ-j) )
  t := scaledtime(2)

  show_stats(i,j,t)
  check_data( tx, rx, SZ)

  writef("*nTest 4: Send/Receive 255 byte blocks, no status checks, 4 byte unrolled loops*n")
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
