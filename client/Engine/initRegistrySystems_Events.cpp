#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"
#include "Engine/Events/EngineEvent.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

void init_registry_systems_events(Rtype::Client::GameWorld &gameWorld) {
    gameWorld.eventBus.subscribe<CollisionEvent>(
        [](const CollisionEvent &event, int) {
            auto &player = event.gameWorld.registry.get_components<Com::PlayerTag>();

            if (player.has(event.entityA) || player.has(event.entityB)) {
                // Handle player collision event
            }
        });
}
}  // namespace Rtype::Client
