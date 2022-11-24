mkdir -p build/bin && cd build
cmake .. && make -j 16 

mv stable-ui bin/
cp -ruv ../data bin/
cp ../start_docker.sh bin/
cp ../requirements.txt bin/