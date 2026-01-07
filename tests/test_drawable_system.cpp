#include <gtest/gtest.h>

#include <memory>
#include <utility>

#include "engine/GameWorld.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "platform/SFMLWindow.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

TEST(DrawableSystem, LoadsAndAppliesTransform) {
    Eng::registry reg;
    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::Drawable> drawables;
    Eng::sparse_array<Com::Shader> shaders;
    Eng::sparse_array<Com::AnimatedSprite> animated_sprites;
    Eng::sparse_array<Com::ParticleEmitter> emitters;

    Com::Transform transform{15.0f, -4.0f, 22.0f, 1.5f};
    Com::Drawable drawable{"Logo.png", 4, 0.6f};
    drawable.color = Engine::Graphics::Color::Green;

    transforms.insert_at(0, transform);
    drawables.insert_at(0, std::move(drawable));

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    Rtype::Client::GameWorld game_world(
        std::move(window), "127.0.0.1", 50000, 50000);

    DrawableSystem(reg, game_world, transforms, drawables, shaders,
        animated_sprites, emitters);

    ASSERT_TRUE(drawables[0].has_value());
    const auto &rendered = drawables[0].value();
    EXPECT_TRUE(rendered.isLoaded);
    EXPECT_FLOAT_EQ(rendered.sprite.getPosition().x, transform.x);
    EXPECT_FLOAT_EQ(rendered.sprite.getPosition().y, transform.y);
    EXPECT_FLOAT_EQ(rendered.sprite.getScale().x, transform.scale.x);
    EXPECT_FLOAT_EQ(rendered.sprite.getScale().y, transform.scale.y);
    EXPECT_FLOAT_EQ(rendered.sprite.getRotation(), transform.rotationDegrees);
    EXPECT_EQ(rendered.sprite.getColor().a,
        static_cast<sf::Uint8>(rendered.opacity * 255));
    EXPECT_EQ(rendered.sprite.getColor().g, sf::Color::Green.g);
}

TEST(DrawableSystem, HandlesMultipleEntitiesSortedByZIndex) {
    Eng::registry reg;
    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::Drawable> drawables;
    Eng::sparse_array<Com::Shader> shaders;

    transforms.insert_at(0, Com::Transform{0.0f, 0.0f, 0.0f, 1.0f});
    transforms.insert_at(1, Com::Transform{1.0f, 2.0f, 0.0f, 1.0f});

    Com::Drawable d0{"Logo.png", 5};
    Com::Drawable d1{"Logo.png", 1};
    drawables.insert_at(0, std::move(d0));
    drawables.insert_at(1, std::move(d1));

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");

    Rtype::Client::GameWorld game_world(
        std::move(window), "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::AnimatedSprite> animated_sprites;
    Eng::sparse_array<Com::ParticleEmitter> emitters;
    DrawableSystem(reg, game_world, transforms, drawables, shaders,
        animated_sprites, emitters);

    ASSERT_TRUE(drawables[0].has_value());
    ASSERT_TRUE(drawables[1].has_value());
    EXPECT_TRUE(drawables[0]->isLoaded);
    EXPECT_TRUE(drawables[1]->isLoaded);

    // After processing, transforms are applied to positions
    EXPECT_FLOAT_EQ(drawables[0]->sprite.getPosition().x, 0.0f);
    EXPECT_FLOAT_EQ(drawables[0]->sprite.getPosition().y, 0.0f);
    EXPECT_FLOAT_EQ(drawables[1]->sprite.getPosition().x, 1.0f);
    EXPECT_FLOAT_EQ(drawables[1]->sprite.getPosition().y, 2.0f);
}
