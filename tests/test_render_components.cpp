#include <gtest/gtest.h>

#include <string>
#include <utility>

#include "include/components/RenderComponent.hpp"

namespace Com = Rtype::Client::Component;

TEST(RenderComponents, DrawableBasicsAndMove) {
    Com::Drawable d{"Logo.png", 3, 0.8f};
    EXPECT_EQ(d.sprite_path, std::string("assets/images/Logo.png"));
    EXPECT_EQ(d.z_index, 3);
    EXPECT_FLOAT_EQ(d.opacity, 0.8f);
    EXPECT_FALSE(d.is_loaded);

    Com::Drawable moved{std::move(d)};
    EXPECT_EQ(moved.sprite_path, std::string("assets/images/Logo.png"));
    EXPECT_EQ(moved.z_index, 3);
    EXPECT_FLOAT_EQ(moved.opacity, 0.8f);
    EXPECT_FALSE(moved.is_loaded);
}

TEST(RenderComponents, ShaderPathAndUniforms) {
    Com::Shader shader{
        "wave.frag", {{"timeScale", 1.5f}, {"amplitude", 0.7f}}};
    EXPECT_EQ(shader.shader_path, std::string("assets/shaders/wave.frag"));
    EXPECT_FALSE(shader.is_loaded);
    ASSERT_EQ(shader.uniforms_float.size(), 2u);
    EXPECT_FLOAT_EQ(shader.uniforms_float.at("timeScale"), 1.5f);
    EXPECT_FLOAT_EQ(shader.uniforms_float.at("amplitude"), 0.7f);
}

TEST(RenderComponents, TextDefaults) {
    Com::Text text{"dogica.ttf"};
    EXPECT_EQ(text.font_path, std::string("assets/fonts/dogica.ttf"));
    EXPECT_EQ(text.content, std::string(""));
    EXPECT_EQ(text.character_size, 30u);
    EXPECT_EQ(text.color, Engine::Graphics::Color::White);
    EXPECT_FLOAT_EQ(text.opacity, 1.0f);
    EXPECT_EQ(text.z_index, 0);
    EXPECT_FALSE(text.is_loaded);
    EXPECT_FLOAT_EQ(text.offset.x, 0.0f);
    EXPECT_FLOAT_EQ(text.offset.y, 0.0f);
}

TEST(RenderComponents, TextMoveSemantics) {
    Com::Text src{"dogica.ttf", "Hello", 42, 2,
        Engine::Graphics::Color::Yellow,
        Engine::Graphics::Vector2f(3.0f, -1.0f)};
    src.opacity = 0.25f;

    Com::Text moved{std::move(src)};
    EXPECT_EQ(moved.font_path, std::string("assets/fonts/dogica.ttf"));
    EXPECT_EQ(moved.content, std::string("Hello"));
    EXPECT_EQ(moved.character_size, 42u);
    EXPECT_EQ(moved.color, Engine::Graphics::Color::Yellow);
    EXPECT_FLOAT_EQ(moved.opacity, 0.25f);
    EXPECT_EQ(moved.z_index, 2);
    EXPECT_FLOAT_EQ(moved.offset.x, 3.0f);
    EXPECT_FLOAT_EQ(moved.offset.y, -1.0f);
}
