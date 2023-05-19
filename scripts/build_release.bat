@echo off
if not exist "src" (
	echo "Please run this script from the root directory" 
	exit 1
)
if not exist "build" (
	mkdir "build"
)

set vcpkg_location=C:/vcpkg
%vcpkg_location%/vcpkg install zeromq --triplet x64-windows

pushd build

cmake -B . -S .. -DCMAKE_TOOLCHAIN_FILE=%vcpkg_location%/scripts/buildsystems/vcpkg.cmake
cmake --build . --config Release

popd

CALL .\scripts\pipeline\package.bat