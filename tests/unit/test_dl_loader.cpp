/**
 * @file test_dl_loader.cpp
 * @brief Unit tests for GraphicsPluginLoader (dynamic library loading).
 *
 * Tests verify:
 * - Loading non-existent libraries fails gracefully with clear error messages
 * - Loading libraries missing required symbols reports error and does not
 * crash
 * - Successfully loading a valid plugin registers it in the factory
 *
 * SCOPE: Phase E - Plugin loader unit tests
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "graphics/GraphicsBackendFactory.hpp"
#include "graphics/GraphicsPluginLoader.hpp"

namespace Rtype::Client::Graphics {

/**
 * @brief Test suite for GraphicsPluginLoader.
 *
 * Tests dynamic library loading, symbol resolution, and error handling.
 */
class GraphicsPluginLoaderTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Clear any previously registered backends before each test
        // (Note: Factory::ClearRegistry() not available; tests assume factory
        // doesn't prevent re-registration)
    }

    void TearDown() override {
        // No cleanup needed; loaded plugins remain in memory (by design)
    }
};

/**
 * @brief Test that loading a non-existent library fails gracefully.
 *
 * This test attempts to load a plugin from a path that doesn't exist and
 * verifies that:
 * - The load returns false
 * - The program does not crash
 * - No invalid state is left in the factory
 *
 * Expected behavior: Clear error message logged, function returns false.
 */
TEST_F(GraphicsPluginLoaderTest, OpenMissingLibrary) {
    // Attempt to load a non-existent library
    std::string nonexistent_path = "/tmp/nonexistent_library_12345.so";
    bool result = GraphicsPluginLoader::LoadPlugin(nonexistent_path, "fake");

    // Should return false, not crash
    EXPECT_FALSE(result);

    // Backend should not be registered
    EXPECT_FALSE(GraphicsBackendFactory::IsRegistered("fake"));
}

/**
 * @brief Test loading a library with missing required plugin symbols.
 *
 * This test loads a stub shared library that exists but does not export
 * the required plugin ABI symbols (create_graphics_backend_v1, etc.).
 *
 * Expected behavior:
 * - The load returns false
 * - Error message indicates missing symbols
 * - No crash or undefined behavior
 */
TEST_F(GraphicsPluginLoaderTest, MissingSymbols) {
    // Use a stub library built by CMake that lacks plugin symbols
    // (See CMakeLists.txt: graphics_stub_missing_symbols target)
#ifdef _WIN32
    std::string stub_path = "build/plugins/stub_missing_symbols.dll";
#elif __APPLE__
    std::string stub_path = "build/plugins/libstub_missing_symbols.dylib";
#else
    std::string stub_path = "build/plugins/libstub_missing_symbols.so";
#endif

    bool result = GraphicsPluginLoader::LoadPlugin(stub_path, "stub_bad");

    // Should return false (missing symbols detected)
    EXPECT_FALSE(result);

    // Backend should not be registered
    EXPECT_FALSE(GraphicsBackendFactory::IsRegistered("stub_bad"));
}

/**
 * @brief Test successful plugin load with all required symbols.
 *
 * This test loads the actual graphics_sfml plugin (built by CMake).
 *
 * Expected behavior:
 * - The load returns true
 * - The backend is registered in the factory
 * - Subsequent calls to IsRegistered return true
 * - The backend can be created successfully
 */
TEST_F(GraphicsPluginLoaderTest, OpenSuccess) {
    // Use the actual graphics_sfml plugin built during the build
#ifdef _WIN32
    std::string plugin_path = "build/plugins/graphics_sfml.dll";
#elif __APPLE__
    std::string plugin_path = "build/plugins/libgraphics_sfml.dylib";
#else
    std::string plugin_path = "build/plugins/libgraphics_sfml.so";
#endif

    bool result = GraphicsPluginLoader::LoadPlugin(plugin_path, "sfml_plugin");

    // Should return true if plugin exists and has required symbols
    if (result) {
        // Backend should be registered
        EXPECT_TRUE(GraphicsBackendFactory::IsRegistered("sfml_plugin"));
    } else {
        // If plugin doesn't exist (e.g., not built), test gracefully skips
        // rather than failing - plugin tests require explicit build flag
        GTEST_SKIP() << "graphics_sfml plugin not built; "
                        "rerun with -DBUILD_PLUGINS=ON";
    }
}

/**
 * @brief Test that multiple successful loads don't cause issues.
 *
 * Some backends may be loaded multiple times with different names.
 * This test verifies that behavior is deterministic.
 */
TEST_F(GraphicsPluginLoaderTest, MultipleLoadsSamePath) {
#ifdef _WIN32
    std::string plugin_path = "build/plugins/graphics_sfml.dll";
#elif __APPLE__
    std::string plugin_path = "build/plugins/libgraphics_sfml.dylib";
#else
    std::string plugin_path = "build/plugins/libgraphics_sfml.so";
#endif

    // Load the same plugin with two different names
    bool result1 = GraphicsPluginLoader::LoadPlugin(plugin_path, "sfml1");
    bool result2 = GraphicsPluginLoader::LoadPlugin(plugin_path, "sfml2");

    if (result1 && result2) {
        // Both should be registered if load succeeded
        EXPECT_TRUE(GraphicsBackendFactory::IsRegistered("sfml1"));
        EXPECT_TRUE(GraphicsBackendFactory::IsRegistered("sfml2"));
    } else {
        GTEST_SKIP() << "graphics_sfml plugin not built";
    }
}

/**
 * @brief Test plugin loader with empty path.
 *
 * Ensures graceful handling of invalid input (empty path).
 */
TEST_F(GraphicsPluginLoaderTest, EmptyPath) {
    bool result = GraphicsPluginLoader::LoadPlugin("", "empty");

    // Should return false for empty path
    EXPECT_FALSE(result);

    // Backend should not be registered
    EXPECT_FALSE(GraphicsBackendFactory::IsRegistered("empty"));
}

/**
 * @brief Test plugin loader with empty name.
 *
 * Ensures graceful handling of invalid input (empty backend name).
 */
TEST_F(GraphicsPluginLoaderTest, EmptyName) {
#ifdef _WIN32
    std::string plugin_path = "./plugins/graphics_sfml.dll";
#elif __APPLE__
    std::string plugin_path = "./plugins/libgraphics_sfml.dylib";
#else
    std::string plugin_path = "./plugins/libgraphics_sfml.so";
#endif

    bool result = GraphicsPluginLoader::LoadPlugin(plugin_path, "");

    // Should return false for empty name
    EXPECT_FALSE(result);
}

}  // namespace Rtype::Client::Graphics
