#!/bin/bash

# Build script for R-Type Server
# Usage: ./build_server.sh [options]
# Options:
#   -c, --clean      Clean build directory before building
#   -r, --release    Build in Release mode (default)
#   -d, --debug      Build in Debug mode
#   -h, --help       Show this help message

set -e

# Default values
CLEAN=false
BUILD_TYPE="Release"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -c|--clean)
            CLEAN=true
            shift
            ;;
        -r|--release)
            BUILD_TYPE="Release"
            shift
            ;;
        -d|--debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        -h|--help)
            echo "Build script for R-Type Server"
            echo ""
            echo "Usage: ./build_server.sh [options]"
            echo ""
            echo "Options:"
            echo "  -c, --clean      Clean build directory before building"
            echo "  -r, --release    Build in Release mode (default)"
            echo "  -d, --debug      Build in Debug mode"
            echo "  -h, --help       Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use -h or --help for usage information"
            exit 1
            ;;
    esac
done

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "🧹 Cleaning build directory..."
    rm -rf "$SCRIPT_DIR/build"
fi

# Create build directory if it doesn't exist
mkdir -p "$SCRIPT_DIR/build"

# Configure with CMake
echo "🔧 Configuring CMake (${BUILD_TYPE} mode)..."
cd "$SCRIPT_DIR"
cmake -S . -B build \
    -DCMAKE_TOOLCHAIN_FILE="vcpkg/scripts/buildsystems/vcpkg.cmake" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# Build
echo "🏗️  Building server..."
cmake --build build -j"$(nproc)" --target r-type_server

# Check if binary was created
if [ -f "$SCRIPT_DIR/build/server/r-type_server" ]; then
    echo "✅ Server built successfully!"
    echo "📍 Location: $SCRIPT_DIR/build/server/r-type_server"
    echo ""
    echo "To run the server:"
    echo "  $SCRIPT_DIR/build/server/r-type_server <port>"
    echo ""
    echo "Example:"
    echo "  $SCRIPT_DIR/build/server/r-type_server 8080"
else
    echo "❌ Server build failed - executable not found"
    exit 1
fi
