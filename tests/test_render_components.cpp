#include <gtest/gtest.h>

#include "../client/include/Components/RenderComponent.hpp"

namespace Com = Rtype::Client::Component;

TEST(RenderComponents, DrawableBasicsAndMove) {
    Com::Drawable d{"Logo.png", 3, 0.8f};
    EXPECT_EQ(d.spritePath, std::string("Assets/Images/Logo.png"));
    EXPECT_EQ(d.z_index, 3);
    EXPECT_FLOAT_EQ(d.opacity, 0.8f);
    EXPECT_FALSE(d.isLoaded);

    Com::Drawable moved{std::move(d)};
    EXPECT_EQ(moved.spritePath, std::string("Assets/Images/Logo.png"));
    EXPECT_EQ(moved.z_index, 3);
    EXPECT_FLOAT_EQ(moved.opacity, 0.8f);
    EXPECT_FALSE(moved.isLoaded);
}

TEST(RenderComponents, ShaderPathAndUniforms) {
    Com::Shader shader{"wave.frag", {{"timeScale", 1.5f}, {"amplitude", 0.7f}}};
    EXPECT_EQ(shader.shaderPath, std::string("Assets/Shaders/wave.frag"));
    EXPECT_FALSE(shader.isLoaded);
    ASSERT_EQ(shader.uniforms_float.size(), 2u);
    EXPECT_FLOAT_EQ(shader.uniforms_float.at("timeScale"), 1.5f);
    EXPECT_FLOAT_EQ(shader.uniforms_float.at("amplitude"), 0.7f);
}

TEST(RenderComponents, AnimatedSpriteCtorLooping) {
    Com::AnimatedSprite anim{32, 16, 0.2f, true, sf::Vector2f(4.0f, 2.0f), 6};
    EXPECT_EQ(anim.frameWidth, 32);
    EXPECT_EQ(anim.frameHeight, 16);
    EXPECT_EQ(anim.totalFrames, 6);
    EXPECT_EQ(anim.currentFrame, 0);
    EXPECT_FLOAT_EQ(anim.frameDuration, 0.2f);
    EXPECT_TRUE(anim.loop);
    EXPECT_TRUE(anim.animated);
    EXPECT_FLOAT_EQ(anim.first_frame_position.x, 4.0f);
    EXPECT_FLOAT_EQ(anim.first_frame_position.y, 2.0f);
}

TEST(RenderComponents, AnimatedSpriteSingleFrame) {
    Com::AnimatedSprite anim{64, 64, 5};
    EXPECT_EQ(anim.frameWidth, 64);
    EXPECT_EQ(anim.frameHeight, 64);
    EXPECT_EQ(anim.currentFrame, 5);
    EXPECT_FALSE(anim.animated);
    EXPECT_EQ(anim.totalFrames, 0);
    EXPECT_TRUE(anim.loop);
}

TEST(RenderComponents, TextDefaults) {
    Com::Text text{"dogica.ttf"};
    EXPECT_EQ(text.fontPath, std::string("Assets/Fonts/dogica.ttf"));
    EXPECT_EQ(text.content, std::string(""));
    EXPECT_EQ(text.characterSize, 30u);
    EXPECT_EQ(text.color, sf::Color::White);
    EXPECT_FLOAT_EQ(text.opacity, 1.0f);
    EXPECT_EQ(text.z_index, 0);
    EXPECT_FALSE(text.is_loaded);
    EXPECT_FLOAT_EQ(text.offset.x, 0.0f);
    EXPECT_FLOAT_EQ(text.offset.y, 0.0f);
}

TEST(RenderComponents, TextMoveSemantics) {
    Com::Text src{"dogica.ttf", "Hello", 42, 2, sf::Color::Yellow,
        sf::Vector2f(3.0f, -1.0f)};
    src.opacity = 0.25f;

    Com::Text moved{std::move(src)};
    EXPECT_EQ(moved.fontPath, std::string("Assets/Fonts/dogica.ttf"));
    EXPECT_EQ(moved.content, std::string("Hello"));
    EXPECT_EQ(moved.characterSize, 42u);
    EXPECT_EQ(moved.color, sf::Color::Yellow);
    EXPECT_FLOAT_EQ(moved.opacity, 0.25f);
    EXPECT_EQ(moved.z_index, 2);
    EXPECT_FLOAT_EQ(moved.offset.x, 3.0f);
    EXPECT_FLOAT_EQ(moved.offset.y, -1.0f);
}
