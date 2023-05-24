@echo off

if not exist "src\" (
    echo please run this script from the root directory
    exit 1
)

mkdir lib
set "lib_output=%CD%\lib"

cd src\ThirdParty\XMP

if not exist "XMP-Toolkit-SDK\" (
    :: Setup base library
    git clone https://github.com/adobe/XMP-Toolkit-SDK

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

cd XMP-Toolkit-SDK\build


Echo Building SDK
echo "Generating XMPSDKToolkit Dynamic x64"
set NEXT_LABEL=ok
set VS_VERSION=2019
set BUILD_TYPE=Dynamic
set BITS=64
IF "%GENERATE_ALL%"=="On" (
	set NEXT_LABEL=ok
)

call cmake_all.bat %BITS% %VS_VERSION% WarningAsError %BUILD_TYPE%
if errorlevel 1 goto error
goto %NEXT_LABEL%

:error
echo CMake Build Failed.
pause
exit /B 1

:ok
echo CMake Build Success.
pause
exit /B 0

cd ..\..\..

:: Copy static libs
