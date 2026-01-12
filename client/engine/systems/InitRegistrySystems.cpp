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

    // Shader initialization system
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

    // Death animation system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::AnimationDeath>>(DeathAnimationSystem);

    // Drawable system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Drawable>, Eng::sparse_array<Com::Shader>,
        Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::ParticleEmitter>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> const &transforms,
            Eng::sparse_array<Com::Drawable> &drawables,
            Eng::sparse_array<Com::Shader> &shaders,
            Eng::sparse_array<Com::AnimatedSprite> const &animated_sprites,
            Eng::sparse_array<Com::ParticleEmitter> &emitters) {
            DrawableSystem(r, game_world, transforms, drawables, shaders,
                animated_sprites, emitters);
        });

    // Particle emitter system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Text>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> const &transforms,
            Eng::sparse_array<Com::Text> &texts) {
            DrawTextRenderSystem(r, game_world, transforms, texts);
        });

    // Health bar system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::HealthBar>, Eng::sparse_array<Com::Health>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> const &transforms,
            Eng::sparse_array<Com::HealthBar> &health_bars,
            Eng::sparse_array<Com::Health> const &healths) {
            HealthBarSystem(r, game_world, transforms, health_bars, healths);
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
    // Patern movement system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Velocity>,
        Eng::sparse_array<Com::PatternMovement>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::Velocity> &velocities,
            Eng::sparse_array<Com::PatternMovement> &patern_movements) {
            PaternMovementSystem(r, game_world.last_delta_, transforms,
                velocities, patern_movements);
        });

    // Movement system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Velocity>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::Velocity> &velocities) {
            MovementSystem(r, game_world.last_delta_, transforms, velocities);
        });

    // Parallax system
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

    // Animation enter player system
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

    // Playfield limit system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::PlayerTag>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::PlayerTag> const &playerTag) {
            PlayfieldLimitSystem(r, game_world, transforms, playerTag);
        });

    // Collision detection system
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
    // Button click system
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

    // Input control systems
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Inputs>>(
        [&game_world](
            Eng::registry &r, Eng::sparse_array<Com::Inputs> &inputs) {
            InputSystem(r, game_world.window_.hasFocus(), inputs);
        });

    // Timed events system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::TimedEvents>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::TimedEvents> &timed_events) {
            TimedEventSystem(r, game_world, timed_events);
        });
}

/**
 * @brief Initialize gameplay-related systems in the registry.
 *
 * This function sets up the necessary systems for handling gameplay
 * mechanics, including player controls, shooting, projectiles,
 * and health management.
 *
 * @param game_world The game world containing the registry.
 */
