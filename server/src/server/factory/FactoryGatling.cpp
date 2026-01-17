#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/factory/FactoryActors.hpp"

#include "include/indexed_zipper.tpp"

namespace fs = std::filesystem;

namespace server {

void FactoryActors::CreateGatlingActor(
    Engine::entity &entity, Engine::registry &reg, EnnemyInfo info) {
    // Add pattern movement following the player
    reg.AddComponent<Component::PatternMovement>(
        entity, Component::PatternMovement(
                    Component::PatternMovement::PatternType::SineHorizontal,
                    vector2f(0.0f, 50.0f), vector2f(0.0f, 2.0f),
                    vector2f(-1.0f, 0.0f), info.speed));

    // Add enemy shooting component
    Component::EnemyShootTag enemy_shoot_tag(200.0f, 10.0f, {-3.0f, -15.0f});

    // Add drawable and animated sprite components
    // AnimatedSprite(bool loop, int totalFrames, float frameDuration)
    Component::AnimatedSprite animated_sprite(true, 1, 0.2f);
    animated_sprite.AddAnimation("Hit", 2, 0.1f, false);
    animated_sprite.AddAnimation("Death", 4, 0.05f, false);
    animated_sprite.AddAnimation("Attack", 2, 0.1f, false);
    animated_sprite.currentAnimation = "Default";
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));

    // For invincibility power-up, heal the player who picks it up.
    float radius = 200.0f;
    int damage = 0;
    int duration_gatling = 5;  // seconds
    Component::ExplodeOnDeath expl(radius, damage,
        [duration_gatling](Engine::registry &reg, int /*explod_ent_id*/,
            int target_player_id) {
            if (target_player_id < 0)
                return;
            auto &player_tags = reg.GetComponents<Component::PlayerTag>();
            // Only heal if the target has a PlayerTag
            if (!player_tags.has(static_cast<std::size_t>(target_player_id)))
                return;
            auto &player_tag =
                player_tags[static_cast<std::size_t>(target_player_id)];
            if (player_tag.has_value())
                player_tag->gatling_duration += duration_gatling;
        });
    reg.AddComponent<Component::ExplodeOnDeath>(entity, std::move(expl));
    reg.AddComponent<Component::PowerUp>(entity, Component::PowerUp{});
}

}  // namespace server
