
fiforsx.bin: fiforsx.s
	rasm  fiforsx.s -o fiforsx -s

fiforsx.rom: fiforsx.s
	rasm -DROM=1  fiforsx.s -o fiforsxrom -s
	truncate -s 16384 fiforsxrom.bin
	mv fiforsxrom.bin fiforsx.rom


all: fiforsx.bin fiforsx.rom

clean:
	rm *.bin *~
