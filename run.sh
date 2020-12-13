#!/usr/bin/env bash

mkdir -p "build"
cd build
cmake ..
make

echo "Classic non vector"
./nonvector 33554432

echo "Standard Version 2 threads"
./projet 33554432 2
echo "Standard Version 4 threads"
./projet 33554432 4


echo "Mutex Version 2 threads"
./projetmutex 33554432 2
echo "Mutex Version 4 threads"
./projetmutex 33554432 4


