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
        if (gs.table.find(gs.current) != gs.table.end() && gs.table[gs.current].onExit)
            gs.table[gs.current].onExit(reg);

        // onEnter du nouvel état
        if (gs.table.find(gs.next) != gs.table.end() && gs.table[gs.next].onEnter)
            gs.table[gs.next].onEnter(reg);

        gs.current = gs.next;
        gs.next = "";
    }

}

}  // namespace Rtype::Client
