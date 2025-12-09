#include "game/InitRegistry.hpp"

#include "engine/InitRegistryComponent.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
void InitRegistry(Rtype::Client::GameWorld &game_world) {
    RC::InitRegistryComponents(game_world.registry_);
    RC::InitRegistrySystemsEvents(game_world);
    RC::InitRegistrySystems(game_world);
}
}  // namespace Rtype::Client
