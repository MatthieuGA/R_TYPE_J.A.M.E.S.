#include <gtest/gtest.h>

#include <memory>
#include <string>
#include <utility>

#include "engine/GameWorld.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "platform/SFMLWindow.hpp"
#include "tests/TestGraphicsSetup.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

TEST(DrawableSystem, LoadsAndAppliesTransform) {
    TestHelper::RegisterTestBackend();

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
        std::move(window), "test", "127.0.0.1", 50000, 50000);

    DrawableSystem(reg, game_world, transforms, drawables, shaders,
        animated_sprites, emitters);

    ASSERT_TRUE(drawables[0].has_value());
    const auto &rendered = drawables[0].value();
    EXPECT_FALSE(rendered.is_loaded);  // No SFML loading in Phase 2
    EXPECT_EQ(rendered.texture_path, std::string("assets/images/Logo.png"));
    EXPECT_FLOAT_EQ(rendered.opacity, 0.6f);
    EXPECT_EQ(rendered.color.r, Engine::Graphics::Color::Green.r);
}

TEST(DrawableSystem, HandlesMultipleEntitiesSortedByZIndex) {
    TestHelper::RegisterTestBackend();

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
        std::move(window), "test", "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::AnimatedSprite> animated_sprites;
    Eng::sparse_array<Com::ParticleEmitter> emitters;
    DrawableSystem(reg, game_world, transforms, drawables, shaders,
        animated_sprites, emitters);

    ASSERT_TRUE(drawables[0].has_value());
    ASSERT_TRUE(drawables[1].has_value());
    EXPECT_FALSE(drawables[0]->is_loaded);  // Phase 2: no SFML loading
    EXPECT_FALSE(drawables[1]->is_loaded);
    EXPECT_EQ(drawables[0]->z_index, 5);
    EXPECT_EQ(drawables[1]->z_index, 1);
}
