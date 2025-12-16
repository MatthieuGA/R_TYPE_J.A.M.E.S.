#include <algorithm>
#include <cmath>

#include <SFML/Graphics.hpp>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Handles scene transitions by invoking exit and enter hooks.
 *
 * @param reg ECS registry used to access entities and systems.
 * @param sceneManagements Sparse array of scene management components tracking
 * current and next scenes.
 */
void GameStateSystem(Eng::registry &reg, GameWorld &gameWorld,
    Eng::sparse_array<Component::SceneManagement> &sceneManagements) {
    for (auto &&[i, gs] : make_indexed_zipper(sceneManagements)) {
        if (gs.next.empty() || gs.next == gs.current)
            continue;

        // Call onExit hook of the previous state if it exists
        if (gs.scenes.find(gs.current) != gs.scenes.end())
            gs.scenes[gs.current]->DestroyScene(reg);

        // Call onEnter hook of the new state if it exists
        if (gs.scenes.find(gs.next) != gs.scenes.end())
            gs.scenes[gs.next]->InitScene(reg, gameWorld);

        gs.current = gs.next;
        gs.next = "";
    }
}

}  // namespace Rtype::Client
