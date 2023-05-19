@echo off
if not exist "build" (
    echo "build directory not present, please build the project with cmake first"
    exit 1
)

mkdir build\stable-ui-bin
mkdir build\stable-ui-bin\data

:: Data
xcopy /Y /s /e /q data build\stable-ui-bin\data

:: Top level
move /Y build\Release\* build\stable-ui-bin
copy /Y src\imgui.ini build\stable-ui-bin
copy /Y scripts\docker\start_docker.bat build\stable-ui-bin