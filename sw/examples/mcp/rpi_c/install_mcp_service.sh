#!/bin/bash

cd /home/pi/mcp_pi

echo "Compile MCP_pi"
make
echo "Copying MCP.Service to systemd"
chmod +x mcp.service
sudo chown root:root mcp.service
sudo mv mcp.service /lib/systemd/system
echo "Reload daemon"
sudo systemctl daemon-reload
echo "Enable Service"
sudo systemctl enable mcp.service
echo "Start Service"
sudo systemctl start mcp.service
sleep 5s
echo "Check the status of the mcp.service"
sudo systemctl status mcp.service
echo "Done."

