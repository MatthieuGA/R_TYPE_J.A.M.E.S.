/**
 * @file TestRenderScene.hpp
 * @brief Minimal test scene to verify rendering API works
 */

#pragma once

#include "game/scenes_management/Scene_A.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
using Engine::registry;

/**
 * @brief Minimal test scene - just draw 3 simple sprites
 */
class TestRenderScene : public Scene_A {
 public:
    TestRenderScene() = default;
    void InitScene(registry &reg, GameWorld &gameWorld) override;
};

}  // namespace Rtype::Client
