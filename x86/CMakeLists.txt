cmake_minimum_required(VERSION 2.6)

project(TSP)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

link_libraries("-lm -lpthread")

include_directories(include)

file(GLOB source_files "src/*")


set (CMAKE_C_FLAGS "-g -Wall -O3")
#if (DEBUG) 
#	set_target_properties (CMAKE_C_FLAGS_PREFIX "${CMAKE_C_FLAGS_PREFIX} -DDEBUG")
#endif(DEBUG)

#seq
add_executable(tsp_seq ${source_files})

#lock
add_executable(tsp_lock ${source_files})
set_target_properties(tsp_lock PROPERTIES COMPILE_DEFINITIONS "MT")

#cas
add_executable(tsp_cas ${source_files})
set_target_properties(tsp_cas PROPERTIES COMPILE_DEFINITIONS "MT" COMPILE_DEFINITIONS "CAS")
#set_target_properties(tsp_cas PROPERTIES COMPILE_DEFINITIONS "CAS")