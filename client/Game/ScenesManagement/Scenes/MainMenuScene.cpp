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
void MainMenuScene::InitScene(Engine::registry &reg) {
    auto play_button_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(play_button_entity,
        Component::Transform{960.0f, 500.0f, 0.0f, 1.0f,
        Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(play_button_entity,
        Component::Drawable{"Logo.png", 0, 1.0f});
    reg.AddComponent<Component::HitBox>(play_button_entity,
        Component::HitBox{572.0f, 547.0f});
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
}

}  // namespace Rtype::Client