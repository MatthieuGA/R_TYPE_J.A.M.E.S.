#pragma once
#include <string>
#include <map>
#include <memory>
#include <vector>
#include <utility>
#include <cstdint>
#include <optional>
#include <SFML/Graphics.hpp>

#include "include/registry.hpp"

namespace Eng = Engine;

namespace Rtype::Client::Component {
using StateFn = std::function<void(Eng::registry& registry)>;

struct SceneManagement {
    struct GameStateCallbacks {
        StateFn onEnter;
        StateFn onExit;
    };

    std::string current = "";
    std::string next = "";
    std::unordered_map<std::string, GameStateCallbacks> table = {};
};

}  // namespace Rtype::Client::Component
