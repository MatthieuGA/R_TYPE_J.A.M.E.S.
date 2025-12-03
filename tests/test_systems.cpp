#include <gtest/gtest.h>

#include "../client/Engine/Systems/initRegistrySystems.hpp"
#include "../client/include/Components/CoreComponents.hpp"
#include "../client/include/Components/GameplayComponents.hpp"
#include "../client/Engine/Events/EngineEvent.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

using namespace Rtype::Client;

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

    MovementSystem(reg, clock, transforms, velocities);

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
    sf::RenderWindow window(sf::VideoMode(200, 150), "test", sf::Style::None);

    PlayfieldLimitSystem(reg, window, transforms, player_tags);

    EXPECT_LE(transforms[0]->x, static_cast<float>(window.getSize().x));
    EXPECT_LE(transforms[0]->y, static_cast<float>(window.getSize().y));

    window.close();
}

TEST(Systems, AnimationSystemAdvancesFrame) {
    Eng::registry reg;

    Eng::sparse_array<Com::AnimatedSprite> anim_sprites;
    Eng::sparse_array<Com::Drawable> drawables;

    // Animated sprite: 4 frames, each 16x16
    anim_sprites.insert_at(0, Com::AnimatedSprite{16, 16, 4});
    auto &anim = anim_sprites[0];
    anim->currentFrame = 0;
    anim->frameDuration = 0.01f;

    drawables.insert_at(0, Com::Drawable("test.png", 0));
    auto &drawable = drawables[0];

    // Create a texture big enough for 4 columns x 1 row
    int columns = 4;
    drawable->texture.create(columns * anim->frameWidth, anim->frameHeight);
    drawable->sprite.setTexture(drawable->texture, true);
    drawable->isLoaded = true;

    sf::Clock clock;
    sf::sleep(sf::milliseconds(20));

    AnimationSystem(reg, clock, anim_sprites, drawables);

    EXPECT_EQ(anim->currentFrame, 1);
    sf::IntRect rect = drawable->sprite.getTextureRect();
    EXPECT_EQ(rect.left, anim->frameWidth); // second frame
    EXPECT_EQ(rect.width, anim->frameWidth);
    EXPECT_EQ(rect.height, anim->frameHeight);
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

    hitboxes.insert_at(0, Com::HitBox{16.0f, 16.0f, 0.0f, 0.0f});
    hitboxes.insert_at(1, Com::HitBox{16.0f, 16.0f, 0.0f, 0.0f});

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
