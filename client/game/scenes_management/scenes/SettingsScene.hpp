#pragma once

#include <optional>
#include <string>

#include "game/scenes_management/Scene_A.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
using Engine::registry;

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

    // Entity references for dynamic updates
    std::optional<Engine::entity> title_entity_;
    std::optional<Engine::entity> volume_slider_entity_;
    std::optional<Engine::entity> back_button_entity_;
};
}  // namespace Rtype::Client
