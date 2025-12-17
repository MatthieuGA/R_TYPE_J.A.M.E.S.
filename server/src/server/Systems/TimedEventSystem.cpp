#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

/**
 * @brief Handles cooldown-based shooting actions for an entity.
 * @param entityId ID of the entity
 * @param deltaTime Time elapsed since last frame
 * @param time_event The cooldown action to handle
 */
void HandleCooldownBasedShooting(int entityId, float deltaTime,
    Component::TimedEvents::CooldownAction &time_event) {
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
void TimedEventSystem(Engine::registry &reg,
    Engine::sparse_array<Component::TimedEvents> &timed_events) {
    for (auto &&[i, timed_event] : make_indexed_zipper(timed_events)) {
        for (auto &cd_action : timed_event.cooldown_actions)
            HandleCooldownBasedShooting(i, TICK_RATE_SECONDS, cd_action);
    }
}
}  // namespace server
