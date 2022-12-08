# For debugging please use the command "cmake -DCMAKE_BUILD_TYPE=Debug .."

# Create library for this package

# Minimum requried version of CMake
cmake_minimum_required(VERSION 3.10)

# Project name
project(networkClientTcp VERSION 2.2 DESCRIPTION "Client for TCP")
project(networkClientTls VERSION 2.2 DESCRIPTION "Client for TLS")

# Include standard directories
include(GNUInstallDirs)

# The deault build type is Release
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Source files to be compiled
add_library(networkClientTcp SHARED "src/TcpClient.cpp")
add_library(networkClientTls SHARED "src/TlsClient.cpp")

# Set include path to search for headers
include_directories(include)

# Set compiler flags
target_compile_options(networkClientTcp PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(networkClientTcp PUBLIC $<$<CONFIG:RELEASE>: -s>)
target_compile_options(networkClientTls PUBLIC -fexceptions
    $<$<CONFIG:DEBUG>: -DDEVELOP -Wall -g -Og>
    $<$<CONFIG:RELEASE>: -O3>)
target_link_options(networkClientTls PUBLIC $<$<CONFIG:RELEASE>: -s>)

# Use C++17 standard
set_target_properties(networkClientTcp PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
    PUBLIC_HEADER "include/TcpClient.h;include/NetworkClient.h;include/NetworkingDefines.h;include/NetworkClient.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)
set_target_properties(networkClientTls PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 2
    PUBLIC_HEADER "include/TlsClient.h;include/NetworkClient.h;include/NetworkingDefines.h;include/NetworkClient.tpp"
    CXX_STANDARD 17
    CMAKE_CXX_STANDARD_REQUIRED True)

# Install rule (copy to install directory)
install(TARGETS networkClientTcp networkClientTls
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/NetworkClient)

# Print build type
message("Build type: ${CMAKE_BUILD_TYPE}")
