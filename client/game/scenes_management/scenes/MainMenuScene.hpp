#pragma once
#include "game/scenes_management/Scene_A.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
using Engine::registry;

class MainMenuScene : public Scene_A {
 public:
    MainMenuScene() = default;
    void InitScene(registry &reg, GameWorld &gameWorld) override;
};
}  // namespace Rtype::Client
