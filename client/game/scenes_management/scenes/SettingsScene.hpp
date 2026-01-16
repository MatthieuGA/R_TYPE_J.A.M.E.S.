#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

#include "game/scenes_management/Scene_A.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
using Engine::registry;

/**
 * @brief Enum representing the available settings tabs.
 */
enum class SettingsTab {
    Inputs,
    Accessibility,
    Graphics,
    Audio
};

/**
 * @brief Settings scene for configuring audio, video, and game options.
 */
class SettingsScene : public Scene_A {
 public:
    SettingsScene() = default;
    void InitScene(registry &reg, GameWorld &gameWorld) override;

    /**
     * @brief Get the entity holding the title text.
     * @return Optional entity if created, nullopt otherwise.
     */
    std::optional<Engine::entity> GetTitleEntity() const {
        return title_entity_;
    }

 private:
    struct BackgroundInfo {
        std::string path;
        float initialY;
        int z_index;
        bool isWave = false;
        float frequency = 20.0f;
        float amplitude = 0.002f;
        float wave_speed = 2.0f;
        float opacity = 1.0f;
        float scale = 3.34f;
    };

    void InitUI(registry &reg, GameWorld &gameWorld);
    void InitBackground(registry &reg);

    // Tab initialization methods
    void InitTabButtons(registry &reg, GameWorld &gameWorld);
    void InitInputsTab(registry &reg, GameWorld &gameWorld);
    void InitAccessibilityTab(registry &reg, GameWorld &gameWorld);
    void InitGraphicsTab(registry &reg, GameWorld &gameWorld);
    void InitAudioTab(registry &reg, GameWorld &gameWorld);

    // Tab switching
    void SwitchToTab(registry &reg, SettingsTab tab);

    // Entity references for dynamic updates
    std::optional<Engine::entity> title_entity_;
    std::optional<Engine::entity> volume_slider_entity_;
    std::optional<Engine::entity> back_button_entity_;
    std::optional<Engine::entity> speed_slider_knob_;  // For external sync

    // Speed slider parameters (for external position updates)
    float speed_slider_x_ = 0.0f;
    float speed_slider_width_ = 0.0f;
    float speed_slider_scale_ = 0.0f;
    float speed_slider_min_ = 0.25f;
    float speed_slider_max_ = 2.0f;

    // Tab state
    SettingsTab active_tab_ = SettingsTab::Inputs;
    std::vector<Engine::entity> inputs_tab_entities_;
    std::vector<Engine::entity> accessibility_tab_entities_;
    std::vector<Engine::entity> graphics_tab_entities_;
    std::vector<Engine::entity> audio_tab_entities_;

    // Store original Y positions for visibility toggling (entity id -> y)
    std::map<size_t, float> entity_original_y_;
};
}  // namespace Rtype::Client
