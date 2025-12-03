#include "include/registry.hpp"
#include "Game/initGameLevel.hpp"
#include "include/Components/CoreComponents.hpp"
#include "include/Components/GameplayComponents.hpp"

namespace Rtype::Client {

void init_game_level(registry &reg) {
    // Initialize background entity
    for (int i = 0; i < 2; i++) {
        auto background_entity = reg.SpawnEntity();
        reg.AddComponent<Component::Transform>(background_entity, Component::Transform{i * 1920.f, 0.0f, 0.0f, 3.34f, Component::Transform::TOP_LEFT});
        reg.AddComponent<Component::Drawable>(background_entity, Component::Drawable{"Background/Level1/1.png", 0});
        reg.AddComponent<Component::ParrallaxLayer>(background_entity, Component::ParrallaxLayer{-300.0f});
    }
}

}  // namespace Rtype::Client