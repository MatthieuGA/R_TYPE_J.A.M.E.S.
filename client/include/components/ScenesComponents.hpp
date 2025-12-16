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

}  // namespace Rtype::Client::Component
