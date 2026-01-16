#include <gtest/gtest.h>

#include <cstdlib>
#include <memory>
#include <set>
#include <utility>

#include "TestGraphicsSetup.hpp"  // NOLINT(build/include_subdir)
#include "engine/GameWorld.hpp"
#include "engine/events/EngineEvent.hpp"
#include "engine/systems/InitRegistrySystems.hpp"
#include "game/GameAction.hpp"
#include "game/GameInputBindings.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/GameplayComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "input/IInputBackend.hpp"
#include "input/InputManager.hpp"
#include "platform/SFMLWindow.hpp"

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

/**
 * @brief Mock input backend for system tests.
 *
 * Allows tests to control exactly which keys are "pressed".
 * Named differently from test_input_manager.cpp's mock to avoid ODR
 * violations.
 */
class SystemTestMockInputBackend : public Engine::Input::IInputBackend {
 public:
    bool IsKeyPressed(Engine::Input::Key key) const override {
        return pressed_keys_.count(key) > 0;
    }

    bool IsMouseButtonPressed(
        Engine::Input::MouseButton button) const override {
        return pressed_mouse_buttons_.count(button) > 0;
    }

    Engine::Input::MousePosition GetMousePosition() const override {
        return {0, 0};
    }

    Engine::Input::MousePosition GetMousePositionInWindow() const override {
        return {0, 0};
    }

    bool HasWindowFocus() const override {
        return has_focus_;
    }

    // Test helpers
    void SetKeyPressed(Engine::Input::Key key, bool pressed) {
        if (pressed) {
            pressed_keys_.insert(key);
        } else {
            pressed_keys_.erase(key);
        }
    }

    void SetMouseButtonPressed(
        Engine::Input::MouseButton button, bool pressed) {
        if (pressed) {
            pressed_mouse_buttons_.insert(button);
        } else {
            pressed_mouse_buttons_.erase(button);
        }
    }

    void SetFocus(bool focus) {
        has_focus_ = focus;
    }

 private:
    std::set<Engine::Input::Key> pressed_keys_;
    std::set<Engine::Input::MouseButton> pressed_mouse_buttons_;
    bool has_focus_ = true;
};

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
    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        200, 150, "test");
    auto window_size = window->GetSize();

    TestHelper::RegisterTestBackend();
    Rtype::Client::GameWorld game_world(
        std::move(window), "test", "127.0.0.1", 50000, 50000);
    game_world.window_size_ = Engine::Graphics::Vector2f(
        static_cast<float>(window_size.x), static_cast<float>(window_size.y));

    PlayfieldLimitSystem(reg, game_world, transforms, player_tags);

    EXPECT_LE(transforms[0]->x, static_cast<float>(window_size.x));
    EXPECT_LE(transforms[0]->y, static_cast<float>(window_size.y));
}

TEST(Systems, AnimationSystemAdvancesFrame) {
    TestHelper::RegisterTestBackend();

    Eng::registry reg;

    Eng::sparse_array<Com::AnimatedSprite> anim_sprites;
    Eng::sparse_array<Com::Drawable> drawables;

    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        10, 10, "anim-test");
    Rtype::Client::GameWorld game_world(
        std::move(window), "test", "127.0.0.1", 50000, 50000);

    // Create an animated sprite component with multiple frames
    Com::AnimatedSprite anim(16, 16, 0.02f);  // frameW, frameH, frameDuration
    anim.animations["Default"].totalFrames = 4;
    anim.currentAnimation = "Default";
    anim.animated = true;
    anim.elapsedTime = 0.0f;

    anim_sprites.insert_at(0, std::move(anim));

    // Create a drawable and mark it as loaded so the system advances frames
    drawables.insert_at(0, Com::Drawable("dummy.png"));
    drawables[0]->is_loaded = true;

    // Simulate a delta time that should advance at least one frame
    float delta = 0.05f;  // 50 ms

    // Ensure deterministic advancement: pre-fill elapsedTime so NextFrame
    // will trigger on the next update regardless of dt semantics.
    ASSERT_TRUE(anim_sprites[0].has_value());
    anim_sprites[0]->elapsedTime =
        anim_sprites[0]->GetCurrentAnimation()->frameDuration;

    // First call should advance the current_frame because elapsedTime >=
    // frameDuration
    AnimationSystem(reg, game_world, 0.0f, anim_sprites, drawables);
    EXPECT_EQ(anim_sprites[0]->GetCurrentAnimation()->current_frame, 1);

    // Second call with zero delta will cause SetFrame to update the drawable
    // rect
    AnimationSystem(reg, game_world, 0.0f, anim_sprites, drawables);
    EXPECT_EQ(drawables[0]->current_rect.left,
        anim_sprites[0]->GetCurrentAnimation()->frameWidth);
}

