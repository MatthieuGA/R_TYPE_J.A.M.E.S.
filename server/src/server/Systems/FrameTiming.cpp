#include <algorithm>

#include "server/systems/Systems.hpp"

namespace server {

// Define frame delta globals (single definition for the program)
float g_frame_delta_ms = 16.0f;
float g_frame_delta_seconds = 16.0f / 1000.0f;
float g_game_speed_multiplier = 1.0f;       // Game speed (set by client)
uint8_t g_difficulty_level = 1;             // 0=Easy, 1=Normal, 2=Hard
bool g_killable_enemy_projectiles = false;  // Can player projectiles destroy
                                            // enemy projectiles

/**
 * @brief Update global frame delta from elapsed seconds (clamped to max FPS)
 *
 * @param seconds Elapsed seconds since last frame
 */
void UpdateFrameDeltaFromSeconds(float seconds) {
    // Enforce maximum 60 FPS (minimum delta)
    if (seconds < kMinFrameDeltaSeconds)
        seconds = kMinFrameDeltaSeconds;
    // Apply game speed multiplier
    g_frame_delta_seconds = seconds * g_game_speed_multiplier;
    g_frame_delta_ms = g_frame_delta_seconds * 1000.0f;
}

}  // namespace server
