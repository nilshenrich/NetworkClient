# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Minimum required version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(example VERSION 1.0)

# Include standard directories
include(GNUInstallDirs)

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_executable(example "main.cpp")

# Set include path to search for headers
include_directories(../include ${CMAKE_INSTALL_INCLUDEDIR})

# Set compiler and linker flags
target_compile_options(example PRIVATE -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(example PRIVATE $<$<CONFIG:RELEASE>: -s>)
target_link_libraries(example -lnetworkClientTcp -lnetworkClientTls -lcrypto -lcrypt -lssl -pthread)

# Use C++17 standard
set_target_properties(example PROPERTIES
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Print build type
message("Build type: ${CMAKE_BUILD_TYPE}")
