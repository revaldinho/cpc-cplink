
fiforsx.bin: fiforsx.s
	cpp  -E fiforsx.s | grep -v '^#'> tmp.s
	rasm tmp.s -o fiforsx -s

fiforsx.rom: fiforsx.s
	cpp -DROM -E fiforsx.s | grep -v '^#'> tmp.s
	rasm tmp.s -o fiforsxrom -s
	truncate -s 16384 fiforsxrom.bin
	mv fiforsxrom.bin fiforsx.rom


all: fiforsx.bin fiforsx.rom

clean:
	rm *.bin *~
