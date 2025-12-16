#include <utility>

#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "include/ColorsConst.hpp"
#include "include/EnemiesConst.hpp"
#include "include/LayersConst.hpp"
#include "include/PlayerConst.hpp"

namespace Rtype::Client {

void FactoryActors::CreatePlayerActor(Engine::entity &entity,
    Engine::registry &reg, EnnemyInfo info, bool is_local) {
    // Add basic enemy components
    Component::AnimatedSprite animated_sprite(34, 18, 2);
    animated_sprite.AddAnimation("Hit", "original_rtype/players_hit.png", 34,
        18, 2, 0.25f, false, sf::Vector2f(0.0f, id_player_ * 18.0f));
    animated_sprite.AddAnimation("Death", "original_rtype/players_death.png",
        36, 35, 6, 0.05f, false, sf::Vector2f(0.0f, id_player_ * 18.0f),
        sf::Vector2f(0.0f, -10.0f));
    reg.AddComponent<Component::AnimatedSprite>(
        entity, std::move(animated_sprite));
    // Add Inputs component only for the local player entity
    if (is_local) {
        reg.AddComponent<Component::Inputs>(entity, Component::Inputs{});
    }
    reg.AddComponent<Component::PlayerTag>(entity,
        Component::PlayerTag{info.speed, Rtype::Client::PLAYER_SHOOT_COOLDOWN,
            Rtype::Client::PLAYER_CHARGE_TIME, false, id_player_});
    // reg.AddComponent<Component::AnimationEnterPlayer>(
    //     entity, Component::AnimationEnterPlayer{true});
    // Reactor particle emitter
    Component::ParticleEmitter emit_reactor(100, 200, sf::Color::Yellow,
        RED_HIT, sf::Vector2f(-15.f, 3.f), true, 0.25f, 40.f,
        sf::Vector2f(-1.f, -0.1f), 30.f, 5.f, 10.0f, 4.0f, 3.f, -1.0f,
        LAYER_ACTORS - 2);
    reg.AddComponent<Component::ParticleEmitter>(
        entity, std::move(emit_reactor));

    id_player_ += 1;
}
}  // namespace Rtype::Client
