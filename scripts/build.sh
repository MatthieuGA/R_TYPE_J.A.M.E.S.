#!/bin/bash

# R-TYPE Build Script for Linux
# Supports both vcpkg and Conan package managers
# Requires: CMake, C++ Compiler (GCC/Clang), Git

set -e  # Exit on error

echo "=========================================="
echo "R-TYPE Build Script (vcpkg / Conan)"
echo "=========================================="
echo ""

# ===========================
# Configuration Options
# ===========================
# Force a specific package manager: "vcpkg", "conan", or "auto" (default)
FORCE_PACKAGE_MANAGER="${FORCE_PACKAGE_MANAGER:-auto}"
BUILD_TYPE="${BUILD_TYPE:-Release}"

# ===========================
# Helper Functions
# ===========================

function testExitStatus() {
    if [ $1 -ne 0 ]; then
        echo "ERROR: $2 failed"
        exit 1
    fi
}

function checkCmake() {
    if ! command -v cmake &> /dev/null; then
        echo "ERROR: CMake is not installed"
        echo "Please install CMake (version 3.23 or higher)"
        exit 1
    fi
    CMAKE_VERSION=$(cmake --version | head -n1 | cut -d' ' -f3)
    echo "✓ CMake version: $CMAKE_VERSION"
}

function checkBuildTool() {
    if ! command -v make &> /dev/null && ! command -v ninja &> /dev/null; then
        echo "ERROR: No build tool found (make or ninja)."
        echo "Please install 'make' or 'ninja' so CMake can generate build files."
        exit 1
    fi
}

