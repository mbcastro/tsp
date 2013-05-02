cmake_minimum_required(VERSION 2.6)

project (TSP) 

if (x86 OR ARM)
add_subdirectory (utils)
add_subdirectory (linux) 
endif (x86 OR ARM)

if (MPPA)
add_subdirectory (mppa)
endif (MPPA)
