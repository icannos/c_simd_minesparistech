cmake_minimum_required(VERSION 3.18)
project(projet C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-O1 -fno-tree-vectorize -lpthread -lm -mavx")

add_executable(projet main.c)
add_executable(projetmutex mutex.c)
add_executable(projetnonvect nonvector.c)
# add_executable(projetunaligned unaligned.c)

find_package(Threads REQUIRED)
target_link_libraries(projet PRIVATE Threads::Threads)