# Initialise docker container
pushd data/docker
docker build -t sd .
docker compose up -d
docker exec sd /home/entrypoint.sh
popd