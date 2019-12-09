#!/bin/bash

echo "Remember to modify this file with YOUR pi user name and ip address!"

echo "Removing Build files"
rm *.o
rm mcp_pc
echo "Sending MCP go PI"
scp -r ../../../* pi@192.168.0.189:/home/pi/sw/
echo "Finished."

