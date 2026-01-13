#include <filesystem>
#include <iostream>
#include <string>
#include <utility>

#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"
#include "server/factory/FactoryActors.hpp"

namespace fs = std::filesystem;

namespace server {

void FactoryActors::CreateInvinsibilityActor(
    Engine::entity &entity, Engine::registry &reg) {
    reg.AddComponent<Component::Velocity>(
        entity, Component::Velocity{-50.0f, 0.0f, 0.0f, 0.0f});
    // Mark as an enemy so server will include it in snapshots sent to clients
    // Use subtype 4 to match client's kPowerUpInvinsibility
    reg.AddComponent<Component::EnemyTag>(
        entity, Component::EnemyTag{0.0f, static_cast<uint8_t>(4)});
    reg.AddComponent<Component::PowerUpTag>(entity,
        Component::PowerUpTag{Component::PowerUpTag::PowerUpType::Health});
}

}  // namespace server
