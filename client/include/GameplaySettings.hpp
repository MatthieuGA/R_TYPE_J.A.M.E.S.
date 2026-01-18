/**
 * @file GameplaySettings.hpp
 * @brief Gameplay settings for accessibility and comfort (speed, auto-fire,
 * etc).
 */

#pragma once

#include <cstdint>

namespace Rtype::Client {

/**
 * @brief Enum for difficulty levels.
 */
enum class DifficultyLevel {
    Easy = 0,    ///< Reduced damage taken, easier gameplay
    Normal = 1,  ///< Standard gameplay
    Hard = 2     ///< Increased enemy fire rate
};

/**
 * @brief Container for gameplay settings.
 *
 * These settings provide accessibility and comfort features
 * without modifying core game rules or ECS structure.
 */
struct GameplaySettings {
    // Game speed (multiplier for gameplay timers)
    float game_speed = 1.0f;  // 0.25x to 2.0x

    // Auto-fire: holding shoot continuously fires (accessibility feature)
    bool auto_fire_enabled = false;

    // Killable enemy projectiles: player projectiles can destroy enemy
    // projectiles
    bool killable_enemy_projectiles = false;

    // Difficulty level (affects damage/fire rate)
    DifficultyLevel difficulty = DifficultyLevel::Normal;

    /**
     * @brief Get damage taken multiplier based on difficulty.
     * @return Multiplier (< 1 = easy, 1 = normal, > 1 = hard)
     */
    float GetDamageMultiplier() const {
        switch (difficulty) {
            case DifficultyLevel::Easy:
                return 0.75f;  // 25% damage reduction
            case DifficultyLevel::Normal:
                return 1.0f;
            case DifficultyLevel::Hard:
                return 1.1f;  // 10% damage increase
            default:
                return 1.0f;
        }
    }

    /**
     * @brief Get enemy fire rate multiplier based on difficulty.
     * @return Multiplier (1 = normal, > 1 = more frequent fire)
     */
    float GetEnemyFireRateMultiplier() const {
        switch (difficulty) {
            case DifficultyLevel::Easy:
                return 0.9f;  // 10% slower fire rate
            case DifficultyLevel::Normal:
                return 1.0f;
            case DifficultyLevel::Hard:
                return 1.15f;  // 15% faster fire rate
            default:
                return 1.0f;
        }
    }
};

}  // namespace Rtype::Client
