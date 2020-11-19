#!/usr/bin/env bash

mkdir -p "build"
cd build
cmake ..
make

# 2^20 elts, on 2 threads
./projet 1048576 2
