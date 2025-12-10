#include "game/scenes_management/scenes/MainMenuScene.hpp"

#include <string>
#include <vector>

#include "include/ColorsConst.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {

void MainMenuScene::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    InitBackground(reg);
    InitUI(reg, gameWorld);
}

void MainMenuScene::InitBackground(Engine::registry &reg) {
    std::vector<BackgroundInfo> background_list = {
        {"background/level_1/1.png", 0.f, LAYER_BACKGROUND - 3, true, 1.f,
            .0005f, 0.2f},
        {"background/level_1/2.png", 0.f, LAYER_BACKGROUND - 2, true, 6.f,
            .007f, 1.2f},
        {"background/level_1/3.png", 0.f, LAYER_BACKGROUND - 1, true, 6.f,
            .007f, 1.2f},
        {"background/level_1/4.png", 0.f, LAYER_BACKGROUND, true, 4.f, .005f,
            1.5f},
        {"background/level_1/5.png", -20.f, LAYER_FOREGROUND, false, 0.f, 0.f,
            0.f, 0.8f},
        {"background/level_1/5.png", -200.f, LAYER_FOREGROUND + 1, false, 0.f,
            0.f, 0.f, 0.6f}};

    // Initialize background entity
    for (const auto &info : background_list) {
        auto background_entity = CreateEntityInScene(reg);
        reg.AddComponent<Component::Transform>(background_entity,
            Component::Transform{0, info.initialY, 0.0f, info.scale,
                Component::Transform::TOP_LEFT});
        if (info.isWave) {
            reg.AddComponent<Component::Shader>(background_entity,
                Component::Shader{"wave.frag",
                    {{"speed", info.wave_speed}, {"amplitude", info.amplitude},
                        {"frequency", info.frequency}}});
        }
        reg.AddComponent<Component::Drawable>(background_entity,
            Component::Drawable{info.path, info.z_index, info.opacity});
    }
}

void MainMenuScene::InitUI(Engine::registry &reg, GameWorld &gameWorld) {
    auto play_button_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        play_button_entity, Component::Transform{960.0f, 400.0f, 0.0f, 3.f,
                                Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(play_button_entity,
        Component::Drawable{"ui/button.png", LAYER_UI, 1.0f});
    reg.AddComponent<Component::HitBox>(
        play_button_entity, Component::HitBox{128.f, 32.f});
    reg.AddComponent<Component::Clickable>(play_button_entity,
        Component::Clickable{
            [&reg]() {
                // OnClick: Transition to GameLevel scene
                for (auto &&[i, gs] : make_indexed_zipper(
                         reg.GetComponents<Component::SceneManagement>())) {
                    gs.next = "GameLevel";
                }
            },
        });
    reg.AddComponent<Component::Text>(play_button_entity,
        Component::Text("dogica.ttf", "Play", 13, LAYER_UI + 1, WHITE_BLUE,
            sf::Vector2f(0.0f, -5.0f)));

    auto quit_button_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        quit_button_entity, Component::Transform{960.0f, 700.0f, 0.0f, 3.f,
                                Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(quit_button_entity,
        Component::Drawable{"ui/button.png", LAYER_UI, 1.0f});
    reg.AddComponent<Component::HitBox>(
        quit_button_entity, Component::HitBox{128.f, 32.f});
    reg.AddComponent<Component::Clickable>(
        quit_button_entity, Component::Clickable{
                                [&gameWorld]() { gameWorld.window_.close(); },
                            });
    reg.AddComponent<Component::Text>(quit_button_entity,
        Component::Text("dogica.ttf", "Quit", 13, LAYER_UI + 1, WHITE_BLUE,
            sf::Vector2f(0.0f, -5.0f)));
}

}  // namespace Rtype::Client
