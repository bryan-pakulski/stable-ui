@echo off

if not exist "src\" (
    echo please run this script from the root directory
    exit 1
)

mkdir lib
set "lib_output=%CD%\lib"

pushd src\ThirdParty\XMP

if not exist "XMP-Toolkit-SDK\" (
    :: Setup base library
    git clone https://github.com/adobe/XMP-Toolkit-SDK

    :: Copy overrides
    copy overrides\CMakeUtils.bat XMP-Toolkit-SDK\build\shared\CMakeUtils.bat

    :: Setup zlib
    powershell -Command "(New-Object System.Net.WebClient).DownloadFile('https://www.zlib.net/zlib-1.2.13.tar.gz', 'zlib-1.2.13.tar.gz')"
    :: Extract and copy .c and .h files
    tar -xzvf zlib-1.2.13.tar.gz
    move zlib-1.2.13\*.c XMP-Toolkit-SDK\third-party\zlib
    move zlib-1.2.13\*.h XMP-Toolkit-SDK\third-party\zlib
    del zlib-1.2.13.tar.gz
    rmdir zlib-1.2.13 /s /q

    :: Set up libexpat
    git clone https://github.com/libexpat/libexpat
    move libexpat\expat\lib XMP-Toolkit-SDK\third-party\expat\
    rmdir libexpat /s /q
)

pushd XMP-Toolkit-SDK\build
echo "Generating XMPSDKToolkit Dynamic x64"
call cmake_all.bat 64 2022 Dynamic
if errorlevel 1 goto error
goto build

:build
echo "Building XMP toolkit" 
pushd vc16\dynamic\windows_x64\
cmake --build . --config Release
popd
popd
:: Copy libs
copy /Y XMP-Toolkit-SDK\public\libraries\windows_x64\Release\*.lib %lib_output%
copy /Y XMP-Toolkit-SDK\public\libraries\windows_x64\Release\*.dll %lib_output%
popd
exit /B 0

:error
echo Failed %PROJECT% build cmake.
echo "Exiting cmake.bat"
exit /B 1