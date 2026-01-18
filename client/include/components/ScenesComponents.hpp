#pragma once
#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

#include "game/scenes_management/Scene_A.hpp"

namespace Rtype::Client::Component {
struct SceneManagement {
    std::string current = "";
    std::string next = "";
    std::unordered_map<std::string, std::shared_ptr<Scene_A>> scenes;
};

/**
 * @brief Component to tag and configure lobby UI elements for dynamic updates.
 *
 * This component identifies entities that display lobby-related information
 * and should be updated when the lobby status changes.
 */
struct LobbyUI {
    enum class Type {
        PlayerCount,  // Text showing "Players: X/Y"
        ReadyCount,   // Text showing "Ready: X/Y"
        ReadyButton   // Button that changes appearance based on ready state
    };

    Type ui_type;

    explicit LobbyUI(Type type) : ui_type(type) {}
};

/**
 * @brief Component to track game over state and visual effects.
 *
 * Manages the "GAME OVER" / "VICTORY" text display, leaderboard, and
 * fade transition timing.
 */
struct GameOverState {
    bool is_active{false};      // Whether game over sequence is in progress
    float display_timer{0.0f};  // Time spent displaying result text
    float leaderboard_timer{0.0f};  // Time spent displaying leaderboard
    float fade_timer{0.0f};         // Time spent fading to lobby
    bool text_phase{true};          // True = showing result text
    bool leaderboard_phase{false};  // True = showing leaderboard
    bool is_victory{false};         // True if this client won
    static constexpr float kTextDuration = 2.0f;  // 2 seconds result text
    static constexpr float kLeaderboardDuration =
        5.0f;                                     // 5 seconds leaderboard
    static constexpr float kFadeDuration = 1.5f;  // 1.5 seconds fade
};

/**
 * @brief Component for the fade overlay used during game over transition.
 */
struct FadeOverlay {
    float alpha{0.0f};  // 0 = transparent, 255 = fully opaque black
};

/**
 * @brief Component to tag the "GAME OVER" / "VICTORY" text entity.
 */
struct GameOverText {
    bool visible{false};
};

/**
 * @brief Component to tag leaderboard text entities.
 */
struct LeaderboardText {
    int rank{0};  // 0 = title, 1+ = player entry
    bool visible{false};
};

}  // namespace Rtype::Client::Component
