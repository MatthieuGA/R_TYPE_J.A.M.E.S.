/**
 * @file stub_missing_symbols.cpp
 * @brief Stub shared library for testing missing symbol detection.
 *
 * This library is intentionally missing the required plugin symbols
 * (create_graphics_backend_v1, destroy_graphics_backend_v1,
 * graphics_backend_name_v1). Used to test that the plugin loader correctly
 * detects and reports missing symbols.
 *
 */

// Deliberately exports nothing that the plugin loader expects.
// The library can be loaded, but symbol resolution will fail.

extern "C" {
void stub_unrelated_function() {
    // This function exists, but it's not what the plugin loader is looking for
}
}  // extern "C"
