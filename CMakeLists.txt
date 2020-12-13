cmake_minimum_required(VERSION 3.18)
project(projet C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-O1 -fno-tree-vectorize -lpthread -lm -mavx2")

add_executable(projet main.c)
add_executable(projetmutex mutex.c)
add_executable(projetnonvect nonvector.c)
# add_executable(projetunaligned unaligned.c)