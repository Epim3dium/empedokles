#!/usr/bin/env bash
mkdir -p build
cd build
cmake -S ../ -B . -D CMAKE_BUILD_TYPE=$1 -G Ninja
cmake --build . -j && ./tests/tests && $2

cd ..

