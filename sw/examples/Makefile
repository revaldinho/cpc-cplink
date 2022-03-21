# Make a dsk image with all the CPC BASIC and BIN files
#

VPATH=../commonlib/cpc_asm
SRC1 = $(wildcard loopback/cpc_basic/*.asc)
SRC2 = $(wildcard vdu/cpc_basic/*.asc)

default: cplink.dsk

fiforsx.bin: fiforsx.s
	(cd ${VPATH} ; make all )

fiforsx.rom: fiforsx.s
	(cd ${VPATH} ; make all )

cplink.dsk: ${SRC1} ${SRC2} fiforsx.bin fiforsx.rom
	iDSK cplink.dsk -n
	for f in ${SRC1} ; do iDSK cplink.dsk -i $$f -t 0 ; done
	for f in ${SRC2} ; do iDSK cplink.dsk -i $$f -t 0 ; done
	iDSK cplink.dsk -i ${VPATH}/fiforsx.bin -t 1
	iDSK cplink.dsk -i ${VPATH}/fiforsx.rom -t 1

all: cplink.dsk

clean:
	rm cplink.dsk *~ *bin
