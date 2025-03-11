mkdir -p build
cd build
cmake -S ../ -B . -D CMAKE_BUILD_TYPE=$1
make && make Shaders && $2
cd ..

