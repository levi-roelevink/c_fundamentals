cmake_minimum_required(VERSION 4.0)
project(c_fundamentals C)

set(CMAKE_C_STANDARD 11)

add_executable(c_fundamentals main.c
        file_open.c
        network_io.c)
