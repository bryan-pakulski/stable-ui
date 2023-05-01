#!/bin/bash
if [ ! -d "build" ]; then
    echo "build directory not present, please build the project with cmake first"
    exit 1
fi

mkdir -p build/stable-ui-bin

if [ -f build/stable-ui ]; then
  mv build/libimgui.a build/stable-ui-bin/
  mv build/stable-ui build/stable-ui-bin/
fi

cp -ruv data build/stable-ui-bin/
cp -uv scripts/lin/start_docker.sh build/stable-ui-bin/
cp -uv requirements.txt build/stable-ui-bin/
cp -uv src/imgui.ini build/stable-ui-bin/

tar -zcvf stable-ui.tar.gz build/stable-ui-bin