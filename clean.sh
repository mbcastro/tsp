#!/bin/bash

rm -fr CMakeCache.txt cmake_install.cmake CMakeFiles Makefile 

rm -fr x86/CMakeCache.txt x86/cmake_install.cmake x86/CMakeFiles x86/Makefile x86/bin

rm -fr arm/CMakeCache.txt arm/cmake_install.cmake arm/CMakeFiles arm/Makefile arm/bin

rm -fr mppa/CMakeCache.txt mppa/cmake_install.cmake mppa/CMakeFiles mppa/Makefile mppa/bin \
	mppa/master/CMakeCache.txt mppa/master/cmake_install.cmake mppa/master/CMakeFiles mppa/master/Makefile mppa/master/bin \
	mppa/slave/CMakeCache.txt mppa/slave/cmake_install.cmake mppa/slave/CMakeFiles mppa/slave/Makefile mppa/slave/bin

rm -fr utils/CMakeCache.txt utils/cmake_install.cmake utils/CMakeFiles utils/Makefile utils/bin
