cmake_minimum_required (VERSION 3.12)
project (BTrack VERSION 1.0.5)

set (CMAKE_CXX_STANDARD 11)

option (BUILD_TESTS "Build tests" OFF)
option (BUILD_PYTHON "Build Python bindings" ON)

add_subdirectory (src)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory (tests)
endif (BUILD_TESTS)

if (BUILD_PYTHON)
    add_subdirectory(modules-and-plug-ins/python-module)
endif()

set (CMAKE_SUPPRESS_REGENERATION true)

