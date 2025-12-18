#include "game/scenes_management/scenes/MainMenuScene.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "engine/audio/AudioManager.hpp"
#include "include/ColorsConst.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {

void MainMenuScene::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    // Register and play menu music
    if (gameWorld.audio_manager_) {
        gameWorld.audio_manager_->RegisterAsset(
            "menu_music", "assets/sounds/menu_music.ogg", true);
        gameWorld.audio_manager_->PlayMusic("menu_music", true);
    }

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
    // --- Lobby Status Display ---
    // Player count text (e.g., "Players: 1/4")
    auto player_count_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        player_count_entity, Component::Transform{960.0f, 200.0f, 0.0f, 2.f,
                                 Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(player_count_entity,
        Component::Text("dogica.ttf", "Players: 0/4", 16, LAYER_UI + 2,
            WHITE_BLUE, sf::Vector2f(0.0f, 0.0f)));
    reg.AddComponent<Component::LobbyUI>(player_count_entity,
        Component::LobbyUI{Component::LobbyUI::Type::PlayerCount});
    player_count_entity_ = player_count_entity;

    // Ready count text (e.g., "Ready: 0/1")
    auto ready_count_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        ready_count_entity, Component::Transform{960.0f, 260.0f, 0.0f, 2.f,
                                Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(ready_count_entity,
        Component::Text("dogica.ttf", "Ready: 0/0", 16, LAYER_UI + 2,
            WHITE_BLUE, sf::Vector2f(0.0f, 0.0f)));
    reg.AddComponent<Component::LobbyUI>(ready_count_entity,
        Component::LobbyUI{Component::LobbyUI::Type::ReadyCount});
    ready_count_entity_ = ready_count_entity;

    // --- Ready Button ---
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
            [&gameWorld]() {
                // OnClick: Toggle READY_STATUS to server
                if (gameWorld.server_connection_ &&
                    gameWorld.server_connection_->is_connected()) {
                    // Toggle ready state
                    bool new_ready_state =
                        !gameWorld.server_connection_->is_local_player_ready();
                    std::cout << "[Client] Toggling ready status to: "
                              << (new_ready_state ? "Ready" : "Not Ready")
                              << std::endl;
                    gameWorld.server_connection_->SendReadyStatus(
                        new_ready_state);
                } else {
                    std::cerr << "[Client] Cannot send ready status: "
                              << "not connected to server" << std::endl;
                }
            },
        });
    reg.AddComponent<Component::Text>(play_button_entity,
        Component::Text("dogica.ttf", "Ready", 13, LAYER_UI + 1, WHITE_BLUE,
            sf::Vector2f(0.0f, -5.0f)));
    reg.AddComponent<Component::LobbyUI>(play_button_entity,
        Component::LobbyUI{Component::LobbyUI::Type::ReadyButton});
    ready_button_entity_ = play_button_entity;

    // --- Quit Button ---
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
