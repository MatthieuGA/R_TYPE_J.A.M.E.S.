#include "game/scenes_management/scenes/SettingsScene.hpp"

#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "engine/audio/AudioManager.hpp"
#include "game/GameAction.hpp"
#include "game/InputRebindHelper.hpp"
#include "include/AccessibilitySettings.hpp"
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
    // Store GameWorld pointer for cleanup
    game_world_ = &gameWorld;

    // Reset internal state before re-initializing
    active_tab_ = SettingsTab::Inputs;
    inputs_tab_entities_.clear();
    accessibility_tab_entities_.clear();
    graphics_tab_entities_.clear();
    audio_tab_entities_.clear();
    entity_original_y_.clear();
    rebind_buttons_.clear();
    action_icon_entities_.clear();
    action_icon_y_.clear();
    title_entity_.reset();
    volume_slider_entity_.reset();
    back_button_entity_.reset();
    speed_slider_knob_.reset();

    // Set up callback for real-time key icon refresh during rebinding
    gameWorld.on_binding_added_ = [this, &reg, &gameWorld](
                                      Game::Action action) {
        RefreshKeyIcons(reg, gameWorld, action);
    };

    InitBackground(reg);
    InitUI(reg, gameWorld);
}

void SettingsScene::DestroyScene(Engine::registry &reg) {
    // Clear external callbacks to prevent them from referencing dead entities
    if (game_world_) {
        game_world_->on_external_game_speed_change_ = nullptr;
        game_world_->on_binding_added_ = nullptr;
    }

    // Clear all tab entity vectors
    inputs_tab_entities_.clear();
    accessibility_tab_entities_.clear();
    graphics_tab_entities_.clear();
    audio_tab_entities_.clear();
    entity_original_y_.clear();
    rebind_buttons_.clear();
    action_icon_entities_.clear();
    action_icon_y_.clear();

    // Reset optional entity references
    title_entity_.reset();
    volume_slider_entity_.reset();
    back_button_entity_.reset();
    speed_slider_knob_.reset();
    hc_toggle_btn_entity_.reset();
    rv_toggle_btn_entity_.reset();

    // Call base class to actually destroy the entities
    Scene_A::DestroyScene(reg);
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
        title_entity, Component::Transform{960.0f, 100.0f, 0.0f, 3.f,
                          Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(title_entity,
        Component::Text("dogica.ttf", "Settings", 20, LAYER_UI + 2, WHITE_BLUE,
            Engine::Graphics::Vector2f(0.0f, 0.0f)));
    title_entity_ = title_entity;

    // --- Tab Buttons ---
    InitTabButtons(reg, gameWorld);

    // --- Tab Content (hidden by default except active tab) ---
    InitInputsTab(reg, gameWorld);
    InitAccessibilityTab(reg, gameWorld);
    InitGraphicsTab(reg, gameWorld);
    InitAudioTab(reg, gameWorld);

    // Show only the active tab
    SwitchToTab(reg, active_tab_);

    // --- Back Button (always visible) ---
    CreateButton(reg, gameWorld, "Back", 960.0f, 950.0f, [&gameWorld]() {
        std::cout << "[Client] Returning to main menu" << std::endl;
        gameWorld.registry_.GetComponents<Component::SceneManagement>()
            .begin()
            ->value()
            .next = "MainMenuScene";
    });
}

void SettingsScene::InitTabButtons(
    Engine::registry &reg, GameWorld &gameWorld) {
    const float tab_y = 180.0f;
    const float tab_spacing = 280.0f;
    // Center 4 buttons: start_x = center - (3 * spacing / 2)
    const float tab_start_x = 960.0f - (1.5f * tab_spacing);

    // Inputs Tab Button
    CreateButton(
        reg, gameWorld, "Inputs", tab_start_x, tab_y,
        [this, &reg]() { SwitchToTab(reg, SettingsTab::Inputs); }, 2.0f);

    // Accessibility Tab Button
    CreateButton(
        reg, gameWorld, "Access.", tab_start_x + tab_spacing, tab_y,
        [this, &reg]() { SwitchToTab(reg, SettingsTab::Accessibility); },
        2.0f);

    // Graphics Tab Button
    CreateButton(
        reg, gameWorld, "Graphics", tab_start_x + tab_spacing * 2, tab_y,
        [this, &reg]() { SwitchToTab(reg, SettingsTab::Graphics); }, 2.0f);

    // Audio Tab Button
    CreateButton(
        reg, gameWorld, "Audio", tab_start_x + tab_spacing * 3, tab_y,
        [this, &reg]() { SwitchToTab(reg, SettingsTab::Audio); }, 2.0f);
}

void SettingsScene::InitInputsTab(
    Engine::registry &reg, GameWorld &gameWorld) {
    const float content_start_y = 280.0f;
    const float row_spacing = 70.0f;  // More spacing between rows

    // --- Input Bindings Title ---
    auto input_title_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        input_title_entity, Component::Transform{960.0f, content_start_y, 0.0f,
                                2.5f, Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(input_title_entity,
        Component::Text("dogica.ttf", "Input Bindings", 14, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    inputs_tab_entities_.push_back(input_title_entity);

    // --- Rebind buttons for movement and shoot actions ---
    std::vector<std::pair<Game::Action, std::string>> rebind_actions = {
        {Game::Action::MoveUp, "Move Up"},
        {Game::Action::MoveDown, "Move Down"},
        {Game::Action::MoveLeft, "Move Left"},
        {Game::Action::MoveRight, "Move Right"},
        {Game::Action::Shoot, "Shoot"}};

    float rebind_y = content_start_y + 70.0f;
    for (const auto &[action, label] : rebind_actions) {
        // Action label
        auto label_entity = CreateEntityInScene(reg);
        reg.AddComponent<Component::Transform>(
            label_entity, Component::Transform{650.0f, rebind_y, 0.0f, 1.8f,
                              Component::Transform::RIGHT_CENTER});
        reg.AddComponent<Component::Text>(label_entity,
            Component::Text("dogica.ttf", label + ":", 12, LAYER_UI + 2,
                WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
        inputs_tab_entities_.push_back(label_entity);

        // Store Y position for this action (for RefreshKeyIcons)
        action_icon_y_[action] = rebind_y;

        // Display current key binding icon(s)
        if (gameWorld.input_manager_) {
            const auto &bindings =
                gameWorld.input_manager_->GetBindings(action);
            float icon_x = 700.0f;
            std::vector<Engine::entity> icon_entities;
            for (const auto &binding : bindings) {
                if (binding.type == Engine::Input::InputBinding::Type::Key) {
                    std::string asset_path =
                        Game::GetKeyAssetPath(binding.key);
                    if (!asset_path.empty()) {
                        auto icon_entity = CreateEntityInScene(reg);
                        reg.AddComponent<Component::Transform>(icon_entity,
                            Component::Transform{icon_x, rebind_y, 0.0f, 2.0f,
                                Component::Transform::CENTER});
                        reg.AddComponent<Component::Drawable>(icon_entity,
                            Component::Drawable{asset_path, LAYER_UI + 1});
                        inputs_tab_entities_.push_back(icon_entity);
                        icon_entities.push_back(icon_entity);
                        icon_x += 60.0f;  // Space between icons
                    }
                }
                // Only show first 3 bindings to avoid overflow
                if (icon_x > 880.0f)
                    break;
            }
            action_icon_entities_[action] = std::move(icon_entities);
        }

        // Rebind button (single entity returned)
        auto btn_entity = CreateButton(
            reg, gameWorld, "Rebind", 1050.0f, rebind_y,
            [this, &gameWorld, &reg, action]() {
                // If already rebinding another action, exit that first
                if (gameWorld.waiting_for_rebind_key_ &&
                    gameWorld.rebinding_action_.has_value()) {
                    ExitRebindMode(reg, gameWorld);
                }

                // Clear previous bindings for this action when starting rebind
                if (gameWorld.input_manager_) {
                    gameWorld.input_manager_->ClearBindings(action);
                }
                // Refresh icons (will show empty since we cleared bindings)
                RefreshKeyIcons(reg, gameWorld, action);

                gameWorld.rebinding_action_ = action;
                gameWorld.waiting_for_rebind_key_ = true;
                // Set the button entity from our map
                auto it = rebind_buttons_.find(action);
                if (it != rebind_buttons_.end()) {
                    gameWorld.rebinding_button_entity_ = it->second;
                    // Change button text to yellow
                    try {
                        auto &text =
                            reg.GetComponent<Component::Text>(it->second);
                        text.color = YELLOW_REBIND;
                    } catch (...) {}
                }
                std::cout << "[Settings] Waiting for key rebind for "
                          << Game::GetActionName(action)
                          << " (press keys to add, Escape to finish)"
                          << std::endl;
            },
            2.0f);
        // Store button entity in map for later reference
        rebind_buttons_.insert_or_assign(action, btn_entity);
        inputs_tab_entities_.push_back(btn_entity);

        rebind_y += row_spacing;
    }

    // --- Instructions ---
    auto instructions_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        instructions_entity, Component::Transform{960.0f, rebind_y + 40.0f,
                                 0.0f, 1.5f, Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(instructions_entity,
        Component::Text("dogica.ttf", "Click Rebind, then press a key", 10,
            LAYER_UI + 2, WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    inputs_tab_entities_.push_back(instructions_entity);

    // Store original Y positions for all inputs tab entities
    for (auto &entity : inputs_tab_entities_) {
        try {
            auto &transform = reg.GetComponent<Component::Transform>(entity);
            entity_original_y_[entity.GetId()] = transform.y;
        } catch (...) {}
    }
}

void SettingsScene::InitAccessibilityTab(
    Engine::registry &reg, GameWorld &gameWorld) {
    const float content_start_y = 280.0f;

    // --- Accessibility Title ---
    auto title_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        title_entity, Component::Transform{960.0f, content_start_y, 0.0f, 2.5f,
                          Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(title_entity,
        Component::Text("dogica.ttf", "Accessibility Options", 14,
            LAYER_UI + 2, WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    accessibility_tab_entities_.push_back(title_entity);

    // --- Game Speed Label ---
    auto speed_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(speed_label_entity,
        Component::Transform{800.0f, content_start_y + 80.0f, 0.0f, 2.f,
            Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(speed_label_entity,
        Component::Text("dogica.ttf", "Game Speed:", 14, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    accessibility_tab_entities_.push_back(speed_label_entity);

    // --- Game Speed Slider ---
    // Store slider parameters for external position updates
    speed_slider_x_ = 1060.0f;
    speed_slider_width_ = 150.0f;
    speed_slider_scale_ = 3.0f;
    speed_slider_min_ = 0.25f;
    speed_slider_max_ = 2.0f;

    // Note: CreateSlider creates track + knob entities internally
    // We track the current scene_entities_ size to capture both
    size_t entities_before = scene_entities_.size();
    auto knob_entity = CreateSlider(
        reg, gameWorld, speed_slider_x_, content_start_y + 80.0f,
        speed_slider_width_, speed_slider_min_, speed_slider_max_,
        gameWorld.game_speed_,
        [&gameWorld](float value) {
            std::cout << "[Settings] Game speed slider changed to: " << value
                      << std::endl;
            gameWorld.game_speed_ = value;
            if (gameWorld.server_connection_) {
                gameWorld.server_connection_->SendGameSpeed(value);
            } else {
                std::cout << "[Settings] No server connection, speed change "
                             "only local"
                          << std::endl;
            }
        },
        speed_slider_scale_);
    speed_slider_knob_ = knob_entity;

    // Add slider entities (track and knob) to tab
    for (size_t i = entities_before; i < scene_entities_.size(); ++i) {
        accessibility_tab_entities_.push_back(scene_entities_[i]);
    }

    // Register callback to update slider position when speed changes
    // externally
    gameWorld.on_external_game_speed_change_ = [this, &reg](float speed) {
        if (!speed_slider_knob_.has_value()) {
            return;
        }
        try {
            auto &transform =
                reg.GetComponent<Component::Transform>(*speed_slider_knob_);
            // Calculate new knob position from speed value
            float value_range = speed_slider_max_ - speed_slider_min_;
            float normalized_value = (speed - speed_slider_min_) / value_range;
            float knob_offset = (normalized_value - 0.5f) *
                                speed_slider_width_ * speed_slider_scale_;
            transform.x = speed_slider_x_ + knob_offset;
            std::cout << "[Settings] Speed slider updated externally to: "
                      << speed << " (x=" << transform.x << ")" << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "[Settings] Failed to update speed slider: "
                      << e.what() << std::endl;
        }
    };

    // --- High Contrast Toggle ---
    float toggle_y = content_start_y + 180.0f;

    auto hc_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        hc_label_entity, Component::Transform{700.0f, toggle_y, 0.0f, 1.8f,
                             Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(hc_label_entity,
        Component::Text("dogica.ttf", "High Contrast:", 12, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    accessibility_tab_entities_.push_back(hc_label_entity);

    auto hc_toggle_btn = CreateButton(
        reg, gameWorld,
        gameWorld.accessibility_settings_.high_contrast ? "ON" : "OFF",
        1050.0f, toggle_y,
        [this, &gameWorld, &reg]() {
            gameWorld.accessibility_settings_.high_contrast =
                !gameWorld.accessibility_settings_.high_contrast;
            std::cout << "[Settings] High contrast mode: "
                      << (gameWorld.accessibility_settings_.high_contrast
                                 ? "ON"
                                 : "OFF")
                      << std::endl;
            // Update button text
            if (hc_toggle_btn_entity_.has_value()) {
                try {
                    auto &text = reg.GetComponent<Component::Text>(
                        hc_toggle_btn_entity_.value());
                    text.content =
                        gameWorld.accessibility_settings_.high_contrast
                            ? "ON"
                            : "OFF";
                } catch (...) {}
            }
        },
        2.0f);
    hc_toggle_btn_entity_ = hc_toggle_btn;
    accessibility_tab_entities_.push_back(hc_toggle_btn);

    // --- Text Size Buttons ---
    toggle_y += 80.0f;

    auto ts_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        ts_label_entity, Component::Transform{700.0f, toggle_y, 0.0f, 1.8f,
                             Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(ts_label_entity,
        Component::Text("dogica.ttf", "Text Size:", 12, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    accessibility_tab_entities_.push_back(ts_label_entity);

    auto ts_small_btn = CreateButton(
        reg, gameWorld, "Small", 850.0f, toggle_y,
        [this, &gameWorld]() {
            gameWorld.accessibility_settings_.text_scale =
                TextSizeScale::Small;
            std::cout << "[Settings] Text size: Small (0.8x)" << std::endl;
        },
        1.8f);
    accessibility_tab_entities_.push_back(ts_small_btn);

    auto ts_normal_btn = CreateButton(
        reg, gameWorld, "Normal", 1100.0f, toggle_y,
        [this, &gameWorld]() {
            gameWorld.accessibility_settings_.text_scale =
                TextSizeScale::Normal;
            std::cout << "[Settings] Text size: Normal (1.0x)" << std::endl;
        },
        1.8f);
    accessibility_tab_entities_.push_back(ts_normal_btn);

    auto ts_large_btn = CreateButton(
        reg, gameWorld, "Large", 1350.0f, toggle_y,
        [this, &gameWorld]() {
            gameWorld.accessibility_settings_.text_scale =
                TextSizeScale::Large;
            std::cout << "[Settings] Text size: Large (1.2x)" << std::endl;
        },
        1.8f);
    accessibility_tab_entities_.push_back(ts_large_btn);

    // --- Reduced Visuals Toggle ---
    toggle_y += 80.0f;

    auto rv_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        rv_label_entity, Component::Transform{700.0f, toggle_y, 0.0f, 1.8f,
                             Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(rv_label_entity,
        Component::Text("dogica.ttf", "Reduced Visuals:", 12, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    accessibility_tab_entities_.push_back(rv_label_entity);

    auto rv_toggle_btn = CreateButton(
        reg, gameWorld,
        gameWorld.accessibility_settings_.reduced_visuals ? "ON" : "OFF",
        1050.0f, toggle_y,
        [this, &gameWorld, &reg]() {
            gameWorld.accessibility_settings_.reduced_visuals =
                !gameWorld.accessibility_settings_.reduced_visuals;
            std::cout << "[Settings] Reduced visuals mode: "
                      << (gameWorld.accessibility_settings_.reduced_visuals
                                 ? "ON"
                                 : "OFF")
                      << std::endl;
            // Update button text
            if (rv_toggle_btn_entity_.has_value()) {
                try {
                    auto &text = reg.GetComponent<Component::Text>(
                        rv_toggle_btn_entity_.value());
                    text.content =
                        gameWorld.accessibility_settings_.reduced_visuals
                            ? "ON"
                            : "OFF";
                } catch (...) {}
            }
        },
        2.0f);
    rv_toggle_btn_entity_ = rv_toggle_btn;
    accessibility_tab_entities_.push_back(rv_toggle_btn);

    // Store original Y positions for all accessibility tab entities
    for (auto &entity : accessibility_tab_entities_) {
        try {
            auto &transform = reg.GetComponent<Component::Transform>(entity);
            entity_original_y_[entity.GetId()] = transform.y;
        } catch (...) {}
    }
}

void SettingsScene::InitGraphicsTab(
    Engine::registry &reg, GameWorld &gameWorld) {
    const float content_start_y = 280.0f;
    float current_y = content_start_y;

    // --- Graphics Title ---
    auto title_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        title_entity, Component::Transform{960.0f, current_y, 0.0f, 2.5f,
                          Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(title_entity,
        Component::Text("dogica.ttf", "Graphics Settings", 14, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    graphics_tab_entities_.push_back(title_entity);

    current_y += 80.0f;

    // --- Resolution Dropdown ---
    auto res_label = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        res_label, Component::Transform{700.0f, current_y, 0.0f, 1.8f,
                       Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(res_label,
        Component::Text("dogica.ttf", "Resolution:", 12, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    graphics_tab_entities_.push_back(res_label);

    // Resolution buttons (common resolutions)
    std::vector<std::pair<std::string, std::pair<int, int>>> resolutions = {
        {"1280x720", {1280, 720}}, {"1600x900", {1600, 900}},
        {"1920x1080", {1920, 1080}}, {"2560x1440", {2560, 1440}}};

    float res_btn_x = 950.0f;
    for (const auto &[res_label_text, res_dims] : resolutions) {
        auto res_btn = CreateButton(
            reg, gameWorld, res_label_text, res_btn_x, current_y,
            [this, &gameWorld, res_dims]() {
                gameWorld.graphics_settings_.pending_resolution_width =
                    res_dims.first;
                gameWorld.graphics_settings_.pending_resolution_height =
                    res_dims.second;
                std::cout << "[Settings] Pending resolution change to "
                          << res_dims.first << "x" << res_dims.second
                          << std::endl;
            },
            1.5f);
        graphics_tab_entities_.push_back(res_btn);
        res_btn_x += 130.0f;
    }

    current_y += 80.0f;

    // --- Window Mode Dropdown ---
    auto wm_label = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        wm_label, Component::Transform{700.0f, current_y, 0.0f, 1.8f,
                      Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(wm_label,
        Component::Text("dogica.ttf", "Window Mode:", 12, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    graphics_tab_entities_.push_back(wm_label);

    std::vector<std::pair<std::string, WindowMode>> window_modes = {
        {"Windowed", WindowMode::Windowed},
        {"Fullscreen", WindowMode::Fullscreen},
        {"Borderless", WindowMode::Borderless}};

    float wm_btn_x = 950.0f;
    for (const auto &[wm_label_text, wm_mode] : window_modes) {
        auto wm_btn = CreateButton(
            reg, gameWorld, wm_label_text, wm_btn_x, current_y,
            [this, &gameWorld, wm_mode]() {
                gameWorld.graphics_settings_.pending_window_mode = wm_mode;
                std::cout << "[Settings] Pending window mode change to: "
                          << static_cast<int>(wm_mode) << std::endl;
            },
            1.5f);
        graphics_tab_entities_.push_back(wm_btn);
        wm_btn_x += 130.0f;
    }

    current_y += 80.0f;

    // --- VSync Toggle ---
    auto vsync_label = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        vsync_label, Component::Transform{700.0f, current_y, 0.0f, 1.8f,
                         Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(
        vsync_label, Component::Text("dogica.ttf", "VSync:", 12, LAYER_UI + 2,
                         WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    graphics_tab_entities_.push_back(vsync_label);

    static std::optional<Engine::entity> vsync_btn_entity;
    auto vsync_btn = CreateButton(
        reg, gameWorld,
        gameWorld.graphics_settings_.vsync_enabled ? "ON" : "OFF", 1050.0f,
        current_y,
        [this, &gameWorld, &reg]() {
            gameWorld.graphics_settings_.vsync_enabled =
                !gameWorld.graphics_settings_.vsync_enabled;
            std::cout << "[Settings] VSync: "
                      << (gameWorld.graphics_settings_.vsync_enabled ? "ON"
                                                                     : "OFF")
                      << std::endl;
            // Update button text
            if (vsync_btn_entity.has_value()) {
                try {
                    auto &text = reg.GetComponent<Component::Text>(
                        vsync_btn_entity.value());
                    text.content = gameWorld.graphics_settings_.vsync_enabled
                                       ? "ON"
                                       : "OFF";
                } catch (...) {}
            }
        },
        2.0f);
    vsync_btn_entity = vsync_btn;
    graphics_tab_entities_.push_back(vsync_btn);

    current_y += 80.0f;

    // --- Frame Rate Limit Dropdown ---
    auto fps_label = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        fps_label, Component::Transform{700.0f, current_y, 0.0f, 1.8f,
                       Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(fps_label,
        Component::Text("dogica.ttf", "Frame Rate Limit:", 12, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    graphics_tab_entities_.push_back(fps_label);

    std::vector<std::pair<std::string, unsigned int>> frame_rates = {
        {"30 FPS", 30}, {"60 FPS", 60}, {"120 FPS", 120}, {"Unlimited", 0}
        // 0 means no limit
    };

    float fps_btn_x = 950.0f;
    for (const auto &[fps_label_text, fps_limit] : frame_rates) {
        auto fps_btn = CreateButton(
            reg, gameWorld, fps_label_text, fps_btn_x, current_y,
            [this, &gameWorld, fps_limit]() {
                gameWorld.graphics_settings_.frame_rate_limit = fps_limit;
                std::cout << "[Settings] Frame rate limit set to: "
                          << (fps_limit == 0
                                     ? "Unlimited"
                                     : std::to_string(fps_limit) + " FPS")
                          << std::endl;
            },
            1.5f);
        graphics_tab_entities_.push_back(fps_btn);
        fps_btn_x += 130.0f;
    }

    current_y += 80.0f;

    // --- Anti-Aliasing Dropdown ---
    auto aa_label = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        aa_label, Component::Transform{700.0f, current_y, 0.0f, 1.8f,
                      Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(aa_label,
        Component::Text("dogica.ttf", "Anti-Aliasing:", 12, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    graphics_tab_entities_.push_back(aa_label);

    std::vector<std::pair<std::string, AntiAliasingLevel>> aa_levels = {
        {"Off", AntiAliasingLevel::Off}, {"2x MSAA", AntiAliasingLevel::AA2x},
        {"4x MSAA", AntiAliasingLevel::AA4x},
        {"8x MSAA", AntiAliasingLevel::AA8x}};

    float aa_btn_x = 950.0f;
    for (const auto &[aa_label_text, aa_level] : aa_levels) {
        auto aa_btn =
            CreateButton(
                reg, gameWorld, aa_label_text, aa_btn_x, current_y,
                [this, &gameWorld, aa_level]() {
                    gameWorld.graphics_settings_.pending_anti_aliasing =
                        aa_level;
                    std::cout
                        << "[Settings] Pending anti-aliasing level change to: "
                        << static_cast<int>(aa_level) << std::endl;
                },
                1.5f);
        graphics_tab_entities_.push_back(aa_btn);
        aa_btn_x += 130.0f;
    }

    current_y += 80.0f;

    // --- Warning text for pending settings ---
    auto warning_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        warning_entity, Component::Transform{960.0f, current_y, 0.0f, 1.5f,
                            Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(warning_entity,
        Component::Text("dogica.ttf",
            "Resolution/Window Mode/Anti-Aliasing require window restart", 10,
            LAYER_UI + 2, Engine::Graphics::Color(255, 200, 100),
            Engine::Graphics::Vector2f(0.0f, 0.0f)));
    graphics_tab_entities_.push_back(warning_entity);

    current_y += 60.0f;

    // --- Apply Button ---
    auto apply_btn = CreateButton(
        reg, gameWorld, "Apply Changes", 960.0f, current_y,
        [this, &gameWorld]() {
            if (gameWorld.graphics_settings_.HasPendingChanges()) {
                std::cout << "[Settings] Applying pending graphics changes..."
                          << std::endl;
                gameWorld.graphics_settings_.ApplyPendingSettings();
                std::cout << "[Settings] Pending changes applied" << std::endl;
            } else {
                std::cout << "[Settings] No pending graphics changes to apply"
                          << std::endl;
            }
        },
        2.0f);
    graphics_tab_entities_.push_back(apply_btn);

    // Store original Y positions for all graphics tab entities
    for (auto &entity : graphics_tab_entities_) {
        try {
            auto &transform = reg.GetComponent<Component::Transform>(entity);
            entity_original_y_[entity.GetId()] = transform.y;
        } catch (...) {}
    }
}

void SettingsScene::InitAudioTab(Engine::registry &reg, GameWorld &gameWorld) {
    const float content_start_y = 280.0f;

    // --- Audio Title ---
    auto title_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        title_entity, Component::Transform{960.0f, content_start_y, 0.0f, 2.5f,
                          Component::Transform::CENTER});
    reg.AddComponent<Component::Text>(title_entity,
        Component::Text("dogica.ttf", "Audio Settings", 14, LAYER_UI + 2,
            WHITE_BLUE, Engine::Graphics::Vector2f(0.0f, 0.0f)));
    audio_tab_entities_.push_back(title_entity);

    // --- Music Volume Label ---
    auto music_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(music_label_entity,
        Component::Transform{800.0f, content_start_y + 80.0f, 0.0f, 2.f,
            Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(music_label_entity,
        Component::Text("dogica.ttf", "Music:", 14, LAYER_UI + 2, WHITE_BLUE,
            Engine::Graphics::Vector2f(0.0f, 0.0f)));
    audio_tab_entities_.push_back(music_label_entity);

    // --- Music Volume Slider ---
    size_t entities_before_music = scene_entities_.size();
    CreateSlider(
        reg, gameWorld, 1060.0f, content_start_y + 80.0f, 150.0f, 0.0f, 1.0f,
        gameWorld.audio_manager_ ? gameWorld.audio_manager_->GetMusicVolume()
                                 : 1.0f,
        [&gameWorld](float value) {
            if (gameWorld.audio_manager_) {
                gameWorld.audio_manager_->SetMusicVolume(value);
            }
        },
        3.0f);
    for (size_t i = entities_before_music; i < scene_entities_.size(); ++i) {
        audio_tab_entities_.push_back(scene_entities_[i]);
    }

    // --- SFX Volume Label ---
    auto sfx_label_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(sfx_label_entity,
        Component::Transform{800.0f, content_start_y + 160.0f, 0.0f, 2.f,
            Component::Transform::RIGHT_CENTER});
    reg.AddComponent<Component::Text>(sfx_label_entity,
        Component::Text("dogica.ttf", "SFX:", 14, LAYER_UI + 2, WHITE_BLUE,
            Engine::Graphics::Vector2f(0.0f, 0.0f)));
    audio_tab_entities_.push_back(sfx_label_entity);

    // --- SFX Volume Slider ---
    size_t entities_before_sfx = scene_entities_.size();
    CreateSlider(
        reg, gameWorld, 1060.0f, content_start_y + 160.0f, 150.0f, 0.0f, 1.0f,
        gameWorld.audio_manager_ ? gameWorld.audio_manager_->GetSfxVolume()
                                 : 1.0f,
        [&gameWorld](float value) {
            if (gameWorld.audio_manager_) {
                gameWorld.audio_manager_->SetSfxVolume(value);
            }
        },
        3.0f);
    for (size_t i = entities_before_sfx; i < scene_entities_.size(); ++i) {
        audio_tab_entities_.push_back(scene_entities_[i]);
    }

    // Store original Y positions for all audio tab entities
    for (auto &entity : audio_tab_entities_) {
        try {
            auto &transform = reg.GetComponent<Component::Transform>(entity);
            entity_original_y_[entity.GetId()] = transform.y;
        } catch (...) {}
    }
}

void SettingsScene::SwitchToTab(Engine::registry &reg, SettingsTab tab) {
    active_tab_ = tab;

    constexpr float kOffScreenY = -9999.0f;

    // Helper to set visibility by moving entities off-screen or restoring
    auto set_visibility = [this, &reg, kOffScreenY](
                              std::vector<Engine::entity> &entities,
                              bool visible) {
        for (auto &entity : entities) {
            try {
                auto &transform =
                    reg.GetComponent<Component::Transform>(entity);
                size_t eid = entity.GetId();
                if (visible) {
                    // Restore original Y position from map
                    if (entity_original_y_.count(eid) > 0) {
                        transform.y = entity_original_y_[eid];
                    }
                } else {
                    // Move off-screen
                    transform.y = kOffScreenY;
                }
            } catch (...) {
                // Entity may not have transform, skip
            }
        }
    };

    // Hide all tabs
    set_visibility(inputs_tab_entities_, false);
    set_visibility(accessibility_tab_entities_, false);
    set_visibility(graphics_tab_entities_, false);
    set_visibility(audio_tab_entities_, false);

    // Show active tab
    switch (tab) {
        case SettingsTab::Inputs:
            set_visibility(inputs_tab_entities_, true);
            break;
        case SettingsTab::Accessibility:
            set_visibility(accessibility_tab_entities_, true);
            break;
        case SettingsTab::Graphics:
            set_visibility(graphics_tab_entities_, true);
            break;
        case SettingsTab::Audio:
            set_visibility(audio_tab_entities_, true);
            break;
    }

    std::cout << "[Settings] Switched to tab: " << static_cast<int>(tab)
              << std::endl;
}

void SettingsScene::ExitRebindMode(
    Engine::registry &reg, GameWorld &gameWorld) {
    if (!gameWorld.waiting_for_rebind_key_) {
        return;  // Not in rebind mode
    }

    // Reset button color to white
    if (gameWorld.rebinding_button_entity_.has_value()) {
        try {
            auto &text = reg.GetComponent<Component::Text>(
                gameWorld.rebinding_button_entity_.value());
            text.color = WHITE_BLUE;
        } catch (...) {}
    }

    // Refresh icons for the action that was being rebound
    if (gameWorld.rebinding_action_.has_value()) {
        RefreshKeyIcons(reg, gameWorld, gameWorld.rebinding_action_.value());
    }

    // Clear rebinding state
    gameWorld.rebinding_action_ = std::nullopt;
    gameWorld.waiting_for_rebind_key_ = false;
    gameWorld.rebinding_button_entity_ = std::nullopt;

    std::cout << "[Settings] Exited rebind mode" << std::endl;
}

void SettingsScene::RefreshKeyIcons(
    Engine::registry &reg, GameWorld &gameWorld, Game::Action action) {
    // Get Y position for this action
    auto y_it = action_icon_y_.find(action);
    if (y_it == action_icon_y_.end()) {
        return;  // Action not found
    }
    float rebind_y = y_it->second;

    // Remove old icon entities from inputs_tab_entities_ and destroy them
    auto icons_it = action_icon_entities_.find(action);
    if (icons_it != action_icon_entities_.end()) {
        for (Engine::entity old_icon : icons_it->second) {
            // Remove from inputs_tab_entities_
            auto it = std::find_if(inputs_tab_entities_.begin(),
                inputs_tab_entities_.end(),
                [&old_icon](const Engine::entity &e) {
                    return e.GetId() == old_icon.GetId();
                });
            if (it != inputs_tab_entities_.end()) {
                inputs_tab_entities_.erase(it);
            }
            // Destroy the entity
            try {
                reg.KillEntity(old_icon);
            } catch (...) {}
        }
        icons_it->second.clear();
    }

    // Create new icons based on current bindings
    if (!gameWorld.input_manager_) {
        return;
    }

    const auto &bindings = gameWorld.input_manager_->GetBindings(action);
    float icon_x = 700.0f;
    std::vector<Engine::entity> new_icons;

    for (const auto &binding : bindings) {
        if (binding.type == Engine::Input::InputBinding::Type::Key) {
            std::string asset_path = Game::GetKeyAssetPath(binding.key);
            if (!asset_path.empty()) {
                auto icon_entity = CreateEntityInScene(reg);
                reg.AddComponent<Component::Transform>(
                    icon_entity, Component::Transform{icon_x, rebind_y, 0.0f,
                                     2.0f, Component::Transform::CENTER});
                reg.AddComponent<Component::Drawable>(icon_entity,
                    Component::Drawable{asset_path, LAYER_UI + 1});
                inputs_tab_entities_.push_back(icon_entity);
                new_icons.push_back(icon_entity);
                icon_x += 60.0f;  // Space between icons
            }
        }
        // Only show first 3 bindings to avoid overflow
        if (icon_x > 880.0f)
            break;
    }

    action_icon_entities_[action] = std::move(new_icons);
}

}  // namespace Rtype::Client