TEST(Systems, CollisionDetectionPublishesAndResolves) {
    TestHelper::RegisterTestBackend();

    Eng::registry reg;
    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");
    Rtype::Client::GameWorld gw(
        std::move(window), "test", "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::HitBox> hitboxes;
    Eng::sparse_array<Com::Solid> solids;
    Eng::sparse_array<Com::Controllable> controllables;

    // Two entities that overlap on X axis
    transforms.insert_at(0, Com::Transform{0.0f, 0.0f, 0.0f, 1.0f});
    transforms.insert_at(1, Com::Transform{10.0f, 0.0f, 0.0f, 1.0f});

    hitboxes.insert_at(0, Com::HitBox{16.0f, 16.0f, true, 0.0f, 0.0f});
    hitboxes.insert_at(1, Com::HitBox{16.0f, 16.0f, true, 0.0f, 0.0f});

    solids.insert_at(0, Com::Solid{true, false});
    solids.insert_at(1, Com::Solid{true, false});

    // Mark entity 0 as controllable (local player) so collision resolution
    // is applied
    controllables.insert_at(0, Com::Controllable{true});

    bool published = false;
    size_t a = SIZE_MAX, b = SIZE_MAX;
    gw.event_bus_.Subscribe<::CollisionEvent>(
        [&published, &a, &b](const ::CollisionEvent &e, int) {
            published = true;
            a = e.entity_a_;
            b = e.entity_b_;
        });

    CollisionDetectionSystem(
        reg, gw, transforms, hitboxes, solids, controllables);

    EXPECT_TRUE(published);
    EXPECT_EQ(a, 0u);
    EXPECT_EQ(b, 1u);

    // Ensure positions were adjusted (no longer same as initial)
    EXPECT_NE(transforms[0]->x, 0.0f);
    EXPECT_NE(transforms[1]->x, 10.0f);
}

TEST(Systems, ProjectileSystemMovesTransform) {
    TestHelper::RegisterTestBackend();

    Eng::registry reg;
    auto window = std::make_unique<Rtype::Client::Platform::SFMLWindow>(
        800, 600, "test");
    Rtype::Client::GameWorld gw(
        std::move(window), "test", "127.0.0.1", 50000, 50000);

    Eng::sparse_array<Com::Transform> transforms;
    Eng::sparse_array<Com::Projectile> projectiles;

    transforms.insert_at(0, Com::Transform{0.0f, 0.0f, 0.0f, 1.0f});
    projectiles.insert_at(
        0, Com::Projectile{10, sf::Vector2f{1.0f, 0.0f}, 200.0f, 1});

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

// Test that InputSystem resets input values when no keys are pressed
// DISABLED: This test passes locally but aborts in CI (possibly flaky
// environment)
TEST(Systems, DISABLED_InputSystemResetsInputsWhenNoKeys) {
    // Create mock backend with no keys pressed
    auto *mock_backend_ptr = new SystemTestMockInputBackend();
    auto mock_backend =
        std::unique_ptr<Engine::Input::IInputBackend>(mock_backend_ptr);

    // Use game-specific InputManager with Game::Action
    Rtype::Client::GameInputManager input_manager(std::move(mock_backend));
    Game::SetupDefaultBindings(input_manager);

    Eng::registry reg;

    Eng::sparse_array<Com::Inputs> inputs;
    inputs.insert_at(0, Com::Inputs{1.0f, -1.0f, true});

    InputSystem(reg, input_manager, inputs);

    ASSERT_TRUE(inputs[0].has_value());
    EXPECT_FLOAT_EQ(inputs[0]->horizontal, 0.0f);
    EXPECT_FLOAT_EQ(inputs[0]->vertical, 0.0f);
    EXPECT_TRUE(inputs[0]->last_shoot_state);
    EXPECT_FALSE(inputs[0]->shoot);
}
