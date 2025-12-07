#pragma once
#include "include/registry.hpp"
#include "Game/ScenesManagement/Scene_A.hpp"

namespace Rtype::Client {
using Engine::registry;

class GameScene : public Scene_A {
public:
    GameScene() = default;
    void InitScene(registry &reg, GameWorld &gameWorld) override;

private:
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

    void InitPlayerLevel(registry &reg);
    void InitBackgrounds(registry &reg);
    void AddBackgroundEntity(registry &reg, struct background_info info,
        float initial_x);
};
}  // namespace Rtype::Client