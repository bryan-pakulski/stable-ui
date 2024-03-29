#!/bin/bash
if [ ! -d "build" ]; then
    echo "build directory not present, please build the project with cmake first"
    exit 1
fi

DYNAMIC_LIBS=$1

mkdir -p build/stable-ui-bin

if [ -f build/stable-ui ]; then
  mv build/stable-ui build/stable-ui-bin/
fi

cp -ruv data build/stable-ui-bin/
cp -uv scripts/docker/start_docker.sh build/stable-ui-bin/
cp -uv data/config/imgui.ini build/stable-ui-bin/

if [[ $DYNAMIC_LIBS == "ON" ]]; then
  cp -uv lib/*.so build/stable-ui-bin
fi