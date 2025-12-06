#pragma once
#include "include/registry.hpp"
#include "Game/ScenesManagement/Scene_A.hpp"

namespace Rtype::Client {
using Engine::registry;

class MainMenuScene : public Scene_A {
public:
    MainMenuScene() = default;
    void InitScene(registry &reg) override;
};
}  // namespace Rtype::Client