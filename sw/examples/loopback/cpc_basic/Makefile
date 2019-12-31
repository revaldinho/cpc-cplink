SRCS = $(wildcard *.asc)
VPATH=../../../commonlib/cpc_asm

default: loopback.dsk

fiforsx.bin: fiforsx.s 
	(cd ${VPATH} ; make all ) 

loopback.dsk: ${SRCS} fiforsx.bin
	iDSK loopback.dsk -n
	for f in ${SRCS} ; do iDSK loopback.dsk -i $$f -t 0 ; done
	iDSK loopback.dsk -i ${VPATH}/fiforsx.bin -t 1 

all: loopback.dsk

clean:
	rm loopback.dsk *~ *bin
