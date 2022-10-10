cmake_minimum_required(VERSION 3.10)

project(sample VERSION 1.0)

if (WIN32)
    message(FATAL_ERROR "this library doesn't support windows")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)

add_subdirectory(source)

add_executable(sample sample.c)

target_link_libraries(sample MouseDetecter)

target_compile_options(sample PRIVATE -Wall -Wextra -pedantic-errors)