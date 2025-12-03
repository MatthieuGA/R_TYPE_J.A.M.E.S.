#include "include/registry.hpp"
#include "Game/initGameLevel.hpp"
#include "include/Components/CoreComponents.hpp"
#include "include/Components/GameplayComponents.hpp"

namespace Rtype::Client {

void add_background_entity(Engine::registry &reg, const std::string &path,
float speed, float initial_x, float initial_y = 0.0f, int z_index = 0) {
    auto background_entity = reg.SpawnEntity();
    reg.AddComponent<Component::Transform>(background_entity,
        Component::Transform{initial_x, initial_y, 0.0f, 3.34f,
        Component::Transform::TOP_LEFT});
    reg.AddComponent<Component::Drawable>(background_entity,
        Component::Drawable{path, z_index});
    reg.AddComponent<Component::ParrallaxLayer>(background_entity,
        Component::ParrallaxLayer{speed});
}

void init_game_level(registry &reg) {
    struct background_info {
        std::string path;
        float scrollSpeed;
        float initialY;
        int z_index;
    };
    std::vector<background_info> background_list = {
        {"Background/Level1/2.png", -15.0f, 0.0f, -9},
        {"Background/Level1/3.png", -25.0f, 0.0f, -8},
        {"Background/Level1/4.png", -35.0f, 0.0f, -7},
        {"Background/Level1/1.png", -5.0f, 0.0f, -10},
        {"Background/Level1/5.png", -150.0f, -20.0f, 10},
        {"Background/Level1/5.png", -130.0f, -200.0f, 11}
    };

    // Initialize background entity
    for (const auto& background : background_list) {
        for (int i = 0; i < 2; i++) {
            add_background_entity(reg, background.path, background.scrollSpeed,
                static_cast<float>(i) * 1920.0f, background.initialY, background.z_index);
        }
    }
}

}  // namespace Rtype::Client