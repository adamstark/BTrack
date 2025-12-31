#!/bin/bash

print_section() {
    echo ""
    echo ""
    echo "=========================================="
    echo "$1"
    echo "=========================================="
    echo ""
    echo ""
}

if [ "$1" = "clean" ]; then
    print_section "CLEANING"
    rm -rf build
    rm -rf libsamplerate
    rm -rf libsamplerate-install
    rm -rf output
    echo "Clean complete!"
    exit 0
fi

# Your normal build commands here
echo "Building btrack~ max external..."

# Clone and build libsamplerate
print_section "BUILDING LIBSAMPLERATE"
git clone https://github.com/libsndfile/libsamplerate.git
cd libsamplerate && mkdir build && cd build
cmake .. \
  -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTING=OFF \
  -DLIBSAMPLERATE_EXAMPLES=OFF \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13 \
  -DCMAKE_INSTALL_PREFIX=../../libsamplerate-install
cmake --build . && cmake --install .
cd ../..

# Clone max-sdk
print_section "CLONING MAX-SDK-BASE"
git clone https://github.com/Cycling74/max-sdk-base.git


# Build max-external
print_section "BUILDING BTRACK MAX EXTERNAL"
mkdir build && cd build
cmake .. -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64"
cmake --build . 