#!/usr/bin/env bash

mkdir -p "build"
cd build

gcc ../main.c -O1 -fno-tree-vectorize -lpthread -lm -mavx -o projet
gcc ../mutex.c -O1 -fno-tree-vectorize -lpthread -lm -mavx -o projetmutex
gcc ../nonvector.c -O1 -fno-tree-vectorize -lpthread -lm -o nonvector

# 2^20 elts, on 2 threads

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

