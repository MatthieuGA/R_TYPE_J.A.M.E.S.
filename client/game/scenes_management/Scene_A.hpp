#pragma once
#include <functional>
#include <string>
#include <vector>

#include "engine/GameWorld.hpp"
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

    /**
     * @brief Creates a button entity with standard UI components.
     *
     * @param reg Reference to the registry used for entity management.
     * @param label The text displayed on the button.
     * @param x The x-coordinate position of the button center.
     * @param y The y-coordinate position of the button center.
     * @param on_click Callback function invoked when the button is clicked.
     * @param scale The scale factor for the button. Default is 3.0f.
     * @return The newly created button entity.
     */
    Engine::entity CreateButton(Engine::registry &reg,
        const std::string &label, float x, float y,
        std::function<void()> on_click, float scale = 3.0f);

    std::vector<Engine::entity> scene_entities_;
};
}  // namespace Rtype::Client
