#pragma once
#include <string>

#include "game/scenes_management/Scene_A.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
using Engine::registry;

class GameScene : public Scene_A {
 public:
    GameScene() = default;
    void InitScene(registry &reg, GameWorld &gameWorld) override;

 private:
    struct BackgroundInfo {
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
    void AddBackgroundEntity(
<<<<<<< HEAD:client/Game/ScenesManagement/Scenes/GameScene.hpp
        registry &reg, struct background_info info, float initial_x);
=======
        registry &reg, struct BackgroundInfo info, float initial_x);
>>>>>>> 3349563e8187828e85f2715d5f173b8deb965efc:client/game/scenes_management/scenes/GameScene.hpp
};
}  // namespace Rtype::Client
