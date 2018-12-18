#!/bin/bash

cd ./src/proto
protoc -I=./ --cpp_out=./ *.proto
cd ../..
mkdir -p build
cd build
cmake ..
make -j 4
cd ..
