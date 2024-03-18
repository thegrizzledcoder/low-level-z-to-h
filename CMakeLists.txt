cmake_minimum_required(VERSION 3.28)
project(employees C)

set(CMAKE_C_STANDARD 11)

add_executable(employees src/main.c
        src/common.h
        src/file.h
        src/parse.h
        src/file.c
        src/parse.c)
