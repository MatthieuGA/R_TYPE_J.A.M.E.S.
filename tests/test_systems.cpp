#include <gtest/gtest.h>

#include <utility>

#include "engine/GameWorld.hpp"
#include "engine/events/EngineEvent.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/GameplayComponents.hpp"
#include "include/components/RenderComponent.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

using Rtype::Client::AnimationSystem;
using Rtype::Client::CollisionDetectionSystem;
using Rtype::Client::InputSystem;
using Rtype::Client::MovementSystem;
using Rtype::Client::PlayerSystem;
using Rtype::Client::PlayfieldLimitSystem;
using Rtype::Client::ProjectileSystem;
using Rtype::Client::ShootPlayerSystem;

TEST(Systems, MovementSystemUpdatesPosition) {
    Eng::registry reg;

    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::Velocity> velocities;

    // Create entity components
    transforms.insert_at(0, Com::Transform{0.0f, 0.0f, 0.0f, 1.0f});
    velocities.insert_at(0, Com::Velocity{200.0f, 0.0f});

    sf::Clock clock;
    // Wait a short time so clock has non-zero elapsed time
    sf::sleep(sf::milliseconds(20));

    MovementSystem(
        reg, clock.getElapsedTime().asSeconds(), transforms, velocities);

    // Expect the transform to have moved to the right
    EXPECT_GT(transforms[0]->x, 0.0f);
}

TEST(Systems, PlayfieldLimitClampsPosition) {
    Eng::registry reg;

    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    // Place transform outside a small window
    transforms.insert_at(0, Com::Transform{500.0f, 400.0f, 0.0f, 1.0f});
    player_tags.insert_at(0, Com::PlayerTag{1});

    // Create a small window (headless CI may still support creation)
    Rtype::Client::GameWorld game_world;
    sf::RenderWindow window(sf::VideoMode(200, 150), "test", sf::Style::None);
    game_world.window_size_ =
        sf::Vector2f(static_cast<float>(window.getSize().x),
            static_cast<float>(window.getSize().y));

    PlayfieldLimitSystem(reg, game_world, transforms, player_tags);

    EXPECT_LE(transforms[0]->x, static_cast<float>(window.getSize().x));
    EXPECT_LE(transforms[0]->y, static_cast<float>(window.getSize().y));

    window.close();
}

TEST(Systems, AnimationSystemAdvancesFrame) {
    Eng::registry reg;

    Eng::sparse_array<Com::AnimatedSprite> anim_sprites;
    Eng::sparse_array<Com::Drawable> drawables;

    // Create an animated sprite component with multiple frames
    Com::AnimatedSprite anim(16, 16, 0.02f);  // frameW, frameH, frameDuration
    anim.animations["Default"].totalFrames = 4;
    anim.currentAnimation = "Default";
    anim.animated = true;
    anim.elapsedTime = 0.0f;

    anim_sprites.insert_at(0, std::move(anim));

    // Create a drawable and mark it as loaded so the system advances frames
    drawables.insert_at(0, Com::Drawable("dummy.png"));
    // Ensure texture has a size so SetFrame won't early-return
    drawables[0]->texture.create(64, 64);
    drawables[0]->sprite.setTexture(drawables[0]->texture, true);
    drawables[0]->isLoaded = true;

    // Simulate a delta time that should advance at least one frame
    float delta = 0.05f;  // 50 ms

    // Ensure deterministic advancement: pre-fill elapsedTime so NextFrame
    // will trigger on the next update regardless of dt semantics.
    ASSERT_TRUE(anim_sprites[0].has_value());
    anim_sprites[0]->elapsedTime =
        anim_sprites[0]->GetCurrentAnimation()->frameDuration;

    // First call should advance the current_frame because elapsedTime >=
    // frameDuration
    AnimationSystem(reg, 0.0f, anim_sprites, drawables);
    EXPECT_EQ(anim_sprites[0]->GetCurrentAnimation()->current_frame, 1);

    // Second call with zero delta will cause SetFrame to update the drawable
    // rect
    AnimationSystem(reg, 0.0f, anim_sprites, drawables);
    sf::IntRect rect = drawables[0]->sprite.getTextureRect();
    EXPECT_EQ(rect.left, anim_sprites[0]->GetCurrentAnimation()->frameWidth);
}

TEST(Systems, CollisionDetectionPublishesAndResolves) {
    Eng::registry reg;
    Rtype::Client::GameWorld gw;

    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::HitBox> hitboxes;
    Eng::sparse_array<Com::Solid> solids;

    // Two entities that overlap on X axis
    transforms.insert_at(0, Com::Transform{0.0f, 0.0f, 0.0f, 1.0f});
    transforms.insert_at(1, Com::Transform{10.0f, 0.0f, 0.0f, 1.0f});

    hitboxes.insert_at(0, Com::HitBox{16.0f, 16.0f, true, 0.0f, 0.0f});
    hitboxes.insert_at(1, Com::HitBox{16.0f, 16.0f, true, 0.0f, 0.0f});

    solids.insert_at(0, Com::Solid{true, false});
    solids.insert_at(1, Com::Solid{true, false});

    bool published = false;
    size_t a = SIZE_MAX, b = SIZE_MAX;
    gw.event_bus_.Subscribe<::CollisionEvent>(
        [&published, &a, &b](const ::CollisionEvent &e, int) {
            published = true;
            a = e.entity_a_;
            b = e.entity_b_;
        });

    CollisionDetectionSystem(reg, gw, transforms, hitboxes, solids);

    EXPECT_TRUE(published);
    EXPECT_EQ(a, 0u);
    EXPECT_EQ(b, 1u);

    // Ensure positions were adjusted (no longer same as initial)
    EXPECT_NE(transforms[0]->x, 0.0f);
    EXPECT_NE(transforms[1]->x, 10.0f);
}

