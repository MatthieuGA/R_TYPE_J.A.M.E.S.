#include "game/scenes_management/scenes/GameScene.hpp"

#include <iostream>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "engine/audio/AudioManager.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"
#include "include/PlayerConst.hpp"
#include "include/components/CoreComponents.hpp"
#include "include/components/GameplayComponents.hpp"
#include "include/components/RenderComponent.hpp"
#include "include/registry.hpp"

namespace Rtype::Client {
void GameScene::AddBackgroundEntity(
    Engine::registry &reg, BackgroundInfo info, float initial_x) {
    auto background_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        background_entity, Component::Transform{initial_x, info.initialY, 0.0f,
                               info.scale, Component::Transform::TOP_LEFT});
    if (info.isWave) {
        reg.AddComponent<Component::Shader>(background_entity,
            Component::Shader{"wave.frag",
                {{"speed", info.wave_speed}, {"amplitude", info.amplitude},
                    {"frequency", info.frequency}}});
    }
    reg.AddComponent<Component::Drawable>(background_entity,
        Component::Drawable{info.path, info.z_index, info.opacity});
    reg.AddComponent<Component::ParrallaxLayer>(
        background_entity, Component::ParrallaxLayer{info.scroll_speed});
}

void GameScene::InitBackgrounds(Engine::registry &reg) {
    std::vector<BackgroundInfo> background_list = {
        {"background/level_1/1.png", -5.f, 0.f, LAYER_BACKGROUND - 3, true,
            1.f, .0005f, 0.2f},
        {"background/level_1/2.png", -15.f, 0.f, LAYER_BACKGROUND - 2, true,
            6.f, .007f, 1.2f},
        {"background/level_1/3.png", -25.f, 0.f, LAYER_BACKGROUND - 1, true,
            6.f, .007f, 1.2f},
        {"background/level_1/4.png", -35.f, 0.f, LAYER_BACKGROUND, true, 4.f,
            .005f, 1.5f},
        {"background/level_1/5.png", -150.f, -20.f, LAYER_FOREGROUND, false,
            0.f, 0.f, 0.f, 0.8f},
        {"background/level_1/5.png", -130.f, -200.f, LAYER_FOREGROUND + 1,
            false, 0.f, 0.f, 0.f, 0.6f}};

    // Initialize background entity
    for (const auto &background : background_list) {
        for (int i = 0; i < 2; i++)
            AddBackgroundEntity(reg, background, i * 1920.0f);
    }
}

void GameScene::InitPlayerLevel(Engine::registry &reg) {
    auto player_entity = CreateEntityInScene(reg);
    FactoryActors::GetInstance().CreateActor(player_entity, reg, "player");

    try {
        Component::Transform &transform =
            reg.GetComponent<Component::Transform>(player_entity);
        transform.x = -100.0f;
        transform.y = 300.0f;
    } catch (const std::exception &e) {
        return;
    }

    auto player_charging_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        player_charging_entity, Component::Transform(100.0f, 0.0f, 0.0f, 1.0f,
                                    Component::Transform::CENTER, {0.0f, 0.0f},
                                    player_entity.GetId()));
    reg.AddComponent<Component::Drawable>(player_charging_entity,
        Component::Drawable{
            "original_rtype/r-typesheet1.gif", LAYER_ACTORS - 1, 0.0f});

    Component::AnimatedSprite charging_anim(
        33, 33, 0.1f, true, Engine::Graphics::Vector2f(0.0f, 50.0f), 8);

    std::cout << "\n=== CHARGING ENTITY CREATION ===" << std::endl;
    std::cout << "AnimatedSprite settings:" << std::endl;
    std::cout << "  - animated flag: " << charging_anim.animated << std::endl;
    std::cout << "  - currentAnimation: " << charging_anim.currentAnimation
              << std::endl;

    auto *default_anim = charging_anim.GetCurrentAnimation();
    if (default_anim) {
        std::cout << "Default animation details:" << std::endl;
        std::cout << "  - path: '" << default_anim->path << "'" << std::endl;
        std::cout << "  - texture_id: '" << default_anim->texture_id << "'"
                  << std::endl;
        std::cout << "  - isLoaded: " << default_anim->isLoaded << std::endl;
        std::cout << "  - frameWidth: " << default_anim->frameWidth
                  << std::endl;
        std::cout << "  - frameHeight: " << default_anim->frameHeight
                  << std::endl;
        std::cout << "  - totalFrames: " << default_anim->totalFrames
                  << std::endl;
        std::cout << "  - frameDuration: " << default_anim->frameDuration
                  << std::endl;
        std::cout << "  - loop: " << default_anim->loop << std::endl;
        std::cout << "  - first_frame_position: ("
                  << default_anim->first_frame_position.x << ", "
                  << default_anim->first_frame_position.y << ")" << std::endl;
    }
    std::cout << "===================================\n" << std::endl;

    reg.AddComponent<Component::AnimatedSprite>(
        player_charging_entity, std::move(charging_anim));

    // Add the charging entity to the player's children list
    try {
        reg.GetComponent<Component::Transform>(player_entity)
            .children.push_back(player_charging_entity.GetId());
    } catch (const std::exception &e) {
        return;
    }
}

void GameScene::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    // Register and play game music
    if (gameWorld.audio_manager_) {
        gameWorld.audio_manager_->RegisterAsset(
            "main_theme", "assets/sounds/main_theme.ogg", true);
        gameWorld.audio_manager_->PlayMusic("main_theme", true);
    }

    InitBackgrounds(reg);
    InitPlayerLevel(reg);
    std::random_device rd;
    std::mt19937 gen(rd());
    for (int i = 0; i < 4; i++) {
        std::vector<Engine::Graphics::Vector2f> enemy_positions = {
            {1400.f, 700.f}, {1100.f, 700.f}, {1100.f, 400.f},
            {1400.f, 400.f}};
        AddEnemyLevel(reg, enemy_positions[i], i);
    }
}

void GameScene::AddEnemyLevel(
    Engine::registry &reg, Engine::Graphics::Vector2f position, int i) {
    auto enemy_entity = CreateEntityInScene(reg);
    FactoryActors::GetInstance().CreateActor(enemy_entity, reg, "mermaid");
    try {
        Component::Transform &transform =
            reg.GetComponent<Component::Transform>(enemy_entity);
        transform.x = position.x;
        transform.y = position.y;
    } catch (const std::exception &e) {
        return;
    }
}
}  // namespace Rtype::Client
