#include "game/scenes_management/scenes/GameScene.hpp"

#include <random>
#include <string>
#include <utility>
#include <vector>

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

    auto player_charging_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        player_charging_entity, Component::Transform(100.0f, 0.0f, 0.0f, 1.0f,
                                    Component::Transform::CENTER, {0.0f, 0.0f},
                                    player_entity.GetId()));
    reg.AddComponent<Component::Drawable>(player_charging_entity,
        Component::Drawable{
            "original_rtype/r-typesheet1.gif", LAYER_ACTORS - 1, 0.0f});
    reg.AddComponent<Component::AnimatedSprite>(
        player_charging_entity, Component::AnimatedSprite(33, 33, 0.1f, true,
                                    sf::Vector2f(0.0f, 50.0f), 8));

    // Add the charging entity to the player's children list
    try {
        reg.GetComponent<Component::Transform>(player_entity)
            .children.push_back(player_charging_entity.GetId());
    } catch (const std::exception &e) {
        return;
    }
}

void GameScene::InitScene(Engine::registry &reg, GameWorld &gameWorld) {
    InitBackgrounds(reg);
    InitPlayerLevel(reg);
    std::random_device rd;
    std::mt19937 gen(rd());
    for (int i = 0; i < 4; i++) {
        std::vector<sf::Vector2f> enemy_positions = {{1400.f, 700.f},
            {1100.f, 700.f}, {1100.f, 400.f}, {1400.f, 400.f}};
        AddEnemyLevel(reg, enemy_positions[i], i);
    }
}

void GameScene::AddEnemyLevel(
    Engine::registry &reg, sf::Vector2f position, int i) {
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
