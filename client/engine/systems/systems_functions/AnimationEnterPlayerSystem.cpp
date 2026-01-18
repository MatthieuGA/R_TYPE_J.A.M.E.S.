#include <vector>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

/**
 * @brief System to handle player entry animation.
 *
 * This system moves players into the play area when they are entering.
 * Once they reach the target position, it marks them as in play and
 * adds the Controllable component.
 *
 * @param reg ECS registry used to access entities and components.
 * @param game_world The game world context.
 * @param velocities Sparse array of Velocity components.
 * @param transforms Sparse array of Transform components.
 * @param player_tags Sparse array of PlayerTag components.
 * @param animation_enter_players Sparse array of AnimationEnterPlayer
 * components.
 */
void AnimationEnterPlayerSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> &player_tags,
    Eng::sparse_array<Com::AnimationEnterPlayer> &animation_enter_players) {
    // Collect entities that have finished entering to avoid modifying sparse
    // arrays during iteration
    std::vector<std::size_t> finished_entering;

    for (auto &&[i, transform, player_tag, animation_enter_player, velocity] :
        make_indexed_zipper(
            transforms, player_tags, animation_enter_players, velocities)) {
        if (animation_enter_player.isEntering) {
            velocity.vx = player_tag.speed_max;
            if (transform.x >= 75.0f) {
                transform.x = 75.0f;
                animation_enter_player.isEntering = false;
                player_tag.isInPlay = true;
                finished_entering.push_back(i);
            }
        }
    }

    // Now modify registry after iteration is complete
    for (std::size_t i : finished_entering) {
        Engine::entity entity = reg.EntityFromIndex(i);
        reg.AddComponent(entity, Com::Controllable{});
        reg.RemoveComponent<Com::AnimationEnterPlayer>(entity);
    }
}
}  // namespace Rtype::Client
