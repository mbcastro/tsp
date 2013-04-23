cmake_minimum_required(VERSION 2.6)

project(TSP)

# bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/arm/bin)

# specify the cross compiler
set(CMAKE_C_COMPILER arm-linux-gnueabi-gcc)

# source files
file(GLOB source_files "${CMAKE_BINARY_DIR}/src/*" "${CMAKE_BINARY_DIR}/arm/src/*")

include_directories(${CMAKE_BINARY_DIR}/src)

# linker
link_libraries("-lm -lpthread")

# set general compilation flags
set (CMAKE_C_FLAGS "-Wall -O3")

#Gets the page size for the current architecture
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPAGE_SIZE=4096")


# add debug flag if needed (example: cmake -DDEBUG:STRING=1 .)
if (DEBUG) 
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
endif(DEBUG)

# default srand()? (example: cmake -DSRAND:STRING=1 .)
if (SRAND) 
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEF_RAND")
endif(SRAND)

# sequential version
add_executable(tsp_seq_arm ${source_files})

# lock-based version
add_executable(tsp_lock_arm ${source_files})
set_target_properties(tsp_lock_arm PROPERTIES COMPILE_FLAGS "-DMT")

# cas-based version
add_executable(tsp_cas_arm ${source_files})
set_target_properties(tsp_cas_arm PROPERTIES COMPILE_FLAGS "-DMT -DCAS")
