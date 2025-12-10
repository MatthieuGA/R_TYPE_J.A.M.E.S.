#include "game/scenes_management/scenes/GameScene.hpp"

#include <random>
#include <string>
#include <utility>
#include <vector>

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
    reg.AddComponent<Component::Transform>(player_entity,
        {-100.0f, 300.0f, 0.0f, 4.0f, Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(player_entity,
        Component::Drawable{"original_rtype/players.png", LAYER_PLAYER});
    Component::AnimatedSprite animated_sprite(34, 18, 2);
    animated_sprite.AddAnimation(
        "Hit", "original_rtype/players_hit.png", 34, 18, 2, 0.15f, false);
    animated_sprite.AddAnimation("Death", "original_rtype/players_death.png",
        36, 35, 6, 0.05f, false, sf::Vector2f(0.0f, 0.0f),
        sf::Vector2f(0.0f, -10.0f));
    reg.AddComponent<Component::AnimatedSprite>(
        player_entity, std::move(animated_sprite));
    reg.AddComponent<Component::Inputs>(player_entity, Component::Inputs{});
    reg.AddComponent<Component::Velocity>(
        player_entity, Component::Velocity{});
    reg.AddComponent<Component::PlayerTag>(
        player_entity, Component::PlayerTag{Rtype::Client::PLAYER_SPEED_MAX,
                           Rtype::Client::PLAYER_SHOOT_COOLDOWN,
                           Rtype::Client::PLAYER_CHARGE_TIME, false});
    reg.AddComponent<Component::Health>(player_entity, Component::Health(100));
    reg.AddComponent<Component::AnimationEnterPlayer>(
        player_entity, Component::AnimationEnterPlayer{true});
    reg.AddComponent<Component::HitBox>(
        player_entity, Component::HitBox{30.0f, 10.0f});
    // Reactor particle emitter
    Component::ParticleEmitter emit_reactor(100, 200, sf::Color::Yellow,
        RED_HIT, sf::Vector2f(-15.f, 3.f), true, 0.25f, 40.f,
        sf::Vector2f(-1.f, -0.1f), 30.f, 5.f, 10.0f, 4.0f, 3.f, -1.0f,
        LAYER_PLAYER - 2);
    reg.AddComponent<Component::ParticleEmitter>(
        player_entity, std::move(emit_reactor));

    auto player_charging_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        player_charging_entity, Component::Transform(100.0f, 0.0f, 0.0f, 1.0f,
                                    Component::Transform::CENTER, {0.0f, 0.0f},
                                    player_entity.GetId()));
    reg.AddComponent<Component::Drawable>(player_charging_entity,
        Component::Drawable{
            "original_rtype/r-typesheet1.gif", LAYER_PLAYER - 1, 0.0f});
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
    std::uniform_int_distribution<> dis_y(50, 1000);
    std::uniform_int_distribution<> dis_x(0, 1400);
    for (int i = 0; i < 5; i++) {
        const float randY = static_cast<float>(dis_y(gen));
        const float randX = 400 + static_cast<float>(dis_x(gen));
        AddEnemyLevel(reg, sf::Vector2f(randX, randY));
    }
}

