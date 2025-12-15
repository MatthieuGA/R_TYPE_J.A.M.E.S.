/**
 * @file exampleGameplaySystems.cpp
 * @brief Example gameplay systems demonstrating audio integration.
 *
 * These are NOT part of the current build - just examples showing how to
 * integrate the audio subsystem into gameplay code.
 */

#include <vector>

#include "include/Components/CoreComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"

namespace Rtype::Client::Examples {
namespace Com = Component;
namespace Eng = Engine;

/**
 * @brief Example: Shooting system with audio feedback.
 *
 * When a player shoots, this system:
 * 1. Creates a projectile entity
 * 2. Adds a SoundRequest component to trigger the shot sound
 */
void ExampleShootingSystem(Eng::registry &reg,
    Eng::sparse_array<Com::InputState> const &inputs,
    Eng::sparse_array<Com::Transform> &transforms) {
    for (auto &&[i, input, transform] :
        make_indexed_zipper(inputs, transforms)) {
        if (input.shoot) {
            // Create projectile entity
            auto projectile = reg.spawn_entity();

            // Add gameplay components
            reg.emplace_component<Com::Transform>(projectile,
                Com::Transform{transform.x + 50.0f, transform.y, 0.0f, 1.0f});
            reg.emplace_component<Com::RigidBody>(
                projectile, Com::RigidBody{300.0f, 0.0f});

            // Request shot sound
            reg.emplace_component<Com::SoundRequest>(
                projectile, Com::SoundRequest{"player_shot", 1.0f, false});
        }
    }
}

/**
 * @brief Example: Collision system with explosion sounds.
 *
 * When a collision is detected, this system:
 * 1. Handles collision logic (damage, destruction)
 * 2. Creates a temporary entity with SoundRequest for explosion
 */
void ExampleCollisionSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::HitBox> const &hitboxes) {
    // Simplified collision detection
    std::vector<std::pair<size_t, size_t>> collisions;

    // ... collision detection logic ...

    for (const auto &[entity_a, entity_b] : collisions) {
        // Handle collision damage/destruction
        // ...

        // Get position for sound (from entity_a)
        const auto &transform = transforms[entity_a];

        // Create temporary entity for explosion sound
        auto sound_entity = reg.spawn_entity();
        reg.emplace_component<Com::SoundRequest>(
            sound_entity, Com::SoundRequest{"explosion", 0.8f, false});

        // Optionally add a visual effect entity with the same position
        auto fx_entity = reg.spawn_entity();
        reg.emplace_component<Com::Transform>(
            fx_entity, Com::Transform{transform.value().x, transform.value().y,
                           0.0f, 1.0f});
        // ... add explosion animation component ...
    }
}

/**
 * @brief Example: Power-up collection with sound.
 *
 * When player collects a power-up:
 * 1. Destroys the power-up entity
 * 2. Applies power-up effect
 * 3. Plays collection sound
 */
void ExamplePowerUpSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &player_transforms,
    Eng::sparse_array<Com::Transform> const &powerup_transforms) {
    std::vector<size_t> collected_powerups;

    // Check for player-powerup collisions
    for (auto &&[player_idx, player_pos] :
        Engine::indexed_zipper(player_transforms)) {
        for (auto &&[powerup_idx, powerup_pos] :
            Engine::indexed_zipper(powerup_transforms)) {
            float dx = player_pos.x - powerup_pos.x;
            float dy = player_pos.y - powerup_pos.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < 30.0f) {  // Collection radius
                // Apply power-up effect
                // ...

                // Play collection sound
                auto sound_entity = reg.spawn_entity();
                reg.emplace_component<Com::SoundRequest>(sound_entity,
                    Com::SoundRequest{"powerup_collect", 0.9f, false});

                // Mark powerup for deletion
                collected_powerups.push_back(powerup_idx);
            }
        }
    }

    // Remove collected power-ups
    for (size_t idx : collected_powerups) {
        reg.kill_entity(reg.entity_from_index(idx));
    }
}

/**
 * @brief Example: Enemy death with sound.
 *
 * When an enemy dies:
 * 1. Destroys enemy entity
 * 2. Spawns explosion visual
 * 3. Plays death sound
 * 4. Possibly spawns power-up
 */
void ExampleEnemyDeathSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Health> const &healths,
    Eng::sparse_array<Com::Transform> const &transforms) {
    std::vector<size_t> dead_enemies;

    for (auto &&[enemy_idx, health, transform] :
        make_indexed_zipper(healths, transforms)) {
        if (health.current <= 0) {
            // Play death sound (different sound based on enemy type)
            auto sound_entity = reg.spawn_entity();
            reg.emplace_component<Com::SoundRequest>(
                sound_entity, Com::SoundRequest{"enemy_death", 0.7f, false});

            // Spawn explosion visual effect
            auto explosion = reg.spawn_entity();
            reg.emplace_component<Com::Transform>(explosion,
                Com::Transform{transform.x, transform.y, 0.0f, 1.0f});
            // ... add explosion animation component ...

            dead_enemies.push_back(enemy_idx);
        }
    }

    // Remove dead enemies
    for (size_t idx : dead_enemies) {
        reg.kill_entity(reg.entity_from_index(idx));
    }
}

/**
 * @brief Example: UI button click with sound.
 *
 * When a UI button is clicked, play a click sound.
 */
void ExampleUIClickSystem(Eng::registry &reg,
    /* ... button/UI components ... */) {
    // Detect button click
    bool button_clicked = false;  // ... actual click detection logic ...

    if (button_clicked) {
        // Play UI click sound
        auto sound_entity = reg.spawn_entity();
        reg.emplace_component<Com::SoundRequest>(
            sound_entity, Com::SoundRequest{"ui_click", 0.5f, false});
    }
}

/**
 * @brief Example: Level start/transition with music.
 *
 * This would be called from a game state manager, NOT from a regular system.
 * It's shown here for completeness.
 */
void ExampleLevelTransition(Audio::AudioManager &audio_manager) {
    // Stop current music
    audio_manager.StopMusic();

    // Play level-specific background music
    audio_manager.PlayMusic("level_1_bgm", true);  // true = loop

    // Play a level-start jingle (non-looping)
    // Note: You'd need a separate entity for this
    // audio_manager.PlaySound("level_start", 0.8f);
}

/**
 * @brief Example: Boss battle music trigger.
 *
 * When boss spawns, change the background music.
 */
void ExampleBossMusicSystem(Eng::registry &reg,
    Audio::AudioManager &audio_manager, bool boss_spawned) {
    static bool boss_music_playing = false;

    if (boss_spawned && !boss_music_playing) {
        audio_manager.StopMusic();
        audio_manager.PlayMusic("boss_battle_bgm", true);
        boss_music_playing = true;
    }
}

}  // namespace Rtype::Client::Examples
