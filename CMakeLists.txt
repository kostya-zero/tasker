cmake_minimum_required(VERSION 3.31)
project(tasker C)

set(CMAKE_C_STANDARD 17)

file(GLOB SRC_FILES src/*.c)

add_executable(tasker ${SRC_FILES})
