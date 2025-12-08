#include "Game/initRegistry.hpp"

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
void init_registry(Rtype::Client::GameWorld &game_world) {
    RC::InitRegistryComponents(game_world.registry_);
    RC::InitRegistrySystemsEvents(game_world);
    RC::InitRegistrySystems(game_world);
}
}  // namespace Rtype::Client
