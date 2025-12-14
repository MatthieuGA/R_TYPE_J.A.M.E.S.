/**
 * @file TestRenderScene.cpp
 * @brief Minimal test scene implementation
 */

#include "game/scenes_management/scenes/TestRenderScene.hpp"

#include <iostream>

#include "include/ColorsConst.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"

namespace Rtype::Client {

void TestRenderScene::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    std::cout << "[TestRenderScene] Initializing minimal test scene..."
              << std::endl;

    // Test 1: Draw a sprite at top-left
    auto entity1 = reg.SpawnEntity();
    reg.AddComponent<Component::Transform>(
        entity1, Component::Transform{100.0f, 100.0f, 0.0f, 2.0f,
                     Component::Transform::TOP_LEFT});
    reg.AddComponent<Component::Drawable>(
        entity1, Component::Drawable{"ui/button.png", LAYER_UI, 1.0f});
    std::cout << "[TestRenderScene] Created entity at (100, 100)" << std::endl;

    // Test 2: Draw a sprite at center
    auto entity2 = reg.SpawnEntity();
    reg.AddComponent<Component::Transform>(
        entity2, Component::Transform{960.0f, 540.0f, 0.0f, 3.0f,
                     Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        entity2, Component::Drawable{"ui/button.png", LAYER_UI, 1.0f});
    std::cout << "[TestRenderScene] Created entity at (960, 540)" << std::endl;

    // Test 3: Draw a sprite at bottom-right
    auto entity3 = reg.SpawnEntity();
    reg.AddComponent<Component::Transform>(
        entity3, Component::Transform{1720.0f, 980.0f, 0.0f, 2.0f,
                     Component::Transform::TOP_LEFT});
    reg.AddComponent<Component::Drawable>(
        entity3, Component::Drawable{"ui/button.png", LAYER_UI, 1.0f});
    std::cout << "[TestRenderScene] Created entity at (1720, 980)"
              << std::endl;

    std::cout << "[TestRenderScene] Scene initialized with 3 test entities"
              << std::endl;
}

}  // namespace Rtype::Client
