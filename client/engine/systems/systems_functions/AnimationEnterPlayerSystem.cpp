#include "engine/systems/InitRegistrySystems.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

void AnimationEnterPlayerSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Velocity> &velocities,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> &player_tags,
    Eng::sparse_array<Com::AnimationEnterPlayer> &animation_enter_players) {
    for (auto &&[i, transform, player_tag, animation_enter_player, velocity] :
        make_indexed_zipper(
            transforms, player_tags, animation_enter_players, velocities)) {
        if (animation_enter_player.isEntering) {
            velocity.vx = player_tag.speed_max;
            if (transform.x >= 75.0f) {
                transform.x = 75.0f;
                animation_enter_player.isEntering = false;
                player_tag.isInPlay = true;
                reg.AddComponent(reg.EntityFromIndex(i), Com::Controllable{});
                reg.RemoveComponent<Com::AnimationEnterPlayer>(
                    reg.EntityFromIndex(i));
            }
        }
    }
}
}  // namespace Rtype::Client
