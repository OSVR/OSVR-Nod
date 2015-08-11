#
# Nod plugin for OSVR
#
cmake_minimum_required(VERSION 2.8.12)
project(com_osvr_Nod)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# This looks for an osvrConfig.cmake file - most of the time it can be
# autodetected but you might need to create/extend CMAKE_PREFIX_PATH to include something like
# C:/Users/Ryan/Desktop/build/OSVR-Core-vc12 or
# C:/Users/Ryan/Downloads/OSVR-Core-Snapshot-v0.1-406-gaa55515-build54-vs12-32bit
# in the CMake GUI or command line.
find_package(osvr REQUIRED)

# Generate our JSON header file
osvr_convert_json(com_osvr_Nod_json
    com_osvr_Nod.json
    "${CMAKE_CURRENT_BINARY_DIR}/com_osvr_Nod_json.h")

# Be able to find our generated header file.
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

# Build and install the plugin
osvr_add_plugin(com_osvr_Nod
    com_osvr_Nod.cpp
    OpenSpatialServiceController.h
    OpenSpatialServiceController.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/com_osvr_Nod_json.h")

# Link against PluginKit (C++ wrapper)
target_link_libraries(com_osvr_Nod
    osvr::osvrPluginKitCpp)

# Install docs, license, sample config
install(FILES
    osvr_server_config.nod.sample.json
    README.md
    LICENSE
    DESTINATION .)