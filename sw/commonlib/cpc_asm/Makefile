
ROMflag ?=

fiforsx.bin: fiforsx.s
	cpp ${ROMflag} -E fiforsx.s | grep -v '^#'> tmp.s
	rasm tmp.s -o fiforsx -s


all: fiforsx.bin

clean:
	rm *.bin *~
