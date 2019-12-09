#!/bin/bash

echo "Sending ping.asc file to M4 on CPC"
xfer -u 192.168.0.55 ping.asc /
echo "Sending shutdown.asc file to M4 on CPC"
xfer -u 192.168.0.55 shutdown.asc /
echo "Sending time.asc file to M4 on CPC"
xfer -u 192.168.0.55 time.asc /
echo "Sending date.asc file to M4 on CPC"
xfer -u 192.168.0.55 date.asc /
echo "Sending reset.asc file to M4 on CPC"
xfer -u 192.168.0.55 reset.asc /
echo "Sending help.asc file to M4 on CPC"
xfer -u 192.168.0.55 help.asc /
echo "Sending reboot.asc file to M4 on CPC"
xfer -u 192.168.0.55 reboot.asc /
echo "Sending mcpterm.asc file to M4 on CPC"
xfer -u 192.168.0.55 mcpterm.asc /
echo "Sending tron.asc file to M4 on CPC"
xfer -u 192.168.0.55 tron.asc /
echo "Sending troff.asc file to M4 on CPC"
xfer -u 192.168.0.55 troff.asc /

