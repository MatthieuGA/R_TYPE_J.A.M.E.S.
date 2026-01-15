/**
 * @file HeadlessTestRenderer.hpp
 * @brief Minimal headless renderer for testing plugin loading and rendering.
 *
 * This test helper creates a minimal rendering context without opening a
 * window, renders a deterministic test scene to an offscreen buffer, and saves
 * it as PNG.
 *
 * SCOPE: Phase E - Integration testing (test-only helper)
 */

#ifndef TESTS_INTEGRATION_HEADLESSTESTRENDERER_HPP_
#define TESTS_INTEGRATION_HEADLESSTESTRENDERER_HPP_

#include <memory>
#include <string>

#include <SFML/Graphics.hpp>

namespace Rtype::Test {

/**
 * @brief Headless renderer for smoke tests.
 *
 * Creates an offscreen render target, renders a simple deterministic scene,
 * and saves the result to a PNG file for comparison testing.
 */
class HeadlessTestRenderer {
 public:
    /**
     * @brief Construct a headless renderer with specified dimensions.
     *
     * @param width Render target width in pixels
     * @param height Render target height in pixels
     */
    explicit HeadlessTestRenderer(
        unsigned int width = 320, unsigned int height = 200)
        : width_(width), height_(height) {
        // Create offscreen render texture
        if (!render_texture_.create(width, height)) {
            throw std::runtime_error("Failed to create render texture");
        }
    }

    /**
     * @brief Render a deterministic test scene.
     *
     * Draws simple geometric shapes at fixed positions with fixed colors.
     * This scene is completely deterministic (no randomness, no time-based
     * animation) to enable pixel-perfect comparison.
     */
    void RenderTestScene() {
        // Clear to a known background color
        render_texture_.clear(sf::Color(32, 32, 48));

        // Draw test pattern: three colored rectangles
        sf::RectangleShape rect1(sf::Vector2f(80, 60));
        rect1.setPosition(40, 40);
        rect1.setFillColor(sf::Color(255, 100, 100));  // Red
        render_texture_.draw(rect1);

        sf::RectangleShape rect2(sf::Vector2f(80, 60));
        rect2.setPosition(140, 40);
        rect2.setFillColor(sf::Color(100, 255, 100));  // Green
        render_texture_.draw(rect2);

        sf::RectangleShape rect3(sf::Vector2f(80, 60));
        rect3.setPosition(90, 100);
        rect3.setFillColor(sf::Color(100, 100, 255));  // Blue
        render_texture_.draw(rect3);

        // Display (finalize rendering)
        render_texture_.display();
    }

    /**
     * @brief Save the rendered image to a PNG file.
     *
     * @param filepath Output PNG path
     * @return True if save succeeded, false otherwise
     */
    bool SaveToPNG(const std::string &filepath) {
        sf::Image image = render_texture_.getTexture().copyToImage();
        return image.saveToFile(filepath);
    }

    /**
     * @brief Get the render target width.
     */
    unsigned int GetWidth() const {
        return width_;
    }

    /**
     * @brief Get the render target height.
     */
    unsigned int GetHeight() const {
        return height_;
    }

 private:
    unsigned int width_;
    unsigned int height_;
    sf::RenderTexture render_texture_;
};

}  // namespace Rtype::Test

#endif  // TESTS_INTEGRATION_HEADLESSTESTRENDERER_HPP_
