#!/bin/bash

mkdir -p build
cd build
cmake ..
make -j 4
cd ..

find build/test -executable -type f -exec '{}' ';'
