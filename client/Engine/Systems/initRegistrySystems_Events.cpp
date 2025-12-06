#include "Engine/Systems/initRegistrySystems.hpp"
#include "include/indexed_zipper.hpp"
#include "Engine/Events/EngineEvent.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

/**
 * @brief Initialize event-related systems in the registry.
 *
 * This function sets up the necessary event subscriptions
 * within the provided game world's event bus.
 *
 * @param game_world The game world containing the registry.
 */
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
