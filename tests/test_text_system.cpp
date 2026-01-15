#include <gtest/gtest.h>

#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include "engine/GameWorld.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "platform/SFMLWindow.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

namespace {
std::string GetFontAbsolutePath(const std::string &font_name) {
    auto source_path = std::filesystem::path(__FILE__);
    auto root = source_path.parent_path().parent_path();
    auto font =
        (root / "client" / "assets" / "fonts" / font_name).lexically_normal();
    return font.string();
}
}  // namespace

TEST(TextComponent, Defaults) {
    Com::Text text("dogica.ttf");
    EXPECT_EQ(text.content, std::string(""));
    EXPECT_EQ(text.fontPath, std::string("assets/fonts/dogica.ttf"));
    EXPECT_EQ(text.characterSize, 30u);
    EXPECT_EQ(text.color, Engine::Graphics::Color::White);
    EXPECT_FLOAT_EQ(text.opacity, 1.0f);
    EXPECT_EQ(text.z_index, 0);
    EXPECT_FLOAT_EQ(text.offset.x, 0.0f);
    EXPECT_FLOAT_EQ(text.offset.y, 0.0f);
    EXPECT_FALSE(text.is_loaded);
}

TEST(TextRenderSystem, LoadsAndAppliesTransform) {
    Eng::registry reg;
    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::Text> texts;

    Com::Transform transform{10.0f, 20.0f, 45.0f, 2.0f};
    Com::Text text("dogica.ttf", "Hello", 16, 3, Engine::Graphics::Color::Red,
        sf::Vector2f(5.0f, -3.0f));
    text.fontPath = GetFontAbsolutePath("dogica.ttf");
    text.opacity = 0.5f;

    transforms.insert_at(0, transform);
    texts.insert_at(0, std::move(text));

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        10, 10, "text-test");
    Rtype::Client::GameWorld game_world(
        std::move(window), "127.0.0.1", 50000, 50000);

    DrawTextRenderSystem(reg, game_world, transforms, texts);

    ASSERT_TRUE(texts[0].has_value());
    const auto &rendered = texts[0].value();

    EXPECT_TRUE(rendered.is_loaded);
    EXPECT_NE(rendered.text.getFont(), nullptr);
    EXPECT_EQ(rendered.text.getString(), sf::String("Hello"));
    EXPECT_FLOAT_EQ(rendered.text.getRotation(), transform.rotationDegrees);
    EXPECT_NEAR(rendered.text.getScale().x, 0.2f, 1e-5f);
    EXPECT_NEAR(rendered.text.getScale().y, 0.2f, 1e-5f);
    EXPECT_FLOAT_EQ(rendered.text.getPosition().x, transform.x + 5.0f);
    EXPECT_FLOAT_EQ(rendered.text.getPosition().y, transform.y - 3.0f);
    EXPECT_EQ(rendered.text.getFillColor().a,
        static_cast<sf::Uint8>(rendered.opacity * 255));
}
