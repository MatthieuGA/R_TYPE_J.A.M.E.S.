#include "Game/initScenes.hpp"
#include "include/Components/ScenesComponents.hpp"


namespace Rtype::Client {
void init_scene_level(registry &reg) {
    auto game_level_entity = reg.SpawnEntity();
    reg.AddComponent<Component::SceneManagement>(game_level_entity,
        Component::SceneManagement{
            "", "GameLevel", // initial state
            {
                {"GameLevel", { init_game_level, nullptr }}
            }
        }
    );
}

}  // namespace Rtype::Client