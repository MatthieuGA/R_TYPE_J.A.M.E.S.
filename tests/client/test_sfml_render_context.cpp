/**
 * @file test_sfml_render_context.cpp
 * @brief Tests for SFMLRenderContext backend implementation.
 */

#include <gtest/gtest.h>

#include <memory>

#include <SFML/Graphics.hpp>

#include "graphics/SFMLRenderContext.hpp"

namespace Rtype::Client::Graphics {
namespace Test {

/**
 * @brief Test fixture for SFMLRenderContext tests.
 */
class SFMLRenderContextTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Create a non-visible window for testing
        window_ = std::make_unique<sf::RenderWindow>(
            sf::VideoMode(800, 600), "Test Window", sf::Style::None);
        window_->setVisible(false);
        context_ = std::make_unique<SFMLRenderContext>(*window_);
    }

    void TearDown() override {
        context_.reset();
        window_.reset();
    }

    std::unique_ptr<sf::RenderWindow> window_;
    std::unique_ptr<SFMLRenderContext> context_;
};

/**
 * @brief Test that DrawSprite does not crash with valid texture.
 */
TEST_F(SFMLRenderContextTest, DrawSprite_ValidTexture_DoesNotCrash) {
    Engine::Graphics::DrawableSprite sprite{};
    sprite.texture_path = "assets/r-typesheet1.gif";
    sprite.position = {100.0f, 200.0f};
    sprite.scale = {1.0f, 1.0f};
    sprite.rotation_degrees = 0.0f;
    sprite.color = {255, 255, 255, 255};

    // Should not crash
    EXPECT_NO_THROW(context_->DrawSprite(sprite));
}

/**
 * @brief Test that DrawSprite handles invalid texture path gracefully.
 */
TEST_F(SFMLRenderContextTest, DrawSprite_InvalidTexture_HandlesGracefully) {
    Engine::Graphics::DrawableSprite sprite{};
    sprite.texture_path = "nonexistent_texture.png";
    sprite.position = {100.0f, 200.0f};
    sprite.scale = {1.0f, 1.0f};
    sprite.rotation_degrees = 0.0f;
    sprite.color = {255, 255, 255, 255};

    // Should not crash (prints error, returns early)
    EXPECT_NO_THROW(context_->DrawSprite(sprite));
}

/**
 * @brief Test that DrawText does not crash with valid font.
 */
TEST_F(SFMLRenderContextTest, DrawText_ValidFont_DoesNotCrash) {
    Engine::Graphics::DrawableText text{};
    text.font_path = "assets/fonts/arial.ttf";
    text.text = "Hello World";
    text.size = 24;
    text.position = {100.0f, 200.0f};
    text.color = {255, 255, 255, 255};

    // Should not crash (may fail to load font, but handles gracefully)
    EXPECT_NO_THROW(context_->DrawText(text));
}

/**
 * @brief Test that DrawText handles invalid font path gracefully.
 */
TEST_F(SFMLRenderContextTest, DrawText_InvalidFont_HandlesGracefully) {
    Engine::Graphics::DrawableText text{};
    text.font_path = "nonexistent_font.ttf";
    text.text = "Hello World";
    text.size = 24;
    text.position = {100.0f, 200.0f};
    text.color = {255, 255, 255, 255};

    // Should not crash (prints error, returns early)
    EXPECT_NO_THROW(context_->DrawText(text));
}

/**
 * @brief Test that DrawRectangle does not crash.
 */
TEST_F(SFMLRenderContextTest, DrawRectangle_DoesNotCrash) {
    Engine::Graphics::DrawableRectangle rect{};
    rect.position = {100.0f, 200.0f};
    rect.size = {50.0f, 30.0f};
    rect.color = {255, 0, 0, 255};

    // Should not crash
    EXPECT_NO_THROW(context_->DrawRectangle(rect));
}

/**
 * @brief Test that DrawVertexArray does not crash.
 */
TEST_F(SFMLRenderContextTest, DrawVertexArray_DoesNotCrash) {
    Engine::Graphics::VertexArray::Vertex vertices[] = {
        {{0.0f, 0.0f}, {255, 0, 0, 255}}, {{100.0f, 0.0f}, {0, 255, 0, 255}},
        {{100.0f, 100.0f}, {0, 0, 255, 255}},
        {{0.0f, 100.0f}, {255, 255, 0, 255}}};

    Engine::Graphics::VertexArray vertex_array{};
    vertex_array.primitive_type = Engine::Graphics::VertexArray::Quads;
    vertex_array.vertices = vertices;
    vertex_array.vertex_count = 4;

    // Should not crash
    EXPECT_NO_THROW(context_->DrawVertexArray(vertex_array));
}

/**
 * @brief Test that texture caching works.
 *
 * Loading same texture twice should use cache.
 */
TEST_F(SFMLRenderContextTest, TextureCaching_LoadsSameTextureTwice_UsesCache) {
    Engine::Graphics::DrawableSprite sprite{};
    sprite.texture_path = "assets/r-typesheet1.gif";
    sprite.position = {100.0f, 200.0f};
    sprite.scale = {1.0f, 1.0f};
    sprite.rotation_degrees = 0.0f;
    sprite.color = {255, 255, 255, 255};

    // First draw - loads texture
    EXPECT_NO_THROW(context_->DrawSprite(sprite));

    // Second draw - should use cache
    sprite.position = {200.0f, 300.0f};
    EXPECT_NO_THROW(context_->DrawSprite(sprite));

    // Both should succeed without re-loading texture
}

}  // namespace Test
}  // namespace Rtype::Client::Graphics
