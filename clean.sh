#!/bin/bash

rm -fr CMakeCache.txt cmake_install.cmake CMakeFiles Makefile 

rm -fr linux/CMakeCache.txt linux/cmake_install.cmake linux/CMakeFiles linux/Makefile linux/bin
rm -fr linux/x86/CMakeCache.txt linux/x86/cmake_install.cmake linux/x86/CMakeFiles linux/x86/Makefile linux/x86/bin
rm -fr linux/arm/CMakeCache.txt linux/arm/cmake_install.cmake linux/arm/CMakeFiles linux/arm/Makefile linux/arm/bin

rm -fr mppa/CMakeCache.txt mppa/cmake_install.cmake mppa/CMakeFiles mppa/Makefile mppa/bin \
	mppa/master/CMakeCache.txt mppa/master/cmake_install.cmake mppa/master/CMakeFiles mppa/master/Makefile mppa/master/bin \
	mppa/slave/CMakeCache.txt mppa/slave/cmake_install.cmake mppa/slave/CMakeFiles mppa/slave/Makefile mppa/slave/bin

rm -fr utils/CMakeCache.txt utils/cmake_install.cmake utils/CMakeFiles utils/Makefile utils/bin
