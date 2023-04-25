#!/bin/bash

if [ ! -d "src" ]; then
    echo "please run this script from the root directory"
    exit 1
fi

echo "Running build..."

mkdir -p build
pushd build
    cmake -DBUILD_SHARED_LIBS=OFF ..
    jinja > build.log
popd

echo "Packing docker container..."
bash -c "./scripts/lin/package.sh"

echo "Starting application..."
bash -c "./scripts/lin/run.sh"

