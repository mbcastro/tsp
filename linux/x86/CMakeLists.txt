cmake_minimum_required(VERSION 2.6)

project(TSP)

# bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/linux/x86/bin)

# source files
file(GLOB source_files_pthreads "${CMAKE_BINARY_DIR}/src/*" "${CMAKE_BINARY_DIR}/linux/src/*" "${CMAKE_BINARY_DIR}/linux/src/pthreads/*")
file(GLOB source_files_trace "${CMAKE_BINARY_DIR}/linux/src/trace/*")
# file(GLOB source_files_mpi "${CMAKE_BINARY_DIR}/src/*" "${CMAKE_BINARY_DIR}/linux/src/*" "${CMAKE_BINARY_DIR}/linux/src/mpi/*")

include_directories(${CMAKE_BINARY_DIR}/linux/src/ ${CMAKE_BINARY_DIR}/src )

# specify the cross compiler
# set(CMAKE_C_COMPILER mpicc)

# linker
link_libraries("-lm -lpthread")

# set general compilation flags
set (CMAKE_C_FLAGS "-Wall -O3")

#Gets the page size for the current architecture
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPAGE_SIZE=`${CMAKE_BINARY_DIR}/utils/bin/getpagesize`")


# add debug flag if needed (example: cmake -DDEBUG:STRING=1 .)
if (DEBUG) 
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG")
endif(DEBUG)

# default srand()? (example: cmake -DSRAND:STRING=1 .)
if (SRAND) 
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEF_RAND")
endif(SRAND)

#Should tracing be enabled?
if (TRACE_ENABLED) 
	set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DTRACE_ENABLED")
endif(TRACE_ENABLED)


# sequential version
add_executable(tsp_seq ${source_files_pthreads})

# lock-based version
add_executable(tsp_lock ${source_files_pthreads})
set_target_properties(tsp_lock PROPERTIES COMPILE_FLAGS "-DMT")

# trace converter
# add_executable(trace_converter ${source_files_trace})
# TARGET_LINK_LIBRARIES(trace_converter poti)

# mpi lock-based version
# add_executable(tsp_mpi ${source_files_mpi})
# set_target_properties(tsp_mpi PROPERTIES COMPILE_FLAGS "-DMT")

# cas-based version
#add_executable(tsp_cas ${source_files_pthreads})
#set_target_properties(tsp_cas PROPERTIES COMPILE_FLAGS "-DMT -DCAS")