void GameScene::AddEnemyLevel(Engine::registry &reg, sf::Vector2f position) {
    auto enemy_entity = CreateEntityInScene(reg);
    reg.AddComponent<Component::Transform>(
        enemy_entity, {position.x, position.y, 0.0f, {-3.0f, 3.0f},
                          Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(
        enemy_entity, Component::Drawable{"ennemies/4/Idle.png", LAYER_ENEMY});
    Component::AnimatedSprite animated_sprite(
        48, 48, 0.2f, true, sf::Vector2f(0.0f, 0.0f), 4);
    animated_sprite.AddAnimation(
        "Hit", "ennemies/4/Hurt.png", 48, 48, 2, 0.1f, false);
    animated_sprite.AddAnimation(
        "Death", "ennemies/4/Death.png", 48, 48, 6, 0.1f, false);
    animated_sprite.AddAnimation(
        "Attack", "ennemies/4/Attack.png", 48, 48, 6, 0.15f, false);
    animated_sprite.AddAnimation(
        "Moving", "ennemies/4/Walk.png", 48, 48, 6, 0.15f, true);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        enemy_entity, std::move(animated_sprite));
    reg.AddComponent<Component::Health>(
        enemy_entity, Component::Health(Rtype::Client::MERMAID_HEALTH));
    reg.AddComponent<Component::EnemyTag>(
        enemy_entity, Component::EnemyTag{Rtype::Client::MERMAID_SPEED});
    reg.AddComponent<Component::Velocity>(enemy_entity, Component::Velocity{});
    reg.AddComponent<Component::PatternMovement>(
        enemy_entity, Component::PatternMovement(Rtype::Client::MERMAID_SPEED,
                          30.f, sf::Vector2f(position.x, position.y)));

    Component::EnemyShootTag enemy_shoot_tag(
        Rtype::Client::MERMAID_PROJECTILE_SPEED,
        Rtype::Client::MERMAID_PROJECTILE_DAMAGE, sf::Vector2f(-3.0f, -15.0f));

    // Add frame event with custom action
    reg.AddComponent<Component::FrameEvents>(enemy_entity,
        Component::FrameEvents("Attack", 5, [this, &reg](int entity_id) {
            // Custom action executed at frame 5 of Attack animation
            try {
                auto &transform = reg.GetComponent<Component::Transform>(
                    reg.EntityFromIndex(entity_id));
                auto &enemy_shoot = reg.GetComponent<Component::EnemyShootTag>(
                    reg.EntityFromIndex(entity_id));

                sf::Vector2f shoot_direction = sf::Vector2f(-1.0f, 0.0f);
                CreateEnemyProjectile(
                    reg, shoot_direction, enemy_shoot, entity_id, transform);
            } catch (const std::exception &e) {
                return;
            }
        }));
    reg.AddComponent<Component::TimedEvents>(enemy_entity,
        Component::TimedEvents(
            [this, &reg](int entity_id) {
                // Default cooldown action: trigger Attack animation
                try {
                    auto &animSprite =
                        reg.GetComponent<Component::AnimatedSprite>(
                            reg.EntityFromIndex(entity_id));
                    auto &health = reg.GetComponent<Component::Health>(
                        reg.EntityFromIndex(entity_id));
                    if (health.currentHealth <= 0)
                        return;
                    animSprite.SetCurrentAnimation("Attack");
                } catch (const std::exception &e) {
                    return;
                }
            },
            MERMAID_SHOOT_COOLDOWN));
    reg.AddComponent<Component::EnemyShootTag>(
        enemy_entity, std::move(enemy_shoot_tag));
    reg.AddComponent<Component::HitBox>(
        enemy_entity, Component::HitBox{8.0f, 32.0f});
}

void GameScene::CreateEnemyProjectile(Engine::registry &reg,
    sf::Vector2f direction, Component::EnemyShootTag &enemy_shoot, int ownerId,
    Component::Transform const &transform) {
    auto projectile_entity = reg.SpawnEntity();
    // Add components to projectile entity
    reg.AddComponent<Component::Transform>(projectile_entity,
        Component::Transform{
            transform.x + (enemy_shoot.offset_shoot_position.x *
                              std::abs(transform.scale.x)),
            transform.y + (enemy_shoot.offset_shoot_position.y *
                              std::abs(transform.scale.y)),
            0.0f, 2.f, Component::Transform::CENTER});
    reg.AddComponent<Component::Drawable>(projectile_entity,
        Component::Drawable("ennemies/4/Projectile.png", LAYER_PROJECTILE));
    reg.AddComponent<Component::Projectile>(projectile_entity,
        Component::Projectile{enemy_shoot.damage_projectile, direction,
            enemy_shoot.speed_projectile, ownerId, true});
    reg.AddComponent<Component::HitBox>(
        projectile_entity, Component::HitBox{8.0f, 8.0f});
    reg.AddComponent<Component::Velocity>(
        projectile_entity, Component::Velocity{direction.x, direction.y});
    reg.AddComponent<Component::ParticleEmitter>(projectile_entity,
        Component::ParticleEmitter(50, 50, RED_HIT, RED_HIT,
            sf::Vector2f(0.f, 0.f), true, 0.3f, 4.f, sf::Vector2f(-1.f, 0.f),
            45.f, 0, 8, 3.0f, 2.0f, -1.0f, LAYER_PARTICLE));
}
}  // namespace Rtype::Client