TEST(Systems, ProjectileSystemMovesTransform) {
    Eng::registry reg;
    Rtype::Client::GameWorld gw;

    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::Projectile> projectiles;

    transforms.insert_at(0, Com::Transform{0.0f, 0.0f, 0.0f, 1.0f});
    projectiles.insert_at(0, Com::Projectile{5.0f, 200.0f, 1});

    gw.last_delta_ = 0.1f;  // 200 * 0.1 = 20

    ProjectileSystem(reg, gw, transforms, projectiles);

    ASSERT_TRUE(transforms[0].has_value());
    EXPECT_GT(transforms[0]->x, 0.0f);
}

TEST(Systems, PlayerSystemSetsFrameBasedOnVelocity) {
    Eng::registry reg;

    Eng::sparse_array<Com::PlayerTag> player_tags;
    Eng::sparse_array<Com::Velocity> velocities;
    Eng::sparse_array<Com::AnimatedSprite> animated_sprites;
    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::ParticleEmitter> particle_emitters;
    Eng::sparse_array<Com::Transform> transforms;

    player_tags.insert_at(0, Com::PlayerTag{400.f, 0.5f, 0.0f, 1});
    velocities.insert_at(0, Com::Velocity{0.0f, 100.0f});
    animated_sprites.insert_at(0, Com::AnimatedSprite(16, 16, 0.1f));
    transforms.insert_at(0, Com::Transform(0.0f, 0.0f, 0.0f, 1.0f));
    inputs.insert_at(0, Com::Inputs{0.0f, 0.0f, false});
    particle_emitters.insert_at(0, Com::ParticleEmitter());

    PlayerSystem(reg, player_tags, velocities, inputs, particle_emitters,
        transforms, animated_sprites);
    ASSERT_TRUE(animated_sprites[0].has_value());
    // velocity.vy == 100 -> should map to current_frame == 1
    EXPECT_EQ(animated_sprites[0]->GetCurrentAnimation()->current_frame, 1);
}

TEST(Systems, ShootPlayerSystemCreatesProjectileAndResetsCooldown) {
    Eng::registry reg;
    Rtype::Client::GameWorld gw;

    // Register components that createProjectile will add
    reg.RegisterComponent<Com::Transform>();
    reg.RegisterComponent<Com::Drawable>();
    reg.RegisterComponent<Com::AnimatedSprite>();
    reg.RegisterComponent<Com::Projectile>();

    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::Inputs> inputs;
    Eng::sparse_array<Com::PlayerTag> player_tags;

    transforms.insert_at(0, Com::Transform{10.0f, 20.0f, 0.0f, 1.0f});
    // Set shoot=true and last_shoot_state=false to trigger a new shot
    inputs.insert_at(0, Com::Inputs{0.0f, 0.0f, true, false});
    // Create PlayerTag with proper field values
    Com::PlayerTag tag;
    tag.speed_max = 400.0f;
    tag.shoot_cooldown_max = 0.2f;
    tag.charge_time_min = 0.5f;
    tag.shoot_cooldown = 0.0f;  // Ready to shoot
    tag.charge_time = 0.0f;
    tag.playerNumber = 1;
    player_tags.insert_at(0, tag);

    gw.last_delta_ = 0.03f;

    ShootPlayerSystem(reg, gw, transforms, inputs, player_tags);

    // After shooting, cooldown should be reset to max
    ASSERT_TRUE(player_tags[0].has_value());
    EXPECT_FLOAT_EQ(
        player_tags[0]->shoot_cooldown, player_tags[0]->shoot_cooldown_max);

    // The projectile component should have been added to the registry at
    // entity 0
    auto &projectiles = reg.GetComponents<Com::Projectile>();
    EXPECT_TRUE(projectiles.has(0));
}

TEST(Systems, InputSystemResetsInputsWhenNoKeys) {
    Eng::registry reg;

    Eng::sparse_array<Com::Inputs> inputs;
    inputs.insert_at(0, Com::Inputs{1.0f, -1.0f, true});

    InputSystem(reg, true, inputs);

    ASSERT_TRUE(inputs[0].has_value());
    EXPECT_FLOAT_EQ(inputs[0]->horizontal, 0.0f);
    EXPECT_FLOAT_EQ(inputs[0]->vertical, 0.0f);
    EXPECT_TRUE(inputs[0]->last_shoot_state);
    EXPECT_FALSE(inputs[0]->shoot);
}
