#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <SFML/Graphics.hpp>

#include "include/registry.hpp"
#include "Game/ScenesManagement/Scene_A.hpp"

namespace Eng = Engine;

namespace Rtype::Client::Component {
struct SceneManagement {
    std::string current = "";
    std::string next = "";
    std::unordered_map<std::string, std::shared_ptr<Scene_A>> scenes;
};

}  // namespace Rtype::Client::Component
