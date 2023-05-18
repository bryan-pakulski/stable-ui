@echo off
if not exist "build" (
    echo "build directory not present, please build the project with cmake first"
    exit 1
)

mkdir build\stable-ui-bin
mkdir build\stable-ui-bin\data

:: Data
xcopy /s /e /q data build\stable-ui-bin\data

:: Top level
move build\Release\stable-ui.exe build\stable-ui-bin
copy src\imgui.ini build\stable-ui-bin
copy scripts\docker\start_docker.bat build\stable-ui-bin
copy scripts\win\stable-ui.bat build\stable-ui-bin