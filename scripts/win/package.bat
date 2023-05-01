@echo off
if not exist "build" (
    echo "build directory not present, please build the project with cmake first"
    exit 1
)

mkdir build\stable-ui-bin
mkdir build\stable-ui-bin\lib
mkdir build\stable-ui-bin\data

:: Libraries
powershell -Command "Invoke-WebRequest https://www.python.org/ftp/python/3.8.10/python-3.8.10-embed-amd64.zip -OutFile python38.zip"
powershell -Command "Expand-Archive -Path python38.zip -DestinationPath build\stable-ui-bin\lib"

:: Data
xcopy /s /e /q data build\stable-ui-bin\data

:: Top level
move build\Release\stable-ui.exe build\stable-ui-bin
copy src\imgui.ini build\stable-ui-bin
copy scripts\win\stable-ui.bat build\stable-ui-bin
copy scripts\win\start_docker.bat build\stable-ui-bin
copy scripts\win\stable-ui.bat build\stable-ui-bin
copy requirements.txt build\stable-ui-bin