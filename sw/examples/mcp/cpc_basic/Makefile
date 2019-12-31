SRCS = $(wildcard *.asc)

command.dsk: ${SRCS}
	iDSK command.dsk -n
	for f in ${SRCS} ; do iDSK command.dsk -i $$f -t 0 ; done

all: command.dsk

clean:
	rm command.dsk *~
