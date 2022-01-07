# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(networkClientTls VERSION 1.0 DESCRIPTION "Client for TLS")
project(networkClientTcp VERSION 1.0 DESCRIPTION "Client for TCP")

# Include standard directories
include(GNUInstallDirs)

# Build type is Release
set(CMAKE_BUILD_TYPE Release)

# Source files to be compiled
add_library(networkClientTls SHARED "src/TlsClient.cpp")
add_library(networkClientTcp SHARED "src/TcpClient.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(networkClientTls PUBLIC -fexceptions -O3)
target_compile_options(networkClientTcp PUBLIC -fexceptions -O3)

# Use C++17 standard
set_target_properties(networkClientTls PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/TlsClient.h;include/NetworkClient.h;include/NetworkingDefines.h"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(networkClientTcp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "include/TcpClient.h;include/NetworkClient.h;include/NetworkingDefines.h"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS networkClientTcp networkClientTls
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NetworkClient)
