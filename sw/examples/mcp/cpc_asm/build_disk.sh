echo "Building commands.dsk in cpc_asm folder"

iDSK cpcasm.dsk -n
iDSK cpcasm.dsk -i mcplib.bin -t 1 -e 8000 -c 8000
iDSK cpcasm.dsk -i date.asc -t 0
iDSK cpcasm.dsk -i help.asc -t 0
iDSK cpcasm.dsk -i mcpterm.asc -t 0
iDSK cpcasm.dsk -i ping.asc -t 0
iDSK cpcasm.dsk -i reboot.asc -t 0
iDSK cpcasm.dsk -i reset.asc -t 0
iDSK cpcasm.dsk -i sexec.asc -t 0
iDSK cpcasm.dsk -i shutdown.asc -t 0
iDSK cpcasm.dsk -i time.asc -t 0
iDSK cpcasm.dsk -i tron.asc -t 0
iDSK cpcasm.dsk -i troff.asc -t 0
iDSK cpcasm.dsk -i version.asc -t 0
iDSK cpcasm.dsk -i wifiip.asc -t 0

echo "Done."
