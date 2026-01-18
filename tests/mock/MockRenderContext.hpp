/**
 * @file MockRenderContext.hpp
 * @brief Mock implementation of IRenderContext for testing.
 *
 * This mock backend does nothing and is used in unit tests that don't require
 * actual rendering. It allows GameWorld to initialize successfully in tests.
 *
 * SCOPE: PR 1.9 - Test infrastructure for new backend ownership model.
 */

#ifndef TESTS_MOCK_MOCKRENDERCONTEXT_HPP_
#define TESTS_MOCK_MOCKRENDERCONTEXT_HPP_

#include "graphics/IRenderContext.hpp"

namespace Rtype::Client::Graphics {

/**
 * @brief No-op mock render context for testing.
 *
 * All operations are no-ops. Used in unit tests where rendering behavior
 * is not relevant to the test outcome.
 */
class MockRenderContext : public Engine::Graphics::IRenderContext {
 public:
    ~MockRenderContext() override = default;

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

    Engine::Graphics::Vector2f GetTextureSize(const char *path) override {
        return Engine::Graphics::Vector2f(0.0f, 0.0f);
    }

    Engine::Graphics::Vector2f GetTextBounds(
        const char *font_path, const char *text, unsigned int size) override {
        return Engine::Graphics::Vector2f(0.0f, 0.0f);
    }

    Engine::Graphics::Vector2i GetGridFrameSize(
        const char *texture_path, int grid_cols, int frame_width) override {
        return Engine::Graphics::Vector2i(0, 0);
    }
};

}  // namespace Rtype::Client::Graphics

#endif  // TESTS_MOCK_MOCKRENDERCONTEXT_HPP_
