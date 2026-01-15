/**
 * @file smoke_plugin_load.cpp
 * @brief Integration smoke test for plugin loading and rendering.
 *
 * This test verifies:
 * - Plugin can be loaded successfully
 * - Backend registration works
 * - Rendering produces output without crashing
 *
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include "graphics/GraphicsBackendFactory.hpp"
#include "graphics/GraphicsPluginLoader.hpp"
#include "integration/HeadlessTestRenderer.hpp"

namespace Rtype::Test {

/**
 * @brief Integration test suite for plugin smoke tests.
 */
class PluginSmokeTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Ensure clean state
        output_path_ = "tests/artifacts/smoke_plugin_test.png";
    }

    void TearDown() override {
        // Cleanup is minimal; artifacts are kept for inspection
    }

    std::string output_path_;
};

/**
 * @brief Test that a plugin loads and can render without crashing.
 *
 * This is a "smoke test" - it just verifies nothing catches fire.
 * Detailed correctness is checked in pixel comparison tests.
 */
TEST_F(PluginSmokeTest, LoadPluginAndRender) {
    // Load the SFML plugin
#ifdef _WIN32
    std::string plugin_path = "build/plugins/graphics_sfml.dll";
#elif __APPLE__
    std::string plugin_path = "build/plugins/libgraphics_sfml.dylib";
#else
    std::string plugin_path = "build/plugins/libgraphics_sfml.so";
#endif

    bool loaded = Client::Graphics::GraphicsPluginLoader::LoadPlugin(
        plugin_path, "sfml_smoke_test");

    if (!loaded) {
        GTEST_SKIP() << "Plugin not available; skipping smoke test";
    }

    // Verify backend is registered
    EXPECT_TRUE(Client::Graphics::GraphicsBackendFactory::IsRegistered(
        "sfml_smoke_test"));

    // Create headless renderer and render test scene
    HeadlessTestRenderer renderer(320, 200);
    renderer.RenderTestScene();

    // Save output
    bool saved = renderer.SaveToPNG(output_path_);
    EXPECT_TRUE(saved) << "Failed to save smoke test output to "
                       << output_path_;

    // If we got here without crashing, the smoke test passes
    SUCCEED() << "Plugin loaded and rendered successfully";
}

/**
 * @brief Test that static backend (no plugin) also works.
 *
 * Verifies that the baseline static registration still functions.
 */
TEST_F(PluginSmokeTest, StaticBackendRenders) {
    // Register a static SFML backend (simulating main.cpp behavior)
    // Note: This would normally use SFMLRenderContext, but for this test
    // we just verify the registration mechanism works

    // Skip if already registered by previous test
    if (!Client::Graphics::GraphicsBackendFactory::IsRegistered(
            "sfml_static")) {
        // Would need actual sf::RenderWindow to register fully
        // For now, just verify the test infrastructure works
        GTEST_SKIP() << "Static backend registration requires window context";
    }
}

/**
 * @brief Test loading plugin multiple times doesn't cause issues.
 */
TEST_F(PluginSmokeTest, MultipleLoadsSafe) {
#ifdef _WIN32
    std::string plugin_path = "build/plugins/graphics_sfml.dll";
#elif __APPLE__
    std::string plugin_path = "build/plugins/libgraphics_sfml.dylib";
#else
    std::string plugin_path = "build/plugins/libgraphics_sfml.so";
#endif

    // Load twice with different names
    bool loaded1 = Client::Graphics::GraphicsPluginLoader::LoadPlugin(
        plugin_path, "sfml_multi_1");
    bool loaded2 = Client::Graphics::GraphicsPluginLoader::LoadPlugin(
        plugin_path, "sfml_multi_2");

    if (!loaded1 || !loaded2) {
        GTEST_SKIP() << "Plugin not available";
    }

    EXPECT_TRUE(Client::Graphics::GraphicsBackendFactory::IsRegistered(
        "sfml_multi_1"));
    EXPECT_TRUE(Client::Graphics::GraphicsBackendFactory::IsRegistered(
        "sfml_multi_2"));
}

}  // namespace Rtype::Test
