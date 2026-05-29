#!/bin/bash

# shutdown_pi.sh

echo "System will shutdown in 60 seconds..."

# Flush filesystem buffers
sync

# Schedule shutdown after 60 seconds
/usr/sbin/shutdown -h +1 "System is down"
