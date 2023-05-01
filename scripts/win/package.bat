@echo off
if not exist "build" (
    echo "build directory not present, please build the project with cmake first"
    exit 1
)

mkdir build\stable-ui-bin

if exist build\stable-ui (
    move build\stable-ui build\stable-ui-bin
)

xcopy /s /e /q data build\stable-ui-bin
copy scripts\win\start_docker.bat build\stable-ui-bin
copy requirements.txt build\stable-ui-bin
copy src\imgui.ini build\stable-ui-bin