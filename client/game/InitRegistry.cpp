#include "game/InitRegistry.hpp"

#include "engine/InitRegistryComponent.hpp"
#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
void InitRegistry(
    Rtype::Client::GameWorld &game_world, Audio::AudioManager &audio_manager) {
    RC::InitRegistryComponents(game_world.registry_);
    RC::InitRegistrySystemsEvents(game_world);
    RC::InitRegistrySystems(game_world, audio_manager);
}
}  // namespace Rtype::Client
