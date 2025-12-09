#pragma once

#include <string>

#include "game/scenes_management/Scene_A.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
using Engine::registry;

class MainMenuScene : public Scene_A {
 public:
    MainMenuScene() = default;
    void InitScene(registry &reg, GameWorld &gameWorld) override;

 private:
    struct BackgroundInfo {
        std::string path;
        float initialY;
        int z_index;
        bool isWave = false;
        float frequency = 20.0f;
        float amplitude = 0.002f;
        float wave_speed = 2.0f;
        float opacity = 1.0f;
        float scale = 3.34f;
    };

    void InitUI(registry &reg, GameWorld &gameWorld);
    void InitBackground(registry &reg);
};
}  // namespace Rtype::Client
