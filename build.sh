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

# Check for vcpkg
VCPKG_CMAKE_TOOLCHAIN=""
if [ -f "vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_CMAKE_TOOLCHAIN="vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "✓ Using local vcpkg in project directory"
elif [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_CMAKE_TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    echo "✓ Using vcpkg from VCPKG_ROOT: $VCPKG_ROOT"
elif [ -f "../vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    VCPKG_CMAKE_TOOLCHAIN="../vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "✓ Using vcpkg from parent directory"
else
    echo "ERROR: vcpkg not found"
    echo "Please set VCPKG_ROOT environment variable or install vcpkg in the project directory"
    exit 1
fi
echo ""

# Create build directory
mkdir -p build
cd build

echo "=========================================="
echo "Configuring with CMake..."
echo "=========================================="
echo ""

# Prepare toolchain path for CMake (build dir is `build/` so make relative if needed)
if [[ "$VCPKG_CMAKE_TOOLCHAIN" = /* ]]; then
    TOOLCHAIN_FILE="$VCPKG_CMAKE_TOOLCHAIN"
else
    TOOLCHAIN_FILE="../$VCPKG_CMAKE_TOOLCHAIN"
fi

echo "Using CMake toolchain file: $TOOLCHAIN_FILE"

# Check that a build program (make or ninja) exists for CMake's default generators
if ! command -v make &> /dev/null && ! command -v ninja &> /dev/null; then
    echo "ERROR: No build tool found (make or ninja)."
    echo "Please install 'make' or 'ninja' so CMake can generate build files."
    exit 1
fi

# Configure with CMake
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
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
