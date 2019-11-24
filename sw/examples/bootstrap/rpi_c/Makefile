# To be run on Raspberry Pi
bootstrap: bootstrap.c
	cc bootstrap.c -o bootstrap -lwiringPi

bootloader.bin: bootloader.s
	z80asm bootloader.s -o bootloader.bin

DEF2_NO_HDR.bin: DEFEND2.BIN
	tail -c +129 DEFEND2.BIN > DEF2_NO_HDR.bin


all: bootstrap bootloader.bin


clean:
	rm DEF2_NO_HDR.bin bootloader.bin bootstrap

