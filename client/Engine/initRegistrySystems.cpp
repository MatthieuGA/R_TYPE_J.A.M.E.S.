#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

void init_render_systems(Rtype::Client::GameWorld &gameWorld) {
    gameWorld.registry.add_system<Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::Drawable>>(
        [&gameWorld](Eng::registry &r,
                Eng::sparse_array<Com::AnimatedSprite> &animatedSprites,
                Eng::sparse_array<Com::Drawable> &drawables) {
            animationSystem(r, gameWorld.deltaTimeClock, animatedSprites, drawables);
        });
    gameWorld.registry.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Drawable>,
        Eng::sparse_array<Com::AnimatedSprite>>(
        [&gameWorld](Eng::registry &r,
                Eng::sparse_array<Com::Transform> const &transforms,
                Eng::sparse_array<Com::Drawable> &drawables,
                Eng::sparse_array<Com::AnimatedSprite> const &animatedSprites) {
            drawableSystem(r, gameWorld.window, transforms, drawables, animatedSprites);
        });
}

void init_movement_system(Rtype::Client::GameWorld &gameWorld) {
    gameWorld.registry.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Velocity>>(
        [&gameWorld](Eng::registry &r,
                Eng::sparse_array<Com::Transform> &transforms,
                Eng::sparse_array<Com::Velocity> &velocities) {
            movementSystem(r, gameWorld.deltaTimeClock, transforms, velocities);
        });
    gameWorld.registry.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::PlayerTag>>(
        [&gameWorld](Eng::registry &r,
                Eng::sparse_array<Com::Transform> &transforms,
                Eng::sparse_array<Com::PlayerTag> const &playerTag) {
            playfieldLimitSystem(r, gameWorld.window, transforms, playerTag);
        });
    gameWorld.registry.add_system<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::HitBox>,
        Eng::sparse_array<Com::Solid>>(
            [&gameWorld](Eng::registry &r,
                Eng::sparse_array<Com::Transform> &transforms,
                Eng::sparse_array<Com::HitBox> const &hitBoxes,
                Eng::sparse_array<Com::Solid> const &solids) {
            collisionDetectionSystem(r, gameWorld, transforms, hitBoxes, solids);
        });
}

void init_registry_systems(Rtype::Client::GameWorld &gameWorld) {
    // Set up systems
    init_movement_system(gameWorld);
    init_render_systems(gameWorld);
    gameWorld.registry.add_system<>([&gameWorld](Eng::registry &r) {
        gameWorld.deltaTimeClock.restart();
    });
}
}  // namespace Rtype::Client
