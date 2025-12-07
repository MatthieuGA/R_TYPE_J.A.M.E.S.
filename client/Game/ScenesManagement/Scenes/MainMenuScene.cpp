#include <string>
#include <vector>

#include "include/registry.hpp"
#include "include/indexed_zipper.hpp"
#include "Game/ScenesManagement/initScenes.hpp"
#include "include/Components/CoreComponents.hpp"
#include "include/Components/RenderComponent.hpp"
#include "include/Components/GameplayComponents.hpp"
#include "include/Components/ScenesComponents.hpp"
#include "Game/ScenesManagement/Scenes/MainMenuScene.hpp"

namespace Rtype::Client {
void MainMenuScene::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    auto play_button_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(play_button_entity,
        Component::Transform{960.0f, 500.0f, 0.0f, 5.0f,
        Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(play_button_entity,
        Component::Drawable{"UI/PlayButton.png", 0, 1.0f});
    reg.AddComponent<Component::HitBox>(play_button_entity,
        Component::HitBox{64.0f * 5, 16.0f * 5, false});
    reg.AddComponent<Component::Clickable>(play_button_entity,
        Component::Clickable{
            [&reg]() {
                // OnClick: Transition to GameLevel scene
                for (auto &&[i, gs] : make_indexed_zipper(reg.
                    GetComponents<Component::SceneManagement>())) {
                    gs.next = "GameLevel";
                }
            },
        });

    auto quit_button_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(quit_button_entity,
        Component::Transform{960.0f, 700.0f, 0.0f, 5.0f,
        Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(quit_button_entity,
        Component::Drawable{"UI/QuitButton.png", 0, 1.0f});
    reg.AddComponent<Component::HitBox>(quit_button_entity,
        Component::HitBox{64.0f, 16.0f});
    reg.AddComponent<Component::Clickable>(quit_button_entity,
        Component::Clickable{
            [&gameWorld]() {
                gameWorld.window_.close();
            },
        });
}

}  // namespace Rtype::Client