mkdir -p build
cd build && cmake -DBUILD_SHARED_LIBS=OFF ..
make -j $(nproc) > build.log