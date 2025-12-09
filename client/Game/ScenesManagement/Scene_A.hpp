#pragma once
#include <vector>

#include "Engine/gameWorld.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
class Scene_A {
 public:
    /**
     * @brief Initializes the scene by creating entities and setting up the
     * game world.
     *
     * @param reg Reference to the ECS registry used for entity/component
     * management.
     * @param gameWorld Reference to the game world context for scene setup.
     */
    virtual void InitScene(Engine::registry &reg, GameWorld &gameWorld);

    /**
     * @brief Cleans up the scene by destroying entities and releasing
     * resources.
     *
     * @param reg Reference to the ECS registry used for entity/component
     * management.
     */
    virtual void DestroyScene(Engine::registry &reg);

 protected:
    Scene_A() = default;
    virtual ~Scene_A() = default;

    /**
     * @brief Creates a new entity in the current scene and registers it.
     *
     * @param reg Reference to the registry used for entity management.
     * @return The newly created entity.
     */
    Engine::entity CreateEntityInScene(Engine::registry &reg);
    std::vector<Engine::entity> scene_entities_;
};
}  // namespace Rtype::Client
