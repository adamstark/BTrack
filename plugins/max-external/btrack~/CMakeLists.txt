cmake_minimum_required(VERSION 3.19)
project(btrack_tilde)

include(${CMAKE_CURRENT_SOURCE_DIR}/max-sdk-base/script/max-pretarget.cmake)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/kiss_fft130)

# Find libsamplerate relative to build folder
set(LIBSAMPLERATE_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/libsamplerate-install)

find_library(LIBSAMPLERATE_LIBRARY 
    NAMES libsamplerate.a
    PATHS ${LIBSAMPLERATE_ROOT}/lib
    NO_DEFAULT_PATH
)

find_path(LIBSAMPLERATE_INCLUDE_DIR 
    NAMES samplerate.h
    PATHS ${LIBSAMPLERATE_ROOT}/include
    NO_DEFAULT_PATH
)

include_directories(${LIBSAMPLERATE_INCLUDE_DIR})

#############################################################
# MAX EXTERNAL
#############################################################
add_definitions(-DUSE_KISS_FFT)

include_directories( 
	"${MAX_SDK_INCLUDES}"
	"${MAX_SDK_MSP_INCLUDES}"
	"${MAX_SDK_JIT_INCLUDES}"
    "../../src/"
    "../../libs/kiss_fft130/"
)

file(GLOB PROJECT_SRC
    "btrack~.cpp"
    "../../src/*.h"
    "../../src/*.cpp"
	"../../libs/kiss_fft130/kiss_fft.c"
)
add_library( 
	${PROJECT_NAME} 
	MODULE
	${PROJECT_SRC}
)

# Link against the samplerate library
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBSAMPLERATE_LIBRARY})

# ----------------------------------------------------------
# Ensure PkgInfo exists in the output bundle (CI-safe)
# ----------------------------------------------------------
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/..
    COMMAND ${CMAKE_COMMAND} -E echo "APPL????"
            > $<TARGET_FILE_DIR:${PROJECT_NAME}>/../PkgInfo
)

include(${CMAKE_CURRENT_SOURCE_DIR}/max-sdk-base/script/max-posttarget.cmake)

# Rename the output file to btrack~
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_NAME "btrack~"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/output"
)