#!/bin/bash
if [ ! -d "build" ]; then
    echo "build directory not present, please build the project with cmake first"
    exit 1
fi

mkdir -p build/stable-ui-bin

if [ -f build/stable-ui ]; then
  mv build/stable-ui build/stable-ui-bin/
fi

cp -ruv data build/stable-ui-bin/
cp -uv scripts/start_docker.sh build/stable-ui-bin/
cp -uv requirements.txt build/stable-ui-bin/
cp -uv src/imgui.ini build/stable-ui-bin/

cd build/stable-ui-bin
./start_docker.sh