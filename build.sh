mkdir -p build/bin && cd build
cmake .. && make -j 16

mv stable-ui bin/
cp -r ../data bin/
cp ../run.sh bin/