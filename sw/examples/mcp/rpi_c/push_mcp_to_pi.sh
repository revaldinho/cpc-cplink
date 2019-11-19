#!/bin/bash

echo "Remember to modify this file with YOUR pi user name and ip address!"

cd mcp_pi
echo "Removing Build files"
rm *.o
rm mcp_pc
echo "Sending MCP to PI"
scp -r * pi@192.168.1.107:/home/pi/mcp_pi
echo "Finished."

