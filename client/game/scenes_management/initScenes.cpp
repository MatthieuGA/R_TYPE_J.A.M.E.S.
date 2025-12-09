#include "client/Game/scenes_management/initScenes.hpp"

#include <memory>

#include "game/scenes_management/scenes/GameScene.hpp"
#include "game/scenes_management/scenes/MainMenuScene.hpp"
#include "include/components/ScenesComponents.hpp"

namespace Rtype::Client {
void InitSceneLevel(registry &reg) {
    auto game_level_entity = reg.SpawnEntity();
    reg.AddComponent<Component::SceneManagement>(game_level_entity,
        Component::SceneManagement{"", "MainMenuScene",  // initial state
            {{"GameLevel", std::make_shared<GameScene>()},
                {"MainMenuScene", std::make_shared<MainMenuScene>()}}});
}

}  // namespace Rtype::Client
