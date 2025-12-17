#!/bin/bash
# Run RFC-compliant packet unit tests
# This script compiles and runs the packet test suite using the system googletest library

set -e

cd "$(dirname "$0")/.."

echo "Building packet tests..."
g++ -std=c++20 -Wall -Wextra \
    -I./server/include \
    -I./engine/include \
    tests/test_packets.cpp \
    -lgtest -lgtest_main -pthread \
    -o test_packets_binary

echo "Running tests..."
./test_packets_binary

echo ""
echo "✅ All packet tests passed!"

rm test_packets_binary
