#!/bin/bash

# R-TYPE Build Script for Linux
# Requires: CMake, C++ Compiler (GCC/Clang), Git

set -e  # Exit on error

echo "=========================================="
echo "R-TYPE Build Script (vcpkg)"
echo "=========================================="
echo ""

# Function to test exit status
function testExitStatus() {
    if [ $1 -ne 0 ]; then
        echo "ERROR: $2 failed"
        exit 1
    fi
}

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed"
    echo "Please install CMake (version 3.17 or higher)"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
echo "✓ CMake version: $CMAKE_VERSION"
echo ""

# Install vcpkg if not present
if [ ! -d "vcpkg" ]; then
    echo "Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git
fi

# Bootstrap vcpkg if needed
if [ ! -f "vcpkg/vcpkg" ]; then
    echo "Bootstrapping vcpkg..."
    cd vcpkg
    ./bootstrap-vcpkg.sh --disable-metrics
    testExitStatus $? "vcpkg bootstrap"
    cd ..
    echo ""
fi

echo "✓ vcpkg is ready"
echo ""

# Create build directory
mkdir -p build
cd build

echo "=========================================="
echo "Configuring with CMake..."
echo "=========================================="
echo ""

# Configure with CMake
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="../vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Release

testExitStatus $? "CMake configuration"

echo ""
echo "=========================================="
echo "Building..."
echo "=========================================="
echo ""

# Build
cmake --build . --config Release -j$(nproc)
testExitStatus $? "Build"

cd ..

echo ""
echo "=========================================="
echo "✓ Build completed successfully!"
echo "=========================================="
echo ""
echo "To run the client:"
echo "  ./r-type_client"
echo ""
echo "To run the server:"
echo "  ./r-type_server"
echo ""
exit 0
