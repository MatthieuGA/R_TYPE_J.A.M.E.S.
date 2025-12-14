#!/bin/bash

# R-Type Server Build and Run Script
# This script configures, builds, and runs the R-Type server

set -e  # Exit on error

# Colors for output
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== R-Type Server Build and Run ===${NC}"

# Check if build directory exists, if not configure
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Build directory not found. Configuring CMake...${NC}"
    cmake -S . -B build
fi

# Build the server
echo -e "${YELLOW}Building server...${NC}"
cmake --build build --target r-type_server -j$(nproc)

if [ $? -eq 0 ]; then
    echo -e "${GREEN}Build successful!${NC}"
    echo -e "${GREEN}Starting server...${NC}"
    echo ""
    
    # Run the server with any passed arguments
    ./build/server/r-type_server "$@"
else
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi
