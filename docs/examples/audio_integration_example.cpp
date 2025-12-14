/**
 * @file audio_integration_example.cpp
 * @brief Practical example of audio system integration in R-Type.
 *
 * This example demonstrates:
 * - Audio system initialization
 * - Asset registration
 * - Sound playback via SoundRequest components
 * - Volume control
 * - Background music management
 */

#include <iostream>
#include <memory>
#include <string>
#include <utility>

#include "Engine/audio/AudioManager.hpp"
#include "Engine/audio/SFMLAudioBackend.hpp"
#include "Engine/gameWorld.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "Engine/initRegistrySystems.hpp"
#include "include/registry.hpp"

using Rtype::Client::Audio::AudioManager;
using Rtype::Client::Audio::SFMLAudioBackend;

/**
 * @brief Example: Playing sound effects from a collision system.
 */
void ExampleCollisionWithAudio(Engine::registry &registry,
    Engine::entity player_entity, Engine::entity enemy_entity) {
    // Detect collision (simplified)
    bool collision_detected = true;  // Placeholder

    if (collision_detected) {
        // Emit sound request component on a temporary entity
        auto audio_entity = registry.SpawnEntity();
        registry.EmplaceComponent<Component::SoundRequest>(audio_entity,
            Component::SoundRequest{
                .sound_id = "explosion", .volume = 0.8f, .loop = false});

        // AudioSystem will automatically:
        // 1. Detect this SoundRequest component
        // 2. Play the sound through AudioManager
        // 3. Remove the component after processing
    }
}

/**
 * @brief Example: Playing background music at game start.
 */
void ExamplePlayBackgroundMusic(Audio::AudioManager &audio_manager) {
    // Play looping background music at 60% volume
    audio_manager.SetMusicVolume(0.6f);
    audio_manager.PlayMusic("level1_bgm", true);  // true = loop
}

/**
 * @brief Example: Playing UI sound effects.
 */
void ExamplePlayUISound(Engine::registry &registry,
    const std::string &sound_name, float volume = 1.0f) {
    auto entity = registry.SpawnEntity();
    registry.EmplaceComponent<Component::SoundRequest>(
        entity, Component::SoundRequest{
                    .sound_id = sound_name, .volume = volume, .loop = false});
}

/**
 * @brief Complete audio integration example.
 */
int main() {
    try {
        // 1. Create game world
        GameWorld game_world;

        // 2. Initialize audio subsystem
        auto audio_backend = std::make_unique<Audio::SFMLAudioBackend>();
        Audio::AudioManager audio_manager(std::move(audio_backend));

        // 3. Register components and systems
        InitRegistryComponents(game_world.registry_);
        InitRegistrySystems(game_world, audio_manager);

        // 4. Register audio assets
        std::cout << "Loading audio assets...\n";

        // Sound effects
        audio_manager.RegisterAsset(
            "explosion", "Assets/explosion.wav", false);
        audio_manager.RegisterAsset("laser", "Assets/laser.wav", false);
        audio_manager.RegisterAsset("powerup", "Assets/powerup.wav", false);
        audio_manager.RegisterAsset(
            "menu_click", "Assets/menu_click.wav", false);

        // Background music
        audio_manager.RegisterAsset(
            "level1_bgm", "Assets/level1_music.ogg", true);
        audio_manager.RegisterAsset("boss_bgm", "Assets/boss_music.ogg", true);

        // 5. Configure audio levels
        audio_manager.SetSfxVolume(0.7f);    // SFX at 70%
        audio_manager.SetMusicVolume(0.5f);  // Music at 50%

        // 6. Start background music
        ExamplePlayBackgroundMusic(audio_manager);

        // 7. Create game entities
        auto player = game_world.registry_.SpawnEntity();
        auto enemy = game_world.registry_.SpawnEntity();

        // ... (Add transform, drawable, velocity components as needed)

        // 8. Game loop
        int frame_count = 0;
        while (game_world.window_.isOpen()) {
            sf::Event event;
            while (game_world.window_.pollEvent(event)) {
                if (event.type == sf::Event::Closed) {
                    game_world.window_.close();
                }

                // Example: Play UI sound on spacebar
                if (event.type == sf::Event::KeyPressed) {
                    if (event.key.code == sf::Keyboard::Space) {
                        ExamplePlayUISound(
                            game_world.registry_, "laser", 0.9f);
                    }
                }
            }

            // Example: Play explosion sound every 120 frames
            if (frame_count % 120 == 0) {
                ExampleCollisionWithAudio(game_world.registry_, player, enemy);
            }

            // Run all systems (including AudioSystem)
            game_world.registry_.RunSystems();

            game_world.window_.clear(sf::Color::Black);
            game_world.window_.display();

            frame_count++;
        }

        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}

/**
 * @section advanced_usage Advanced Usage Patterns
 *
 * @subsection pattern1 Pattern 1: Temporary Audio Entity
 * @code
 * void PlayOneShot(Engine::registry &reg, const std::string &sound) {
 *     auto entity = reg.SpawnEntity();
 *     reg.EmplaceComponent<Component::SoundRequest>(
 *         entity, Component::SoundRequest{sound, 1.0f, false});
 *     // Entity will be cleaned up after sound plays
 * }
 * @endcode
 *
 * @subsection pattern2 Pattern 2: Attach Sound to Game Entity
 * @code
 * void PlayerShoot(Engine::registry &reg, Engine::entity player) {
 *     // Add SoundRequest to player entity
 *     reg.EmplaceComponent<Component::SoundRequest>(
 *         player, Component::SoundRequest{"laser", 0.8f, false});
 *     // Sound will be removed after playing, entity remains
 * }
 * @endcode
 *
 * @subsection pattern3 Pattern 3: Dynamic Volume Based on Game State
 * @code
 * void UpdateAudioVolume(Audio::AudioManager &manager, float health) {
 *     // Reduce volume as health decreases (muffled hearing effect)
 *     float volume = 0.3f + (health / 100.0f) * 0.7f;
 *     manager.SetSfxVolume(volume);
 * }
 * @endcode
 *
 * @subsection pattern4 Pattern 4: Music Transition
 * @code
 * void SwitchToBossMusic(Audio::AudioManager &manager) {
 *     manager.StopMusic();
 *     manager.PlayMusic("boss_bgm", true);
 * }
 * @endcode
 */
