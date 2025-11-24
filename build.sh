#!/bin/bash

# R-TYPE J.A.M.E.S. Build Script for Linux
# This script sets up vcpkg and builds the project

set -e  # Exit on error

echo "=========================================="
echo "R-TYPE J.A.M.E.S. Build Script"
echo "=========================================="
echo ""

# Check if vcpkg is installed
if [ -z "$VCPKG_ROOT" ]; then
    echo "⚠️  VCPKG_ROOT is not set."
    echo "Please install vcpkg and set VCPKG_ROOT environment variable."
    echo ""
    echo "Quick setup:"
    echo "  git clone https://github.com/microsoft/vcpkg.git"
    echo "  cd vcpkg && ./bootstrap-vcpkg.sh"
    echo "  export VCPKG_ROOT=\$(pwd)"
    echo ""
    exit 1
fi

echo "✓ vcpkg found at: $VCPKG_ROOT"
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "❌ CMake is not installed. Please install CMake 3.21 or higher."
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
echo "✓ CMake version: $CMAKE_VERSION"
echo ""

# Create build directory
BUILD_DIR="build"
if [ -d "$BUILD_DIR" ]; then
    echo "⚠️  Build directory exists. Cleaning..."
    rm -rf "$BUILD_DIR"
fi

mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

echo ""
echo "=========================================="
echo "Configuring with CMake..."
echo "=========================================="
echo ""

# Configure with CMake
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE=Release

echo ""
echo "=========================================="
echo "Building..."
echo "=========================================="
echo ""

# Build
cmake --build . --config Release -j$(nproc)

echo ""
echo "=========================================="
echo "✓ Build completed successfully!"
echo "=========================================="
echo ""
echo "To run the client:"
echo "  cd $BUILD_DIR"
echo "  ./r-type_client"
echo ""
