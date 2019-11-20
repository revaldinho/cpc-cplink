#!/bin/bash

echo "Build ping.bin"
cd ping
make
cd ..

echo "Sending ping.bin file to M4 on CPC"
../xfer -u 192.168.0.55 ping/obj/ping.bin / 2 0x4000 0x4000

echo "Tidy up build files for ping"
rm ping/*.cdt
rm ping/*.dsk
rm -r -f ping/obj/

echo "Build shutdown.bin"
cd shutdown
make
cd ..

echo "Sending shutdown.bin file to M4 on CPC"
../xfer -u 192.168.0.55 shutdown/obj/shutdown.bin / 2 0x4000 0x4000

echo "Tidy up build files for shutdown"
rm shutdown/*.cdt
rm shutdown/*.dsk
rm -r -f shutdown/obj/

echo "Build time.bin"
cd time
make
cd ..

echo "Sending time.bin file to M4 on CPC"
../xfer -u 192.168.0.55 time/obj/time.bin / 2 0x4000 0x4000

echo "Tidy up build files for time"
rm time/*.cdt
rm time/*.dsk
rm -r -f time/obj/

echo "Build date.bin"
cd date
make
cd ..

echo "Sending date.bin file to M4 on CPC"
../xfer -u 192.168.0.55 date/obj/date.bin / 2 0x4000 0x4000

echo "Tidy up build files for date"
rm date/*.cdt
rm date/*.dsk
rm -r -f date/obj/

echo "Build reset.bin"
cd reset
make
cd ..

echo "Sending reset.bin file to M4 on CPC"
../xfer -u 192.168.0.55 reset/obj/reset.bin / 2 0x4000 0x4000

echo "Tidy up build files for reset"
rm reset/*.cdt
rm reset/*.dsk
rm -r -f reset/obj/

