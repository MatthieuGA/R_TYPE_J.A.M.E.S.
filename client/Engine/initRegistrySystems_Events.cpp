#include "Engine/Events/EngineEvent.hpp"
#include "Engine/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

void InitRegistrySystemsEvents(Rtype::Client::GameWorld &game_world) {
    game_world.event_bus_.Subscribe<CollisionEvent>(
        [](const CollisionEvent &event, int) {
            auto &player =
                event.game_world_.registry_.GetComponents<Com::PlayerTag>();

            if (player.has(event.entity_a_) || player.has(event.entity_b_)) {
                // Handle player collision event
            }
        });
}
}  // namespace Rtype::Client
