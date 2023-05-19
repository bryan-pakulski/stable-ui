@echo off
if not exist "build" (
	echo "Please please build & package the project first"
    exit 1
)

pushd build\stable-ui-bin
START stable-ui.exe
popd