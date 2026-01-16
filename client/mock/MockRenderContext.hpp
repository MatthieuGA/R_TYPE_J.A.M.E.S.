/**
 * @file MockRenderContext.hpp
 * @brief Mock implementation of IRenderContext for testing.
 *
 * SCOPE: PR 1.9 - Test infrastructure.
 */

#ifndef CLIENT_MOCK_MOCKRENDERCONTEXT_HPP_
#define CLIENT_MOCK_MOCKRENDERCONTEXT_HPP_

#include "graphics/IRenderContext.hpp"

namespace Rtype::Client::Graphics {

/**
 * @brief Mock render context that does nothing (for testing).
 *
 * All drawing operations are no-ops. Query methods return sensible defaults.
 * Useful for testing systems that depend on IRenderContext without needing
 * actual SFML rendering.
 */
class MockRenderContext : public Engine::Graphics::IRenderContext {
 public:
    MockRenderContext() = default;
    ~MockRenderContext() override = default;

    // All drawing operations are no-ops
    void DrawSprite(const Engine::Graphics::DrawableSprite &sprite,
        const Engine::Graphics::DrawableShader *shader = nullptr) override {
        // No-op
    }

    void DrawText(const Engine::Graphics::DrawableText &text) override {
        // No-op
    }

    void DrawRectangle(
        const Engine::Graphics::DrawableRectangle &rect) override {
        // No-op
    }

    void DrawVertexArray(
        const Engine::Graphics::VertexArray &vertices) override {
        // No-op
    }

    /**
     * @brief Return zero size for all textures (mock behavior).
     *
     * @param path Path to texture file (ignored)
     * @return Zero vector (0, 0)
     */
    Engine::Graphics::Vector2f GetTextureSize(const char *path) override {
        return Engine::Graphics::Vector2f(0.0f, 0.0f);
    }

    /**
     * @brief Return zero bounds for all text (mock behavior).
     *
     * @param font_path Path to font file (ignored)
     * @param text Text string (ignored)
     * @param size Character size (ignored)
     * @return Zero vector (0, 0)
     */
    Engine::Graphics::Vector2f GetTextBounds(
        const char *font_path, const char *text, unsigned int size) override {
        return Engine::Graphics::Vector2f(0.0f, 0.0f);
    }

    /**
     * @brief Return default frame size (mock behavior).
     *
     * @param texture_path Path to texture file (ignored)
     * @param grid_cols Number of columns (ignored)
     * @param frame_width Width of each frame (returned as-is)
     * @return Vector2i {frame_width, frame_width}
     */
    Engine::Graphics::Vector2i GetGridFrameSize(
        const char *texture_path, int grid_cols, int frame_width) override {
        return Engine::Graphics::Vector2i(frame_width, frame_width);
    }
};

}  // namespace Rtype::Client::Graphics

#endif  // CLIENT_MOCK_MOCKRENDERCONTEXT_HPP_
