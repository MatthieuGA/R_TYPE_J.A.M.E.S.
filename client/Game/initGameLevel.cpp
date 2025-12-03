#include "include/registry.hpp"
#include "Game/initGameLevel.hpp"
#include "include/Components/CoreComponents.hpp"
#include "include/Components/GameplayComponents.hpp"

namespace Rtype::Client {

void add_background_entity(Engine::registry &reg, const std::string &path,
float speed, float initial_x, float initial_y = 0.0f) {
    auto background_entity = reg.SpawnEntity();
    reg.AddComponent<Component::Transform>(background_entity,
        Component::Transform{initial_x, initial_y, 0.0f, 3.34f,
        Component::Transform::TOP_LEFT});
    reg.AddComponent<Component::Drawable>(background_entity,
        Component::Drawable{path, 0});
    reg.AddComponent<Component::ParrallaxLayer>(background_entity,
        Component::ParrallaxLayer{speed});
}

void init_game_level(registry &reg) {
    std::vector<std::pair<std::string, std::pair<float, float>>> background_list = {
        {"Background/Level1/1.png", {-5.0f, 0.0f}},
        {"Background/Level1/2.png", {-15.0f, 0.0f}},
        {"Background/Level1/3.png", {-25.0f, 0.0f}},
        {"Background/Level1/4.png", {-35.0f, 0.0f}},
        {"Background/Level1/5.png", {-150.0f, -20.0f}},
        {"Background/Level1/5.png", {-130.0f, -200.0f}}
    };

    // Initialize background entity
    for (const auto& [path, speed_pos] : background_list) {
        for (int i = 0; i < 2; i++) {
            add_background_entity(reg, path, speed_pos.first,
                static_cast<float>(i) * 1920.0f, speed_pos.second);
        }
    }
}

}  // namespace Rtype::Client