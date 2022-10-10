cmake_minimum_required(VERSION 3.10)

project(MouseDetecter VERSION 1.0)

if (WIN32)
    message(FATAL_ERROR "this library doesn't support windows")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_library(MouseDetecter STATIC MouseDetecter.c)

target_compile_options(MouseDetecter PRIVATE -Wall -Wextra -pedantic-errors)