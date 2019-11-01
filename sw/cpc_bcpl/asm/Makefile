#
# Make process requires z80asm to be installed from
#
# http://savannah.nongnu.org/projects/z80asm
#
ALLSRCS   ?= $(wildcard *.asm)
ALL_LST   ?= $(patsubst %.asm,%.lst,$(ALLSRCS))
ALL_B   ?= $(patsubst %.asm,%.b,$(ALLSRCS))



%.lst : %.asm
	z80asm $< -l >& $@
	rm a.bin


%.b : %.lst
	python ../../util/lst2inline.py -f $<  > $@

fifolib.b:  ${ALL_LST} ${ALL_B}
	/bin/tcsh  lst2bcpl.csh
	cp fifolib.b ..

all:   ${ALL_LST} ${ALL_B} fifolib.b 

clean:
	rm -rf *.b *.lst *~  a.bin




