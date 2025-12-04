#include <string>
#include <vector>

#include "include/registry.hpp"
#include "Game/initGameLevel.hpp"
#include "include/Components/CoreComponents.hpp"
#include "include/Components/GameplayComponents.hpp"

namespace Rtype::Client {

struct background_info {
    std::string path;
    float scroll_speed;
    float initialY;
    int z_index;
    bool isWave = false;
    float frequency = 20.0f;
    float amplitude = 0.002f;
    float wave_speed = 2.0f;
    float opacity = 1.0f;
    float scale = 3.34f;
};

void add_background_entity(Engine::registry &reg, background_info info,
float initial_x) {
    auto background_entity = reg.SpawnEntity();
    reg.AddComponent<Component::Transform>(background_entity,
        Component::Transform{initial_x, info.initialY, 0.0f, info.scale,
        Component::Transform::TOP_LEFT});
    if (info.isWave) {
        reg.AddComponent<Component::Shader>(background_entity,
            Component::Shader{"wave.frag",
            {
                {"speed", info.wave_speed},
                {"amplitude", info.amplitude},
                {"frequency", info.frequency}
            }
            });
    }
    reg.AddComponent<Component::Drawable>(background_entity,
        Component::Drawable{info.path, info.z_index, info.opacity});
    reg.AddComponent<Component::ParrallaxLayer>(background_entity,
        Component::ParrallaxLayer{info.scroll_speed});
}

void init_backgrounds(Engine::registry &reg) {
    std::vector<background_info> background_list = {
        {"Background/Level1/1.png", -5.f, 0.f, -10, true, 1.f, .0005f, 0.2f},
        {"Background/Level1/2.png", -15.f, 0.f, -9, true, 6.f, .007f, 1.2f},
        {"Background/Level1/3.png", -25.f, 0.f, -8, true, 6.f, .007f, 1.2f},
        {"Background/Level1/4.png", -35.f, 0.f, -7, true, 4.f, .005f, 1.5f},
        {"Background/Level1/5.png", -150.f, -20.f, 10, false, 0.f, 0.f, 0.f,
            0.8f},
        {"Background/Level1/5.png", -130.f, -200.f, 11, false, 0.f, 0.f, 0.f,
            0.6f},
        {"Background/Level1/WaterEffect.jpg", -50.f, 0.f, 12, true, 10.f,
            .01f, 2.0f, 0.1f, 3.84f}
    };

    // Initialize background entity
    for (const auto& background : background_list) {
        for (int i = 0; i < 2; i++)
            add_background_entity(reg, background, i * 1920.0f);
    }
}

void init_player_level(Engine::registry &reg) {
    auto player_entity = reg.SpawnEntity();
    reg.AddComponent<Component::Transform>(player_entity,
        Component::Transform{100.0f, 300.0f, 0.0f, 4.0f,
        Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(player_entity,
        Component::Drawable{"OriginalRtype/r-typesheet42.gif"});
    reg.AddComponent<Component::AnimatedSprite>(player_entity,
        Component::AnimatedSprite(33, 19, 2));
    reg.AddComponent<Component::Controllable>(player_entity,
        Component::Controllable{true});
    reg.AddComponent<Component::Inputs>(player_entity, Component::Inputs{});
    reg.AddComponent<Component::Velocity>(player_entity,
        Component::Velocity{0.0f, 0.0f, 0.0f, 0.0f});
    reg.AddComponent<Component::PlayerTag>(player_entity,
        Component::PlayerTag{400.0f});
}

void init_game_level(registry &reg) {
    init_backgrounds(reg);
    init_player_level(reg);
}

}  // namespace Rtype::Client
