#include "game/scenes_management/InitScenes.hpp"

#include <memory>

#include "game/scenes_management/scenes/GameScene.hpp"
#include "game/scenes_management/scenes/MainMenuScene.hpp"
#include "game/scenes_management/scenes/SettingsScene.hpp"
#include "include/components/ScenesComponents.hpp"

namespace Rtype::Client {
void InitSceneLevel(registry &reg) {
    auto game_level_entity = reg.SpawnEntity();
    reg.AddComponent<Component::SceneManagement>(game_level_entity,
        Component::SceneManagement{"", "MainMenuScene",  // initial state
            {{"GameLevel", std::make_shared<GameScene>()},
                {"MainMenuScene", std::make_shared<MainMenuScene>()},
                {"SettingsScene", std::make_shared<SettingsScene>()}}});
}

}  // namespace Rtype::Client
