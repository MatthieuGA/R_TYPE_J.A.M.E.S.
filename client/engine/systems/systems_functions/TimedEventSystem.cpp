#include <string>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

/**
 * @brief Handles cooldown-based shooting actions for an entity.
 * @param entityId ID of the entity
 * @param deltaTime Time elapsed since last frame
 * @param time_event The cooldown action to handle
 */
void HandleCooldownBasedShooting(int entityId, float deltaTime,
    Com::TimedEvents::CooldownAction &time_event) {
    time_event.cooldown += deltaTime;
    if (time_event.cooldown > time_event.cooldown_max) {
        time_event.cooldown = 0.0f;

        // Execute custom action if set
        if (time_event.action)
            time_event.action(entityId);
    }
}

/**
 * @brief System to process timed events for entities.
 * @param reg The registry containing entities and components
 * @param game_world The game world context
 * @param timed_events Sparse array of TimedEvents components
 */
void TimedEventSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::TimedEvents> &timed_events) {
    for (auto &&[i, timed_event] : make_indexed_zipper(timed_events)) {
        for (auto &cd_action : timed_event.cooldown_actions)
            HandleCooldownBasedShooting(i, game_world.last_delta_, cd_action);
    }
}
}  // namespace Rtype::Client
