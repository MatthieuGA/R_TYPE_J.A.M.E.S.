#pragma once
#include "include/components/CoreComponents.hpp"
#include "include/components/GameplayComponents.hpp"
#include "include/components/NetworkingComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
void InitRegistryComponents(Engine::registry &reg);
}  // namespace Rtype::Client
