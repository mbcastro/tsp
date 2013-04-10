cmake_minimum_required(VERSION 2.6)

project(utils)

# bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/utils/bin)

# source files
file(GLOB source_files "src/*")

add_executable(getpagesize ${source_files})
