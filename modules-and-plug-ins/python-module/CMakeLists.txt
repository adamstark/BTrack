cmake_minimum_required(VERSION 3.15)
project(BTrackPythonModule LANGUAGES C CXX)

# ------------------------------------------------------------
# Find Python interpreter + development headers
# ------------------------------------------------------------
find_package(Python REQUIRED COMPONENTS Interpreter Development)

# ------------------------------------------------------------
# Find NumPy headers
# ------------------------------------------------------------
execute_process(
    COMMAND "${Python_EXECUTABLE}" -c
            "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# ------------------------------------------------------------
# Build core BTrack library
# ------------------------------------------------------------
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../../src build/src)

# ------------------------------------------------------------
# Python extension module
# ------------------------------------------------------------
add_library(
    btrack
    MODULE
    btrack_python_module.cpp
)

# ------------------------------------------------------------
# Set version
# ------------------------------------------------------------
set(BTRACK_VERSION "1.0.5")
target_compile_definitions(btrack PRIVATE BTRACK_VERSION=\"${BTRACK_VERSION}\")

# Include directories
target_include_directories(btrack PRIVATE
    ${Python_INCLUDE_DIRS}
    ${NUMPY_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../libs/kiss_fft130
)

# Link against BTrack and Python
target_link_libraries(btrack PRIVATE BTrack ${Python_LIBRARIES})

# Ensure proper module name and location
set_target_properties(btrack PROPERTIES
    PREFIX ""                          # btrack.so, not libbtrack.so
    OUTPUT_NAME "btrack"               # final module name: btrack.so
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(APPLE)
    set_target_properties(btrack PROPERTIES 
        SUFFIX ".so"
        LINK_FLAGS "-undefined dynamic_lookup"
    )
endif()

# ------------------------------------------------------------
# Install btrack.so directly (no __init__.py, no submodule)
# ------------------------------------------------------------
install(TARGETS btrack
        LIBRARY DESTINATION ".")
