#pragma once
#include "include/registry.hpp"

namespace Rtype::Client {
using Engine::registry;

void init_scene_level(registry &reg);

void init_game_level(registry &reg);

}  // namespace Rtype::Client
