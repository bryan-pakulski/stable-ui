#!/bin/bash

# Initialise docker container
pushd data/docker
docker build -t sd .
docker compose up -d
docker exec sd /home/entrypoint.sh
docker exec sd pip install -e /home/
popd

# Run application
./stable-ui