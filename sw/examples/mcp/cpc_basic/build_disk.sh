echo "Building commands.dsk in cpc_basic folder"

iDSK cpcbasic.dsk -n
iDSK cpcbasic.dsk -i date.asc -t 0
iDSK cpcbasic.dsk -i help.asc -t 0
iDSK cpcbasic.dsk -i mcpterm.asc -t 0
iDSK cpcbasic.dsk -i ping.asc -t 0
iDSK cpcbasic.dsk -i reboot.asc -t 0
iDSK cpcbasic.dsk -i reset.asc -t 0
iDSK cpcbasic.dsk -i sexec.asc -t 0
iDSK cpcbasic.dsk -i shutdown.asc -t 0
iDSK cpcbasic.dsk -i time.asc -t 0
iDSK cpcbasic.dsk -i tron.asc -t 0
iDSK cpcbasic.dsk -i troff.asc -t 0
iDSK cpcbasic.dsk -i version.asc -t 0
iDSK cpcbasic.dsk -i wifiip.asc -t 0

echo "Done."