function checkVcpkg() {
    # Check for vcpkg in various locations
    if [ -f "vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
        VCPKG_CMAKE_TOOLCHAIN="vcpkg/scripts/buildsystems/vcpkg.cmake"
        echo "✓ Found local vcpkg in project directory"
        return 0
    elif [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
        VCPKG_CMAKE_TOOLCHAIN="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
        echo "✓ Found vcpkg from VCPKG_ROOT: $VCPKG_ROOT"
        return 0
    elif [ -f "../vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
        VCPKG_CMAKE_TOOLCHAIN="../vcpkg/scripts/buildsystems/vcpkg.cmake"
        echo "✓ Found vcpkg in parent directory"
        return 0
    fi
    return 1
}

function checkConan() {
    if command -v conan &> /dev/null; then
        CONAN_VERSION=$(conan --version | head -n1)
        echo "✓ Conan found: $CONAN_VERSION"
        return 0
    fi
    return 1
}

function installVcpkg() {
    echo "Cloning vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git
    testExitStatus $? "vcpkg clone"

    echo "Bootstrapping vcpkg..."
    cd vcpkg
    ./bootstrap-vcpkg.sh
    testExitStatus $? "vcpkg bootstrap"
    cd ..

    VCPKG_CMAKE_TOOLCHAIN="vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo "✓ vcpkg installed successfully"
}

function installConan() {
    echo "Installing Conan via pip..."
    if command -v pip3 &> /dev/null; then
        pip3 install conan
    elif command -v pip &> /dev/null; then
        pip install conan
    else
        echo "ERROR: pip not found. Please install Python and pip first."
        exit 1
    fi
    testExitStatus $? "Conan installation"

    # Detect default profile
    echo "Detecting Conan profile..."
    conan profile detect --force 2>/dev/null || true
    echo "✓ Conan installed successfully"
}

function setupConanDependencies() {
    echo "Installing dependencies with Conan..."
    conan install . \
        --output-folder=build \
        --build=missing \
        -s build_type=$BUILD_TYPE \
        -s compiler.cppstd=20 \
        -c tools.system.package_manager:mode=install \
        -c tools.system.package_manager:sudo=True
    testExitStatus $? "Conan install"
    echo "✓ Conan dependencies installed"
}

function buildWithVcpkg() {
    echo ""
    echo "=========================================="
    echo "Building with vcpkg"
    echo "=========================================="
    echo ""

    mkdir -p build
    cd build

    # Prepare toolchain path for CMake
    if [[ "$VCPKG_CMAKE_TOOLCHAIN" = /* ]]; then
        TOOLCHAIN_FILE="$VCPKG_CMAKE_TOOLCHAIN"
    else
        TOOLCHAIN_FILE="../$VCPKG_CMAKE_TOOLCHAIN"
    fi

    echo "Using CMake toolchain file: $TOOLCHAIN_FILE"

    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE

    testExitStatus $? "CMake configuration"
    buildProject
    cd ..
}

function buildWithConan() {
    echo ""
    echo "=========================================="
    echo "Building with Conan"
    echo "=========================================="
    echo ""

    # Install Conan dependencies
    setupConanDependencies

    mkdir -p build
    cd build

    echo "Configuring with CMake (Conan)..."
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE="conan_toolchain.cmake" \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE

    testExitStatus $? "CMake configuration"
    buildProject
    cd ..
}

function buildProject() {
    echo ""
    echo "=========================================="
    echo "Building..."
    echo "=========================================="
    echo ""

    # Determine optimal number of parallel jobs
    NUM_CORES=$(nproc)
    NUM_JOBS=$((NUM_CORES + 1))
    echo "Building with $NUM_JOBS parallel jobs (CPU cores: $NUM_CORES)"
    echo ""

    cmake --build . --config $BUILD_TYPE -j${NUM_JOBS}
    testExitStatus $? "Build"
}

function printSuccess() {
    echo ""
    echo "========================================="
    echo "✓ Build completed successfully!"
    echo "========================================="
    echo ""
    echo "To run the client:"
    echo "  cd ./build/client && ./r-type_client"
    echo ""
    echo "To run the server:"
    echo "  cd ./build/server && ./r-type_server"
    echo ""
}

# ===========================
# Main Script
# ===========================

# Check prerequisites
checkCmake
checkBuildTool
echo ""

# Determine which package manager to use
PACKAGE_MANAGER=""

if [ "$FORCE_PACKAGE_MANAGER" = "vcpkg" ]; then
    echo "Forced package manager: vcpkg"
    if checkVcpkg; then
        PACKAGE_MANAGER="vcpkg"
    else
        echo "ERROR: vcpkg forced but not found"
        exit 1
    fi
elif [ "$FORCE_PACKAGE_MANAGER" = "conan" ]; then
    echo "Forced package manager: Conan"
    if checkConan; then
        PACKAGE_MANAGER="conan"
    else
        echo "Conan not found, attempting to install..."
        installConan
        PACKAGE_MANAGER="conan"
    fi
else
    # Auto-detect: prefer vcpkg, fallback to Conan
    echo "Auto-detecting package manager..."
    echo ""

    if checkVcpkg; then
        PACKAGE_MANAGER="vcpkg"
    elif checkConan; then
        echo "⚠ vcpkg not found, using Conan instead"
        PACKAGE_MANAGER="conan"
    else
        echo ""
        echo "Neither vcpkg nor Conan found."
        echo ""
        echo "Which package manager would you like to use?"
        echo "  1) vcpkg (recommended)"
        echo "  2) Conan"
        echo "  3) Cancel"
        echo ""
        read -p "Enter choice [1-3]: " choice

        case $choice in
            1)
                echo ""
                installVcpkg
                PACKAGE_MANAGER="vcpkg"
                ;;
            2)
                echo ""
                installConan
                PACKAGE_MANAGER="conan"
                ;;
            *)
                echo "Build cancelled."
                exit 0
                ;;
        esac
    fi
fi

echo ""
echo "Using package manager: $PACKAGE_MANAGER"

# Build with selected package manager
if [ "$PACKAGE_MANAGER" = "vcpkg" ]; then
    buildWithVcpkg
elif [ "$PACKAGE_MANAGER" = "conan" ]; then
    buildWithConan
fi

printSuccess
