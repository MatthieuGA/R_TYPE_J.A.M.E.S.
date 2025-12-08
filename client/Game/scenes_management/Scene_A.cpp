#include "client/Game/scenes_management/Scene_A.hpp"

namespace Rtype::Client {
void Scene_A::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    // Implementation for initializing the scene
}

void Scene_A::DestroyScene(Engine::registry &reg) {
    for (auto &entity : scene_entities_)
        reg.KillEntity(entity);
}

Engine::entity Scene_A::CreateEntityInScene(Engine::registry &reg) {
    auto entity = reg.SpawnEntity();
    scene_entities_.push_back(entity);
    return entity;
}

}  // namespace Rtype::Client
