mkdir -p build/bin && cd build
cmake .. && make -j 16

mv stable-ui bin/