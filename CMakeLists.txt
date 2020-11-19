cmake_minimum_required(VERSION 3.18)
project(projet C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-pthread -mavx2 -lm -O0" )

add_executable(projet main.c)
add_executable(projetmutex mutex.c)

find_package(Threads REQUIRED)
target_link_libraries(projet PRIVATE Threads::Threads)