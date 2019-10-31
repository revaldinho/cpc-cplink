#!/bin/tcsh -f
set files = `ls -1 *lst`



cat > fifolib.b <<EOF 
// FIFOLIB.B
//
// A library of functions for the CPC-CPlink project.
// 
// Copyright (C) 2019 Revaldinho
LET fifo_reset() BE \$(
    inline #x3E,#xFD             // LD   A,0xFD   
    inline #xD3,#x81             // OUT  (0x81),a   
\$)


EOF

set libname = fifolib.b

foreach f ( $files[*] )
    echo "Processing $f "
    set name = $f:r
    if ($name == fifo_in_byte || $name == fifo_out_byte) then
        echo "AND " $name' (ptr) = VALOF $(' >> $libname    
    else
        echo "AND " $name' (ptr, max) = VALOF $(' >> $libname
    endif
    echo '  LET f=0' >> $libname
    python ../../util/lst2inline.py -f $f  >> $libname
    echo "  RESULTIS f" >> $libname
    echo '$)'     >> $libname
    echo ''     >> $libname    
end

mv fifolib.b ..
