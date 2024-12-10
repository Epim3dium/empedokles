mkdir -p build
cd build
cmake -S ../ -B . -D CMAKE_BUILD_TYPE=$1
make && make Shaders && ./tests/tests && $2

cd ..

