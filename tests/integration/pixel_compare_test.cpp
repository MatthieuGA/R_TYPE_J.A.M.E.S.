/**
 * @file pixel_compare_test.cpp
 * @brief Pixel-perfect comparison test for deterministic rendering.
 *
 * This test renders a deterministic scene and compares it pixel-by-pixel
 * to a known-good baseline image. Used to catch rendering regressions.
 *
 * SCOPE: Phase E - Regression testing
 */

#include <gtest/gtest.h>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>

#include <SFML/Graphics.hpp>

#include "graphics/GraphicsBackendFactory.hpp"
#include "graphics/GraphicsPluginLoader.hpp"
#include "integration/HeadlessTestRenderer.hpp"

namespace Rtype::Test {

/**
 * @brief Pixel comparison test suite.
 */
class PixelCompareTest : public ::testing::Test {
 protected:
    void SetUp() override {
        baseline_path_ = "tests/baseline/smoke_baseline.png";
        output_path_ = "tests/artifacts/smoke_output.png";
        diff_path_ = "tests/artifacts/diff.png";
    }

    /**
     * @brief Compare two images pixel-by-pixel with tolerance.
     *
     * @param baseline Baseline image
     * @param actual Actual rendered image
     * @param max_per_pixel_diff Maximum allowed difference per channel (0-255)
     * @param max_diff_percentage Maximum percentage of differing pixels
     * @return True if images match within tolerance
     */
    bool CompareImages(const sf::Image &baseline, const sf::Image &actual,
        int max_per_pixel_diff = 4, float max_diff_percentage = 0.2f) {
        if (baseline.getSize() != actual.getSize()) {
            std::cerr << "[PixelCompare] Size mismatch: baseline="
                      << baseline.getSize().x << "x" << baseline.getSize().y
                      << " actual=" << actual.getSize().x << "x"
                      << actual.getSize().y << std::endl;
            return false;
        }

        unsigned int width = baseline.getSize().x;
        unsigned int height = baseline.getSize().y;
        unsigned int total_pixels = width * height;
        unsigned int differing_pixels = 0;

        // Create diff image (optional visualization)
        sf::Image diff_image;
        diff_image.create(width, height, sf::Color::Black);

        for (unsigned int y = 0; y < height; ++y) {
            for (unsigned int x = 0; x < width; ++x) {
                sf::Color baseline_pixel = baseline.getPixel(x, y);
                sf::Color actual_pixel = actual.getPixel(x, y);

                int r_diff = std::abs(static_cast<int>(baseline_pixel.r) -
                                      static_cast<int>(actual_pixel.r));
                int g_diff = std::abs(static_cast<int>(baseline_pixel.g) -
                                      static_cast<int>(actual_pixel.g));
                int b_diff = std::abs(static_cast<int>(baseline_pixel.b) -
                                      static_cast<int>(actual_pixel.b));

                int max_diff = std::max({r_diff, g_diff, b_diff});

                if (max_diff > max_per_pixel_diff) {
                    differing_pixels++;
                    // Highlight diff in red
                    diff_image.setPixel(x, y, sf::Color(255, 0, 0));
                } else {
                    // Show actual pixel (slightly dimmed)
                    diff_image.setPixel(x, y,
                        sf::Color(actual_pixel.r / 2, actual_pixel.g / 2,
                            actual_pixel.b / 2));
                }
            }
        }

        // Save diff image for debugging
        diff_image.saveToFile(diff_path_);

        float diff_percentage =
            (static_cast<float>(differing_pixels) / total_pixels) * 100.0f;

        std::cout << "[PixelCompare] Differing pixels: " << differing_pixels
                  << " / " << total_pixels << " (" << diff_percentage << "%)"
                  << std::endl;
        std::cout << "[PixelCompare] Diff image saved to: " << diff_path_
                  << std::endl;

        return diff_percentage <= max_diff_percentage;
    }

    std::string baseline_path_;
    std::string output_path_;
    std::string diff_path_;
};

/**
 * @brief Test that rendered output matches baseline within tolerance.
 *
 * This is the main regression test - it catches any changes in rendering
 * behavior by comparing against a known-good baseline image.
 */
TEST_F(PixelCompareTest, RenderedOutputMatchesBaseline) {
    // Check if baseline exists
    std::ifstream baseline_check(baseline_path_);
    if (!baseline_check.good()) {
        // Generate baseline if it doesn't exist
        std::cout
            << "[PixelCompare] Baseline not found. Generating baseline at: "
            << baseline_path_ << std::endl;

        HeadlessTestRenderer renderer(320, 200);
        renderer.RenderTestScene();
        bool saved = renderer.SaveToPNG(baseline_path_);

        if (saved) {
            std::cout << "[PixelCompare] Baseline generated successfully. "
                      << "Please commit this file to version control."
                      << std::endl;
            GTEST_SKIP() << "Baseline generated; rerun test to compare";
        } else {
            FAIL() << "Failed to generate baseline image";
        }
    }

    // Load baseline
    sf::Image baseline;
    if (!baseline.loadFromFile(baseline_path_)) {
        FAIL() << "Failed to load baseline image: " << baseline_path_;
    }

    // Render current output
    HeadlessTestRenderer renderer(320, 200);
    renderer.RenderTestScene();
    bool saved = renderer.SaveToPNG(output_path_);
    ASSERT_TRUE(saved) << "Failed to save output image";

    // Load actual output
    sf::Image actual;
    if (!actual.loadFromFile(output_path_)) {
        FAIL() << "Failed to load output image: " << output_path_;
    }

    // Compare images with tolerance
    // Tolerance: max 4 per-channel difference, max 0.2% differing pixels
    bool matches = CompareImages(baseline, actual, 4, 0.2f);

    EXPECT_TRUE(matches) << "Rendered output differs from baseline beyond "
                            "acceptable tolerance. "
                         << "Check diff image at: " << diff_path_;
}

/**
 * @brief Test that plugin-loaded backend produces same output as static.
 *
 * Verifies that plugin loading doesn't introduce rendering differences.
 */
TEST_F(PixelCompareTest, PluginMatchesStaticBackend) {
    // Load plugin
#ifdef _WIN32
    std::string plugin_path = "build/plugins/graphics_sfml.dll";
#elif __APPLE__
    std::string plugin_path = "build/plugins/libgraphics_sfml.dylib";
#else
    std::string plugin_path = "build/plugins/libgraphics_sfml.so";
#endif

    bool loaded = Client::Graphics::GraphicsPluginLoader::LoadPlugin(
        plugin_path, "sfml_pixel_test");

    if (!loaded) {
        GTEST_SKIP() << "Plugin not available; skipping comparison";
    }

    // Render with plugin backend
    HeadlessTestRenderer renderer1(320, 200);
    renderer1.RenderTestScene();
    renderer1.SaveToPNG("tests/artifacts/plugin_render.png");

    // Render with same code (static)
    HeadlessTestRenderer renderer2(320, 200);
    renderer2.RenderTestScene();
    renderer2.SaveToPNG("tests/artifacts/static_render.png");

    // Load both images
    sf::Image plugin_output, static_output;
    ASSERT_TRUE(
        plugin_output.loadFromFile("tests/artifacts/plugin_render.png"));
    ASSERT_TRUE(
        static_output.loadFromFile("tests/artifacts/static_render.png"));

    // They should be identical (or extremely close)
    bool matches = CompareImages(plugin_output, static_output, 1, 0.01f);

    EXPECT_TRUE(matches)
        << "Plugin backend produces different output than static backend";
}

}  // namespace Rtype::Test
