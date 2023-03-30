mkdir -p build
cd build && cmake -DBUILD_SHARED_LIBS=OFF ..
jinja > build.log