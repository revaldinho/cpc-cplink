#!/bin/bash

echo "Sending DATE.ASC file to M4 on CPC"
xfer -u 192.168.0.55 date.asc /
echo "Sending HELP.ASC file to M4 on CPC"
xfer -u 192.168.0.55 help.asc /
echo "Sending MCPTERM.ASC file to M4 on CPC"
xfer -u 192.168.0.55 mcpterm.asc /
echo "Sending PING.ASC file to M4 on CPC"
xfer -u 192.168.0.55 ping.asc /
echo "Sending REBOOT.ASC file to M4 on CPC"
xfer -u 192.168.0.55 reboot.asc /
echo "Sending RESET.ASC file to M4 on CPC"
xfer -u 192.168.0.55 reset.asc /
echo "Sending SEXEC.ASC file to M4 on CPC"
xfer -u 192.168.0.55 sexec.asc /
echo "Sending SHUTDOWN.ASC file to M4 on CPC"
xfer -u 192.168.0.55 shutdown.asc /
echo "Sending TIME.ASC file to M4 on CPC"
xfer -u 192.168.0.55 time.asc /
echo "Sending TRON.ASC file to M4 on CPC"
xfer -u 192.168.0.55 tron.asc /
echo "Sending TROFF.ASC file to M4 on CPC"
xfer -u 192.168.0.55 troff.asc /
echo "Sending VERSION.ASC file to M4 on CPC"
xfer -u 192.168.0.55 version.asc /
echo "Sending WIFIIP.ASC file to M4 on CPC"
xfer -u 192.168.0.55 wifiip.asc /
echo "Assemble MCPLIB.BIN using RASM"
rasm mcplib.s -o mcplib
echo "Sending MCPLIB.BIN file to M4 on CPC"
xfer -u 192.168.0.55 mcplib.bin /  2 0x8000 0x8000
