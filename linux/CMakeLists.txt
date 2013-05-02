cmake_minimum_required(VERSION 2.6)

project (TSP) 

if (x86)
add_subdirectory (x86) 
endif (x86)

if (ARM)
add_subdirectory (arm)
endif (ARM)
