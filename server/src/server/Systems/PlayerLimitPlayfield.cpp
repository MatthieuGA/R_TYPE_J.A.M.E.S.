#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameWorldDatas.hpp"
#include "server/GameplayComponents.hpp"
#include "server/systems/Systems.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

void PlayerLimitPlayfield(Engine::registry &reg,
    Engine::sparse_array<Component::Transform> &transforms,
    Engine::sparse_array<Component::PlayerTag> const &player_tags) {
    for (auto &&[i, transform, player_tag] :
        make_indexed_zipper(transforms, player_tags)) {
        if (player_tag.isInPlay == false)
            continue;
        // Update position based on velocity
        if (transform.x < 0)
            transform.x = 0;
        if (transform.y < 0)
            transform.y = 0;
        if (transform.x > WINDOW_WIDTH)
            transform.x = static_cast<float>(WINDOW_WIDTH);
        if (transform.y > WINDOW_HEIGHT)
            transform.y = static_cast<float>(WINDOW_HEIGHT);
    }
}

}  // namespace server
