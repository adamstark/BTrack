option(USE_KISS_FFT "Enable Kiss FFT backend" ON)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../libs/kiss_fft130)

if(APPLE)
    include_directories (/opt/homebrew/include)
elseif(UNIX)
    include_directories (/usr/include)
endif()

# Find libsamplerate
if(APPLE)
    find_library(LIBSAMPLERATE_LIBRARIES NAMES samplerate PATHS /opt/homebrew/lib /usr/local/lib)
elseif(WIN32)
    # Windows users need to set path manually or via vcpkg
    find_library(LIBSAMPLERATE_LIBRARIES NAMES libsamplerate samplerate PATHS C:/libsamplerate/lib)
else()
    # Linux / other UNIX
    find_library(LIBSAMPLERATE_LIBRARIES NAMES samplerate)
endif()

if(NOT LIBSAMPLERATE_LIBRARIES)
    message(FATAL_ERROR "libsamplerate not found! Please install it.")
endif()

message(STATUS "Using libsamplerate: ${LIBSAMPLERATE_LIBRARIES}")

set(BTRACK_SOURCES
    BTrack.cpp
    BTrack.h
    OnsetDetectionFunction.cpp
    OnsetDetectionFunction.h
    CircularBuffer.h
)

source_group (Source src)

if(USE_KISS_FFT)
    list(APPEND BTRACK_SOURCES ../libs/kiss_fft130/kiss_fft.c)
endif()

add_library (
    BTrack STATIC
    ${BTRACK_SOURCES}
)

if(USE_KISS_FFT)
    target_compile_definitions(BTrack PUBLIC -DUSE_KISS_FFT)
endif()

# Link against libsamplerate
target_link_libraries(BTrack PRIVATE ${LIBSAMPLERATE_LIBRARIES})