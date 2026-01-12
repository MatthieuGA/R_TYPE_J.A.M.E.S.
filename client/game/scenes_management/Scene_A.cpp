#include "game/scenes_management/Scene_A.hpp"

#include <string>

#include "include/ColorsConst.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"

namespace Rtype::Client {
void Scene_A::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    // Implementation for initializing the scene
}

void Scene_A::DestroyScene(Engine::registry &reg) {
    for (auto &entity : scene_entities_)
        reg.KillEntity(entity);
    scene_entities_.clear();  // Clear stale entity list for next init
}

Engine::entity Scene_A::CreateEntityInScene(Engine::registry &reg) {
    auto entity = reg.SpawnEntity();
    scene_entities_.push_back(entity);
    return entity;
}

Engine::entity Scene_A::CreateButton(Engine::registry &reg,
    const std::string &label, float x, float y, std::function<void()> on_click,
    float scale) {
    auto button_entity = CreateEntityInScene(reg);

    reg.AddComponent<Component::Transform>(button_entity,
        Component::Transform{x, y, 0.0f, scale, Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        button_entity, Component::Drawable{"ui/button.png", LAYER_UI, 1.0f});
    reg.AddComponent<Component::HitBox>(
        button_entity, Component::HitBox{128.f, 32.f});
    reg.AddComponent<Component::Clickable>(
        button_entity, Component::Clickable{on_click});
    reg.AddComponent<Component::Text>(
        button_entity, Component::Text("dogica.ttf", label, 13, LAYER_UI + 1,
                           WHITE_BLUE, sf::Vector2f(0.0f, -5.0f)));

    return button_entity;
}

}  // namespace Rtype::Client
