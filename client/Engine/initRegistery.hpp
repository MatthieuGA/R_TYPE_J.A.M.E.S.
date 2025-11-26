#pragma once

#include "include/Components/CoreComponents.hpp"
#include "include/Components/GameplayComponents.hpp"
#include "include/Components/NetworkingComponents.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
void init_registry_components(Engine::registry &reg);
}  // namespace Rtype::Client