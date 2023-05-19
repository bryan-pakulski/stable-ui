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

if "%1"=="-r" (
	cmake --build . --config Release
)
else (
	cmake --build . --config Debug
)
popd

CALL .\scripts\pipeline\package.bat