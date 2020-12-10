#!/usr/bin/env bash

mkdir -p "build"
cd build
cmake ..
make

# 2^20 elts, on 2 threads

echo "## Standard Version"
./projet 1048576 2
echo "## Mutex Version"
./projetmutex 1048576 2
