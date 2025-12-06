#include <algorithm>
#include <cmath>
#include <SFML/Graphics.hpp>
#include "Engine/Systems/initRegistrySystems.hpp"

namespace Rtype::Client {
void GameStateSystem(Eng::registry &reg,
Eng::sparse_array<Component::SceneManagement> &sceneManagements) {

    for (auto &&[i, gs] : make_indexed_zipper(sceneManagements)) {
        if (gs.next.empty() || gs.next == gs.current)
            continue;

        // onExit de l’ancien état
        if (gs.scenes.find(gs.current) != gs.scenes.end())
            gs.scenes[gs.current]->DestroyScene(reg);

        // onEnter du nouvel état
        if (gs.scenes.find(gs.next) != gs.scenes.end())
            gs.scenes[gs.next]->InitScene(reg);

        gs.current = gs.next;
        gs.next = "";
    }

}

}  // namespace Rtype::Client
