#pragma once
#include <vector>

#include "include/registry.hpp"
#include "Engine/gameWorld.hpp"

namespace Rtype::Client {
class Scene_A {
 public:
    virtual void InitScene(Engine::registry &reg, GameWorld &gameWorld);
    virtual void DestroyScene(Engine::registry &reg);

 protected:
    Scene_A() = default;
    virtual ~Scene_A() = default;

    Engine::entity CreateEntityInScene(Engine::registry &reg);
    std::vector<Engine::entity> scene_entities_;
};
}  // namespace Rtype::Client
