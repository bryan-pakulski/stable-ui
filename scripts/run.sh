#!/bin/bash

if [ ! -d "build/stable-ui-bin" ]; then
    echo "build/stable-ui-bin directory not present, please build & package the project first"
    exit 1
fi

cd build/stable-ui-bin
./start_docker.sh &
./stable-ui