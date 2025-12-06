#include "Game/ScenesManagement/initScenes.hpp"
#include "include/Components/ScenesComponents.hpp"

#include "Game/ScenesManagement/Scenes/GameScene.hpp"
#include "Game/ScenesManagement/Scenes/MainMenuScene.hpp"


namespace Rtype::Client {
void InitSceneLevel(registry &reg) {
    auto game_level_entity = reg.SpawnEntity();
    reg.AddComponent<Component::SceneManagement>(game_level_entity,
        Component::SceneManagement{
            "", "MainMenuScene", // initial state
            {
                {"GameLevel", std::make_shared<GameScene>()},
                {"MainMenuScene", std::make_shared<MainMenuScene>()}
            }
        }
    );
}

}  // namespace Rtype::Client