include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../libs/kiss_fft130)
include_directories (/opt/homebrew/include)

# Find libsamplerate
find_library(LIBSAMPLERATE_LIBRARIES NAMES samplerate)

add_library (
    BTrack STATIC
    BTrack.cpp
    BTrack.h
    OnsetDetectionFunction.cpp
    OnsetDetectionFunction.h
    CircularBuffer.h
)

source_group (Source src)

target_compile_definitions (BTrack PUBLIC -DUSE_KISS_FFT)

# Link against libsamplerate
target_link_libraries(BTrack PRIVATE ${LIBSAMPLERATE_LIBRARIES})