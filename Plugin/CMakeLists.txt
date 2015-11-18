cmake_minimum_required(VERSION 2.8.12)
project(org_osvr_nod)

find_package(osvr REQUIRED)

osvr_convert_json(org_osvr_nod
    org_osvr_nod.json
    "${CMAKE_CURRENT_BINARY_DIR}/org_osvr_nod.h")
include_directories("${CMAKE_CURRENT_BINARY_DIR}")

osvr_add_plugin(NAME org_osvr_nod
    CPP 
    SOURCES
    org_osvr_nod.cpp
    "${CMAKE_CURRENT_BINARY_DIR}/org_osvr_nod.h")

#check for openspatial dependencies and link in, fail if not found
if(CMAKE_GENERATOR MATCHES "Win64")
    MESSAGE("64 bit OpenSpatial")
    set(OPENSPATIAL_LIB "C:/Program Files (x86)/Nod Labs/Nod Configuration/Dynamic SDK/x86_64")
else()
    MESSAGE("32 bit OpenSpatial")
    set(OPENSPATIAL_LIB "C:/Program Files (x86)/Nod Labs/Nod Configuration/Dynamic SDK/x86")
endif()
set(OPENSPATIAL_INC "C:/Program Files (x86)/Nod Labs/Nod Configuration/Dynamic SDK")
if(EXISTS "${OPENSPATIAL_INC}/" AND EXISTS "${OPENSPATIAL_LIB}/")
    MESSAGE("OPENSPATIAL FOUND")
    include_directories(${OPENSPATIAL_INC})
    target_link_libraries(org_osvr_nod ${OPENSPATIAL_LIB}/OpenSpatialDll.lib)
else()
    MESSAGE(FATAL_ERROR "REQUIRES OPENSPATIAL SDK TO BE INSTALLED -- please visit openspatial.net")
endif()