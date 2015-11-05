cmake_minimum_required(VERSION 2.8.12)
project(NodOSVRPlugin)

find_package(osvr REQUIRED)

osvr_convert_json(NodOSVR
    NodOSVR.json
    "${CMAKE_CURRENT_BINARY_DIR}/NodOSVR.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

osvr_add_plugin(NAME NodOSVRPlugin
    CPP 
    SOURCES
    NodOSVRPlugin.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/NodOSVR.h"
	Settings.h
	NodPlugin.h)
