#!/bin/bash
cd build

set -e

echo -e "\n######################################################################"
echo -e "🚀 Running Engine Tests..."
echo -e "######################################################################\n"

cmake --build . --target engine_tests
./tests/engine_tests

echo -e "\n######################################################################"
echo -e "🚀 Running Server Tests..."
echo -e "######################################################################\n"

cmake --build . --target server_tests
./tests/server_tests