void InitGameplaySystems(Rtype::Client::GameWorld &game_world) {
    // Player control systems
    // Send network input packets after capturing local input
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Inputs>,
        Eng::sparse_array<Com::PlayerTag>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Inputs> const &inputs,
            Eng::sparse_array<Com::PlayerTag> const &player_tags) {
            NetworkInputSystem(r, game_world, inputs, player_tags);
        });
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Inputs>,
        Eng::sparse_array<Com::Controllable>, Eng::sparse_array<Com::Velocity>,
        Eng::sparse_array<Com::PlayerTag>>(ControllablePlayerSystem);

    // Player gameplay systems
    game_world.registry_.AddSystem<Eng::sparse_array<Com::PlayerTag>,
        Eng::sparse_array<Com::Velocity>, Eng::sparse_array<Com::Inputs>,
        Eng::sparse_array<Com::ParticleEmitter>,
        Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::AnimatedSprite>>(PlayerSystem);

    // Players Shooting systems
    // game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
    //     Eng::sparse_array<Com::Inputs>, Eng::sparse_array<Com::PlayerTag>>(
    //     [&game_world](Eng::registry &r,
    //         Eng::sparse_array<Com::Transform> &transforms,
    //         Eng::sparse_array<Com::Inputs> const &inputs,
    //         Eng::sparse_array<Com::PlayerTag> &player_tags) {
    //         ShootPlayerSystem(r, game_world, transforms, inputs,
    //         player_tags);
    //     });

    // Frame base event system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::FrameEvents>>(FrameBaseEventSystem);

    // Charging show asset system
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

    // Projectile systems
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Transform>,
        Eng::sparse_array<Com::Projectile>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::Transform> &transforms,
            Eng::sparse_array<Com::Projectile> &projectiles) {
            ProjectileSystem(r, game_world, transforms, projectiles);
        });

    // Player hit system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::AnimatedSprite>,
        Eng::sparse_array<Com::PlayerTag>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
            Eng::sparse_array<Com::PlayerTag> &player_tags) {
            PlayerHitSystem(r, game_world, animated_sprites, player_tags);
        });

    // Health deduction system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::Health>,
        Eng::sparse_array<Com::HealthBar>,
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
    // Scene management system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::SceneManagement>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::SceneManagement> &sceneManagements) {
            GameStateSystem(r, game_world, sceneManagements);
        });

    // Lobby UI system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::LobbyUI>,
        Eng::sparse_array<Com::Text>, Eng::sparse_array<Com::Drawable>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::LobbyUI> &lobby_uis,
            Eng::sparse_array<Com::Text> &texts,
            Eng::sparse_array<Com::Drawable> &drawables) {
            LobbyUISystem(r, game_world, lobby_uis, texts, drawables);
        });

    // Game over system
    game_world.registry_.AddSystem<Eng::sparse_array<Com::GameOverState>,
        Eng::sparse_array<Com::GameOverText>,
        Eng::sparse_array<Com::FadeOverlay>, Eng::sparse_array<Com::Text>,
        Eng::sparse_array<Com::Drawable>,
        Eng::sparse_array<Com::SceneManagement>>(
        [&game_world](Eng::registry &r,
            Eng::sparse_array<Com::GameOverState> &states,
            Eng::sparse_array<Com::GameOverText> &go_texts,
            Eng::sparse_array<Com::FadeOverlay> &overlays,
            Eng::sparse_array<Com::Text> &text_comps,
            Eng::sparse_array<Com::Drawable> &drawables,
            Eng::sparse_array<Com::SceneManagement> &scene_mgmt) {
            GameOverSystem(r, game_world, states, go_texts, overlays,
                text_comps, drawables, scene_mgmt);
        });
}

/**
 * @brief Initialize audio system in the registry.
 *
 * This function sets up the audio system responsible for processing
 * sound requests and playing audio through the audio manager.
 *
 * @param game_world The game world containing the registry.
 * @param audio_manager The audio manager for handling audio playback.
 */
void InitAudioSystem(
    Rtype::Client::GameWorld &game_world, Audio::AudioManager &audio_manager) {
    game_world.registry_.AddSystem<Com::SoundRequest>(
        [&audio_manager](Eng::registry &reg,
            Eng::sparse_array<Com::SoundRequest> &sound_requests) {
            AudioSystem(reg, audio_manager, sound_requests);
        });
}

void InitKillEntitiesSystem(Rtype::Client::GameWorld &game_world) {
    game_world.registry_.AddSystem<Com::NetworkId, Com::AnimationDeath>(
        KillEntitiesSystem);
}

/**
 * @brief Initialize all registry systems for the game world.
 *
 * This function sets up control, movement, audio, and rendering systems
 * within the provided game world's registry.
 *
 * @param game_world The game world containing the registry.
 * @param audio_manager The audio manager for the audio system.
 */
void InitRegistrySystems(
    Rtype::Client::GameWorld &game_world, Audio::AudioManager &audio_manager) {
    // Set up systems
    InitSceneManagementSystem(game_world);
    InitKillEntitiesSystem(game_world);
    InitControlsSystem(game_world);
    InitMovementSystem(game_world);
    InitGameplaySystems(game_world);
    InitAudioSystem(game_world, audio_manager);
    InitRenderSystems(game_world);
}
}  // namespace Rtype::Client
