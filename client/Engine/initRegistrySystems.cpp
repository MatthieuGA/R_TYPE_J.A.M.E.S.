#include "Engine/initRegistrySystems.hpp"

#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

void init_render_systems(Rtype::Client::GameWorld &game_world) {
    game_world.registry_.AddSystem<Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::Drawable>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
            Eng::sparse_array<Com::Drawable> &drawables) {
            AnimationSystem(
                r, game_world.delta_time_clock_, animated_sprites, drawables);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Drawable>,
        Eng::sparse_array<Com::AnimatedSprite>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> const &transforms,
            Eng::sparse_array<Com::Drawable> &drawables,
            Eng::sparse_array<Com::AnimatedSprite> const &animated_sprite) {
            DrawableSystem(
                r, game_world.window_, transforms, drawables, animated_sprite);
        });
}

void init_movement_system(Rtype::Client::GameWorld &game_world) {
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Velocity>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::Velocity> &velocities) {
            MovementSystem(
                r, game_world.delta_time_clock_, transforms, velocities);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::PlayerTag>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::PlayerTag> const &playerTag) {
            PlayfieldLimitSystem(r, game_world.window_, transforms, playerTag);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::HitBox>, Eng::sparse_array<Com::Solid>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::HitBox> const &hitbox,
            Eng::sparse_array<Com::Solid> const &solids) {
            CollisionDetectionSystem(
                r, game_world, transforms, hitbox, solids);
        });
}

void InitRegistrySystems(Rtype::Client::GameWorld &game_world) {
    // Set up systems
    init_movement_system(game_world);
    init_render_systems(game_world);
    game_world.registry_.AddSystem<>([&game_world](Eng::registry &r) {
        game_world.delta_time_clock_.restart();
    });
}
}  // namespace Rtype::Client
