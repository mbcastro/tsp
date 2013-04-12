cmake_minimum_required(VERSION 2.6)

project (TSP) 

add_subdirectory (utils)

add_subdirectory (x86) 

if (ARM)
add_subdirectory (arm)
endif (ARM)

if (MPPA)
add_subdirectory (mppa)
endif (MPPA)
