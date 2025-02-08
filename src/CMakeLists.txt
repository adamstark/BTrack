include_directories (${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../libs/kiss_fft130)

IF (WIN32)
	# Find libsamplerate
	message(STATUS "Windows System...")
	include_directories (${CMAKE_CURRENT_SOURCE_DIR}/../libs/libsamplerate-0.2.2-win64/include) # We need the include dir for compilation on windows
	set(SAMPLERATE_VAR "${CMAKE_CURRENT_SOURCE_DIR}/../libs/libsamplerate-0.2.2-win64/lib") # path to libsample rate
	message(STATUS "${SAMPLERATE_VAR}")
	find_library(LIBSAMPLERATE_LIBRARIES NAMES samplerate PATHS ${SAMPLERATE_VAR})
ELSE()
	# It means we are in a UNIX system (most likely)
	message(STATUS "Linux System...")
	find_library(LIBSAMPLERATE_LIBRARIES NAMES samplerate)
ENDIF()

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
