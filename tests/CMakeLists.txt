include_directories (doctest)
include_directories (${BTrack_SOURCE_DIR}/src)
include_directories (${BTrack_SOURCE_DIR}/libs/kiss_fft130)

add_executable (Tests 
    main.cpp 
    ${BTrack_SOURCE_DIR}/libs/kiss_fft130/kiss_fft.c
    Test_BTrack.cpp
    )

target_link_libraries (Tests BTrack)
target_compile_features (Tests PRIVATE cxx_std_17)
add_test (NAME Tests COMMAND Tests)