#!/bin/bash

# adobe XMP is a bit of a tricky one, quite a large library with no real easy way to strip out the extact things needed
# this script will build a static release of adobe XMP without SSP on linux, the files that need to be modifed are already saved 
# to be copied over the git repository when pulled

# This script should be run from the root directory

if [ ! -d "src" ]; then
    echo "please run this script from the root directory"
    exit 1
fi

mkdir -p lib
lib_output=$(pwd)/lib

pushd src/ThirdParty/XMP

if [ ! -d "XMP-Toolkit-SDK" ]; then
    # Setup base library
    git clone https://github.com/adobe/XMP-Toolkit-SDK

    # Copy and replace cmake configuration to allow us to build in non secure mode
    cp overrides/ProductConfig.cmake XMP-Toolkit-SDK/build/ProductConfig.cmake
    cp overrides/ToolchainGCC.cmake XMP-Toolkit-SDK/build/shared/ToolchainGCC.cmake

    # Setup zlib
    wget https://www.zlib.net/zlib-1.2.13.tar.gz
    # Extract and copy .c and .h files
    tar xzvf zlib-1.2.13.tar.gz
    cp zlib-1.2.13/*.c XMP-Toolkit-SDK/third-party/zlib
    cp zlib-1.2.13/*.h XMP-Toolkit-SDK/third-party/zlib
    rm -rf zlib-1.2.13.tar.gz zlib-1.2.13

    # Set up libexpat
    git clone https://github.com/libexpat/libexpat
    cp -r libexpat/expat/lib XMP-Toolkit-SDK/third-party/expat/
    rm -rf libexpat
fi

pushd XMP-Toolkit-SDK/build

# Get the list of GCC library paths and append the paths to the $LD_LIBRARY_PATH
gcc_libs=`gcc -print-search-dirs | grep "^libraries:" | cut -f2- -d':' | tr ':' '\n'`
for lib in $gcc_libs; do
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$lib"
done
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/libuuid.so

# Build
./cmake.command 64 Static ToolchainGCC.cmake
cmake --build gcc/static/i80386linux_64/Release

popd

# Copy static libs
cp XMP-Toolkit-SDK/public/libraries/i80386linux_x64/release/* $lib_output

popd