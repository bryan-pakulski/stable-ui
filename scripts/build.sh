#!/bin/bash

if [ ! -d "src" ]; then
    echo "please run this script from the root directory"
    exit 1
fi

export DYNAMIC_LIBS=OFF

echo "Building ThirdParty libs..."
./src/ThirdParty/XMP/build_xmp_libraries.sh $DYNAMIC_LIBS


echo "Building stable-ui..."

mkdir -p build
pushd build
    echo "dynamic libraries ${DYNAMIC_LIBS}"
    if [[ $1 == "-r" ]]; then
        echo "Running in release mode..."
        cmake .. -DBUILD_SHARED_LIBS=$DYNAMIC_LIBS -DCMAKE_BUILD_TYPE=Release
    else
        echo "Running in debug mode..."
        cmake .. -DBUILD_SHARED_LIBS=$DYNAMIC_LIBS -DCMAKE_BUILD_TYPE=Debug
    fi

    cmake --build . -- -j $(nproc)
popd

echo "Packing docker container..."
bash -c "./scripts/pipeline/package.sh" $DYNAMIC_LIBS