SRCS = $(wildcard *.asc)

default: cpcasm.dsk

mcplib.bin: mcplib.s
	rasm mcplib.s -o mcplib

cpcasm.dsk: ${SRCS} mcplib.bin
	iDSK cpcasm.dsk -n
	for f in ${SRCS} ; do iDSK cpcasm.dsk -i $$f -t 0 ; done
	iDSK cpcasm.dsk -i mcplib.bin -t 1

all: cpcasm.dsk mcplib.bin

clean:
	rm mcplib.bin *dsk *~
