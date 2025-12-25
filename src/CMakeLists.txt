option(USE_KISS_FFT "Enable Kiss FFT backend" ON)

include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../libs/kiss_fft130)
include_directories (/opt/homebrew/include)

# Find libsamplerate
find_library(LIBSAMPLERATE_LIBRARIES NAMES samplerate)

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