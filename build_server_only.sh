#!/bin/bash

# Simple build for server and tests WITHOUT client/SFML
# This avoids the openal-soft and freetype vcpkg issues

set -e

echo "=========================================="
echo "R-TYPE Server-Only Build (No SFML)"
echo "=========================================="

# Clean
rm -rf build_server
mkdir -p build_server
cd build_server

# Configure without vcpkg - use system libraries only
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_CLIENT=OFF \
    -DCMAKE_CXX_COMPILER=g++ \
    -DCMAKE_C_COMPILER=gcc

# Build server
cmake --build . --target r-type_server -j$(nproc)

echo ""
echo "=========================================="
echo "✓ Server build completed!"
echo "=========================================="
echo ""
echo "Binary: ./build_server/server/r-type_server"
echo ""
