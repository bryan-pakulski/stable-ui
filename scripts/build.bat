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

echo "Building ThirdParty libs..."
call .\src\ThirdParty\XMP\build_xmp_libraries.bat > build\XMP_build.log

pushd build
cmake -B . -S .. > build.log

if "%1"=="-r" (
	cmake --build . --config Release >> build.log
) else (
	cmake --build . --config Debug >> build.log
)
popd

if "%1"=="-r" (
	CALL .\scripts\pipeline\package.bat -r
) else (
	CALL .\scripts\pipeline\package.bat
)