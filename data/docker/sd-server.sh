#!/bin/bash
# This script will actually run the the sd server using numactl to pin across all available cpu cores
umask 0000
cores=$(expr $(nproc) - 1)
echo "Starting sd_model_server with cores 0-${cores}"
conda run -n ldm numactl -C 0-${cores} python '/modules/stable-ui/sd_model_server.py'