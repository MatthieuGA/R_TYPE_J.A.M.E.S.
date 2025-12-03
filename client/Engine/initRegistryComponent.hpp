#pragma once
#include "include/Components/CoreComponents.hpp"
#include "include/Components/GameplayComponents.hpp"
#include "include/Components/NetworkingComponents.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
void InitRegistryComponents(Engine::registry &reg);
}  // namespace Rtype::Client
