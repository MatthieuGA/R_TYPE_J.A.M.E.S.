#!/bin/bash

# R-TYPE Build Script for Linux (System Libraries)
# Uses system-installed libraries instead of vcpkg

set -e  # Exit on error

echo "=========================================="
echo "R-TYPE Build Script (System Libraries)"
echo "=========================================="
echo ""

# Check for CMake
if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake is not installed"
    exit 1
fi

CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
echo "✓ CMake version: $CMAKE_VERSION"

# Check for system SFML
if ! pkg-config --exists sfml-all 2>/dev/null; then
    echo "WARNING: System SFML not detected via pkg-config"
    echo "Checking for SFML headers..."
    if [ ! -f "/usr/include/SFML/Audio.hpp" ]; then
        echo "ERROR: SFML not found. Install with: sudo dnf install SFML-devel"
        exit 1
    fi
fi
echo "✓ Using system SFML"

# Check for Boost
if ! ldconfig -p | grep -q libboost_system; then
    echo "WARNING: Boost libraries not detected"
    echo "Install with: sudo dnf install boost-devel"
fi
echo "✓ Using system Boost (if available)"

echo ""

# Create build directory
mkdir -p build_system
cd build_system

echo "=========================================="
echo "Configuring with CMake (NO vcpkg)..."
echo "=========================================="
echo ""

# Configure WITHOUT vcpkg toolchain
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DUSE_SYSTEM_LIBS=ON

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi

echo ""
echo "=========================================="
echo "Building..."
echo "=========================================="
echo ""

# Build
cmake --build . --config Release -j$(nproc)

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi

cd ..

echo ""
echo "========================================="
echo "✓ Build completed successfully!"
echo "========================================="
echo ""
echo "To run the client:"
echo "  ./build_system/client/r-type_client"
echo ""
