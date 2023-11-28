#!/bin/bash

# adobe XMP is a bit of a tricky one, quite a large library with no real easy way to strip out the extact things needed
# this script will build a static release of adobe XMP without SSP on linux, the files that need to be modifed are already saved 
# to be copied over the git repository when pulled

# This script should be run from the root directory

if [ ! -d "src" ]; then
    echo "please run this script from the root directory"
    exit 1
fi

DYNAMIC_LIBS=$1
mkdir -p lib
lib_output=$(pwd)/lib

pushd src/ThirdParty/XMP

if [ ! -d "XMP-Toolkit-SDK" ]; then
    # Setup base library, we are using v2023.07
    git clone --depth 1 --branch v2023.07 https://github.com/adobe/XMP-Toolkit-SDK

    # Setup zlib
    wget https://www.zlib.net/current/zlib.tar.gz
    mkdir -p zlib
    # Extract and copy .c and .h files
    tar xzvf zlib.tar.gz -C zlib --strip-components=1
    cp zlib/*.c XMP-Toolkit-SDK/third-party/zlib
    cp zlib/*.h XMP-Toolkit-SDK/third-party/zlib
    rm -rf zlib*

    # Set up libexpat
    git clone --depth 1 --branch R_2_5_0 https://github.com/libexpat/libexpat
    mkdir -p XMP-Toolkit-SDK/third-party/expat
    cp -r libexpat/expat/lib XMP-Toolkit-SDK/third-party/expat/
    rm -rf libexpat
fi

# Copy and replace cmake configuration to allow us to build in non secure mode
cp overrides/ProductConfig.cmake XMP-Toolkit-SDK/build/ProductConfig.cmake
cp overrides/ToolchainGCC.cmake XMP-Toolkit-SDK/build/shared/ToolchainGCC.cmake

pushd XMP-Toolkit-SDK/build

# Get the list of GCC library paths and append the paths to the $LD_LIBRARY_PATH
gcc_libs=`gcc -print-search-dirs | grep "^libraries:" | cut -f2- -d':' | tr ':' '\n'`
for lib in $gcc_libs; do
    export LD_LIBRARY_PATH="$LD_LIBRARY_PATH:$lib"
done
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/lib/libuuid.so

# Build
echo "Dynamic Libs: $DYNAMIC_LIBS"
if [[ $DYNAMIC_LIBS == "ON" ]]; then
    ./cmake.command 64 Dynamic ToolchainGCC.cmake
    cmake --build gcc/dynamic/i80386linux_64/Release
else
    ./cmake.command 64 Static ToolchainGCC.cmake
    cmake --build gcc/static/i80386linux_64/Release
fi

popd

# Copy libs
cp XMP-Toolkit-SDK/public/libraries/i80386linux_x64/release/* $lib_output

popd