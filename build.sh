# Retrieve third party submodules & any updates
git submodule update --init --recursive

mkdir -p build/bin && cd build
cmake .. -G 'Unix Makefiles' && make -j 16 

mv stable-ui bin/
cp -ruv ../data bin/
cp ../start_docker.sh bin/
cp ../requirements.txt bin/