@echo off

:: Ensure log files have correct permissions, create as current user if they don't exist
type nul >> data\logs\sd_server.log
type nul >> data\logs\docker_server.err
type nul >> data\logs\docker_server.out

pushd data\docker
echo Building docker image, this may take some time...

docker build -t sd .

echo Detecting Nvidia GPU...

set status=Does Not Exist

for /f "tokens=*" %%i in ('wmic path Win32_VideoController get description /format:list') do (
   set desc=%%i
   if "!desc!" == "Description=NVIDIA" set status=Exists
)

echo Nvidia GPU: %status%

if "%status%" == "Exists" (
  echo Nvidia GPU found.
  docker compose --file docker-compose-nvidia.yml up -d
) else (
  echo Nvidia GPU not found.
  docker compose --file docker-compose-cpu.yml up -d
)

docker exec sd /home/entrypoint.sh

popd
pause