cmake_minimum_required(VERSION 3.18)
project(projet C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "-pthread -mavx2 -lm -O3" )

add_executable(projet main.c)