#!/bin/bash

echo "Reloading supervisord"
unlink /run/supervisor.sock
service supervisor stop

# Run supervisor (or restart if already running)
if pgrep -f "sd_model_server.py"; then
    echo "SD Model server already running, killing process"
    kill $(pgrep -f "sd_model_server.py")
fi

echo "Starting up supervisor..."
/usr/bin/supervisord