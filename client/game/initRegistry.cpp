#include "game/initRegistry.hpp"

#include "engine/initRegistryComponent.hpp"
#include "engine/systems/initRegistrySystems.hpp"

namespace Rtype::Client {
void InitRegistry(Rtype::Client::GameWorld &game_world) {
    RC::InitRegistryComponents(game_world.registry_);
    RC::InitRegistrySystemsEvents(game_world);
    RC::InitRegistrySystems(game_world);
}
}  // namespace Rtype::Client
