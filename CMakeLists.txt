cmake_minimum_required(VERSION 2.6)

project (TSP) 

if (x86)
add_subdirectory (utils)
add_subdirectory (x86) 
endif (x86)

if (ARM)
add_subdirectory (arm)
endif (ARM)

if (MPPA)
add_subdirectory (mppa)
endif (MPPA)
