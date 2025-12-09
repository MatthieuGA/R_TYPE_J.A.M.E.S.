#include "engine/systems/InitRegistrySystems.hpp"

#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

/**
 * @brief Initialize render-related systems in the registry.
 *
 * This function sets up the necessary systems for rendering,
 * including drawable initialization, shader setup, and animation handling.
 *
 * @param game_world The game world containing the registry.
 */
void InitRenderSystems(Rtype::Client::GameWorld &game_world) {
    // Initialize systems
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Drawable>,
        Eng::sparse_array<Com::AnimatedSprite>>(
        InitializeDrawableAnimatedSystem);
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Shader>>(
        InitializeShaderSystem);
    // Main render system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::Drawable>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
            Eng::sparse_array<Com::Drawable> &drawables) {
            AnimationSystem(
                r, game_world.last_delta_, animated_sprites, drawables);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::AnimationDeath>>(DeathAnimationSystem);
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Drawable>, Eng::sparse_array<Com::Shader>,
        Eng::sparse_array<Com::AnimatedSprite>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> const &transforms,
            Eng::sparse_array<Com::Drawable> &drawables,
            Eng::sparse_array<Com::Shader> &shaders,
            Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites) {
            DrawableSystem(r, game_world, transforms, drawables, shaders,
                animated_sprites);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Text>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> const &transforms,
            Eng::sparse_array<Com::Text> &texts) {
            DrawTextRenderSystem(r, game_world, transforms, texts);
        });
}

/**
 * @brief Initialize movement-related systems in the registry.
 *
 * This function sets up the necessary systems for handling movement,
 * including velocity updates, parallax effects, playfield limits, and
 * collision detection.
 *
 * @param game_world The game world containing the registry.
 */
void InitMovementSystem(Rtype::Client::GameWorld &game_world) {
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Velocity>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::Velocity> &velocities) {
            MovementSystem(r, game_world.last_delta_, transforms, velocities);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::ParrallaxLayer>,
        Eng::sparse_array<Com::Drawable>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::ParrallaxLayer> &parallax_layers,
            Eng::sparse_array<Com::Drawable> &drawables) {
            ParallaxSystem(
                r, game_world, transforms, parallax_layers, drawables);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Velocity>,
        Eng::sparse_array<Com::Transform>, Eng::sparse_array<Com::PlayerTag>,
        Eng::sparse_array<Com::AnimationEnterPlayer>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Velocity> &velocities,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::PlayerTag> &player_tags,
            Eng::sparse_array<Com::AnimationEnterPlayer>
                &animation_enter_players) {
            AnimationEnterPlayerSystem(r, game_world, velocities, transforms,
                player_tags, animation_enter_players);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::PlayerTag>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::PlayerTag> const &playerTag) {
            PlayfieldLimitSystem(r, game_world, transforms, playerTag);
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

/**
 * @brief Initialize control-related systems in the registry.
 *
 * This function sets up the necessary systems for handling player input,
 * controllable entities, player-specific logic, shooting mechanics, and
 * visual feedback for charging actions.
 *
 * @param game_world The game world containing the registry.
 */
void InitControlsSystem(Rtype::Client::GameWorld &game_world) {
    game_world.registry_.AddSystem<Eng::sparse_array<Com::HitBox>,
        Eng::sparse_array<Com::Clickable>, Eng::sparse_array<Com::Drawable>,
        Eng::sparse_array<Com::Transform>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::HitBox> &hit_boxes,
            Eng::sparse_array<Com::Clickable> &clickables,
            Eng::sparse_array<Com::Drawable> &drawables,
            Eng::sparse_array<Com::Transform> &transforms) {
            ButtonClickSystem(
                r, game_world, hit_boxes, clickables, drawables, transforms);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Inputs>>(
        [&game_world](
            Eng::registry &r, Eng::sparse_array<Com::Inputs> &inputs) {
            InputSystem(r, game_world.window_.hasFocus(), inputs);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Inputs>,
        Eng::sparse_array<Com::Controllable>, Eng::sparse_array<Com::Velocity>,
        Eng::sparse_array<Com::PlayerTag>>(ControllablePlayerSystem);
    game_world.registry_.AddSystem<Eng::sparse_array<Com::PlayerTag>,
        Eng::sparse_array<Com::Velocity>, Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::AnimatedSprite>>(PlayerSystem);
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Inputs>, Eng::sparse_array<Com::PlayerTag>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::Inputs> const &inputs,
            Eng::sparse_array<Com::PlayerTag> &player_tags) {
            ShootPlayerSystem(r, game_world, transforms, inputs, player_tags);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::EnemyShootTag>,
        Eng::sparse_array<Com::EnemyTag>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
            Eng::sparse_array<Com::EnemyShootTag> &enemy_shoot_tags,
            Eng::sparse_array<Com::EnemyTag> const &enemy_tags) {
            ShootEnemySystem(r, game_world, transforms, animated_sprites,
                enemy_shoot_tags, enemy_tags);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::PlayerTag>,
        Eng::sparse_array<Com::Drawable>,
        Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::Transform>>(
        [](Eng::registry &r, Eng::sparse_array<Com::PlayerTag> &player_tags,
            Eng::sparse_array<Com::Drawable> &drawables,
            Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
            Eng::sparse_array<Com::Transform> &transforms) {
            ChargingShowAssetPlayerSystem(
                r, player_tags, drawables, animated_sprites, transforms);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Projectile>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::Projectile> &projectiles) {
            ProjectileSystem(r, game_world, transforms, projectiles);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::PlayerTag>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
            Eng::sparse_array<Com::PlayerTag> &player_tags) {
            PlayerHitSystem(r, game_world, animated_sprites, player_tags);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Health>,
        Eng::sparse_array<Com::EnemyTag>, Eng::sparse_array<Com::PlayerTag>,
        Eng::sparse_array<Com::AnimatedSprite>, Eng::sparse_array<Com::HitBox>,
        Eng::sparse_array<Com::Transform>, Eng::sparse_array<Com::Projectile>>(
        HealthDeductionSystem);
}

/**
 * @brief Initialize scene management system in the registry.
 *
 * This function sets up the system responsible for handling
 * scene transitions within the provided game world's registry.
 *
 * @param game_world The game world containing the registry.
 */
void InitSceneManagementSystem(Rtype::Client::GameWorld &game_world) {
    game_world.registry_.AddSystem<Eng::sparse_array<Com::SceneManagement>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::SceneManagement> &sceneManagements) {
            GameStateSystem(r, game_world, sceneManagements);
        });
}

/**
 * @brief Initialize all registry systems for the game world.
 *
 * This function sets up control, movement, and rendering systems
 * within the provided game world's registry.
 *
 * @param game_world The game world containing the registry.
 */
void InitRegistrySystems(Rtype::Client::GameWorld &game_world) {
    // Set up systems
    InitSceneManagementSystem(game_world);
    InitControlsSystem(game_world);
    InitMovementSystem(game_world);
    InitRenderSystems(game_world);
}
}  // namespace Rtype::Client
