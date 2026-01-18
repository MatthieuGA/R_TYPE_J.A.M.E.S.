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

echo -e "\n######################################################################"
echo -e "🧪 Running Plugin Loader Unit Tests..."
echo -e "######################################################################\n"

cmake --build . --target plugin_loader_tests
./tests/plugin_loader_tests

echo -e "\n######################################################################"
echo -e "🔌 Running Integration Smoke Tests..."
echo -e "######################################################################\n"

cmake --build . --target integration_smoke_test
./tests/integration_smoke_test

echo -e "\n######################################################################"
echo -e "🎨 Running Pixel Comparison Tests..."
echo -e "######################################################################\n"

cmake --build . --target pixel_compare_test
./tests/pixel_compare_test

echo -e "\n######################################################################"
echo -e "✅ All tests completed successfully!"
echo -e "######################################################################\n"