# Initialise docker container

if [ ! -d "data/docker" ]; then
    echo "docker data directory not present, please run this script from the root directory"
    exit 1
fi

pushd data/docker
docker build -t sd .

# Some distro requires that the absolute path is given when invoking lspci
# e.g. /sbin/lspci if the user is not root.
gpu=$(lspci | grep -i '.* vga .* nvidia .*')

shopt -s nocasematch

if [[ $gpu == *' nvidia '* ]]; then
  printf 'Nvidia GPU is present:  %s\n' "$gpu"
  docker compose --file docker-compose-nvidia.yml up -d
else
  printf 'Nvidia GPU is not present: %s\n' "$gpu"
  docker compose --file docker-compose-cpu.yml up -d
fi

docker exec sd /home/entrypoint.sh
popd