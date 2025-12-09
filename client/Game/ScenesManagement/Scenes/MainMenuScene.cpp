#include "Game/ScenesManagement/Scenes/MainMenuScene.hpp"

#include <string>
#include <vector>

#include "Engine/Audio/AudioManager.hpp"
#include "Game/ScenesManagement/initScenes.hpp"
#include "include/Components/CoreComponents.hpp"
#include "include/Components/GameplayComponents.hpp"
#include "include/Components/RenderComponent.hpp"
#include "include/Components/ScenesComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
void MainMenuScene::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    // Register and play menu music
    if (gameWorld.audio_manager_) {
        gameWorld.audio_manager_->RegisterAsset(
            "menu_music", "Assets/Sounds/menu_music.ogg", true);
        gameWorld.audio_manager_->PlayMusic("menu_music", true);
    }

    auto play_button_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        play_button_entity, Component::Transform{960.0f, 500.0f, 0.0f, 5.0f,
                                Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        play_button_entity, Component::Drawable{"UI/Button.png", 0, 1.0f});
    reg.AddComponent<Component::HitBox>(
        play_button_entity, Component::HitBox{64.0f * 5, 16.0f * 5, false});
    reg.AddComponent<Component::Clickable>(play_button_entity,
        Component::Clickable{
            [&reg, &gameWorld]() {
                // OnClick: Transition to GameLevel scene
                if (gameWorld.audio_manager_) {
                    gameWorld.audio_manager_->StopMusic();
                }
                for (auto &&[i, gs] : make_indexed_zipper(
                         reg.GetComponents<Component::SceneManagement>())) {
                    gs.next = "GameLevel";
                }
            },
        });
    reg.AddComponent<Component::Text>(
        play_button_entity, Component::Text("dogica.ttf", "Play", 10, 1,
                                sf::Color::Black, sf::Vector2f(0.0f, -5.0f)));

    auto quit_button_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        quit_button_entity, Component::Transform{960.0f, 700.0f, 0.0f, 5.0f,
                                Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        quit_button_entity, Component::Drawable{"UI/Button.png", 0, 1.0f});
    reg.AddComponent<Component::HitBox>(
        quit_button_entity, Component::HitBox{64.0f, 16.0f});
    reg.AddComponent<Component::Clickable>(
        quit_button_entity, Component::Clickable{
                                [&gameWorld]() { gameWorld.window_.close(); },
                            });
    reg.AddComponent<Component::Text>(
        quit_button_entity, Component::Text("dogica.ttf", "Quit", 10, 1,
                                sf::Color::Black, sf::Vector2f(0.0f, -5.0f)));
}

}  // namespace Rtype::Client
