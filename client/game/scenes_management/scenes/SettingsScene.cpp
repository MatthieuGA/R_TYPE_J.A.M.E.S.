#include "game/scenes_management/scenes/SettingsScene.hpp"

#include <iostream>
#include <string>
#include <vector>

#include "engine/audio/AudioManager.hpp"
#include "game/GameAction.hpp"
#include "game/InputRebindHelper.hpp"
#include "include/ColorsConst.hpp"
#include "include/LayersConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/components/ScenesComponents.hpp"
#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"
#include "input/Key.hpp"

namespace Rtype::Client {

void SettingsScene::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    InitBackground(reg);
    InitUI(reg, gameWorld);
}

void SettingsScene::InitBackground(Engine::registry &reg) {
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

void SettingsScene::InitUI(Engine::registry &reg, GameWorld &gameWorld) {
    // --- Title ---
    auto title_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        title_entity, Component::Transform{960.0f, 150.0f, 0.0f, 3.f,
                          Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(title_entity,
        Component::Text("dogica.ttf", "Settings", 20, LAYER_UI + 2, WHITE_BLUE,
            Engine::Graphics::Vector2f(0.0f, 0.0f)));
    title_entity_ = title_entity;

    // --- Music Volume Label ---
    auto music_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        music_label_entity, Component::Transform{800.0f, 300.0f, 0.0f, 2.f,
                                Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(music_label_entity,
        Component::Text("dogica.ttf", "Music:", 14, LAYER_UI + 2, WHITE_BLUE,
            Engine::Graphics::Vector2f(0.0f, 0.0f)));

    // --- Music Volume Slider ---
    CreateSlider(
        reg, gameWorld, 1060.0f, 300.0f, 150.0f, 0.0f, 1.0f,
        gameWorld.audio_manager_ ? gameWorld.audio_manager_->GetMusicVolume()
                                 : 1.0f,
        [&gameWorld](float value) {
            if (gameWorld.audio_manager_) {
                gameWorld.audio_manager_->SetMusicVolume(value);
            }
        },
        3.0f);

    // --- SFX Volume Label ---
    auto sfx_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        sfx_label_entity, Component::Transform{800.0f, 400.0f, 0.0f, 2.f,
                              Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(sfx_label_entity,
        Component::Text("dogica.ttf", "SFX:", 14, LAYER_UI + 2, WHITE_BLUE,
            Engine::Graphics::Vector2f(0.0f, 0.0f)));

    // --- SFX Volume Slider ---
    CreateSlider(
        reg, gameWorld, 1060.0f, 400.0f, 150.0f, 0.0f, 1.0f,
        gameWorld.audio_manager_ ? gameWorld.audio_manager_->GetSfxVolume()
                                 : 1.0f,
        [&gameWorld](float value) {
            if (gameWorld.audio_manager_) {
                gameWorld.audio_manager_->SetSfxVolume(value);
            }
        },
        3.0f);

    // --- Game Speed Label ---
    auto speed_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        speed_label_entity, Component::Transform{800.0f, 500.0f, 0.0f, 2.f,
                                Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(speed_label_entity,
        Component::Text("dogica.ttf", "Game Speed:", 14, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));

    // --- Game Speed Slider ---
    CreateSlider(
        reg, gameWorld, 1060.0f, 500.0f, 150.0f, 0.25f, 2.0f,
        gameWorld.game_speed_,
        [&gameWorld](float value) {
            gameWorld.game_speed_ = value;
            // Send game speed to server
            if (gameWorld.server_connection_) {
                gameWorld.server_connection_->SendGameSpeed(value);
            }
        },
        3.0f);

    // --- Input Bindings Section ---
    // Title for input bindings
    auto input_title_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        input_title_entity, Component::Transform{960.0f, 570.0f, 0.0f, 2.5f,
                                Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(input_title_entity,
        Component::Text("dogica.ttf", "Input Bindings", 14, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));

    // Create rebind buttons for movement and shoot actions
    std::vector<std::pair<Game::Action, std::string>> rebind_actions = {
        {Game::Action::MoveUp, "Move Up"},
        {Game::Action::MoveDown, "Move Down"},
        {Game::Action::MoveLeft, "Move Left"},
        {Game::Action::MoveRight, "Move Right"},
        {Game::Action::Shoot, "Shoot"}};

    float rebind_y = 610.0f;
    for (const auto &[action, label] : rebind_actions) {
        // Action label
        auto label_entity = CreateEntityInScene(reg);
        reg.AddComponent<Component::Transform>(
            label_entity, Component::Transform{650.0f, rebind_y, 0.0f, 1.8f,
                              Component::Transform::RIGHT_CENTER});
        reg.AddComponent<Component::Text>(label_entity,
            Component::Text("dogica.ttf", label + ":", 12, LAYER_UI + 2,
                WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));

        // Rebind button
        CreateButton(
            reg, gameWorld, "Rebind", 800.0f, rebind_y,
            [&gameWorld, action]() {
                gameWorld.rebinding_action_ = action;
                gameWorld.waiting_for_rebind_key_ = true;

                std::cout << "[Settings] Waiting for key rebind for "
                          << Game::GetActionName(action) << std::endl;
            },
            2.0f);

        rebind_y += 40.0f;
    }

    // --- Back Button ---
    CreateButton(reg, gameWorld, "Back", 960.0f, 700.0f, [&gameWorld]() {
        // OnClick: Return to main menu
        std::cout << "[Client] Returning to main menu" << std::endl;
        gameWorld.registry_.GetComponents<Component::SceneManagement>()
            .begin()
            ->value()
            .next = "MainMenuScene";
    });
}

}  // namespace Rtype::Client
