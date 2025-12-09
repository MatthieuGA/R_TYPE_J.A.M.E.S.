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

}  // namespace Rtype::Client::Component
