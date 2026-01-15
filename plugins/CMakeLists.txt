# Graphics Backend Plugins
# This directory contains reference implementations of graphics backends
# that load as standalone shared libraries via the plugin ABI.

add_subdirectory(graphics_sfml)

# Test stub: library with missing plugin symbols (for testing error handling)
add_library(stub_missing_symbols SHARED stub_missing_symbols.cpp)
set_target_properties(stub_missing_symbols PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)
