#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"

namespace server {

void Server::RegisterComponents() {
    registry_.RegisterComponent<Component::Transform>();
    registry_.RegisterComponent<Component::Velocity>();
    registry_.RegisterComponent<Component::Controllable>();
    registry_.RegisterComponent<Component::InputState>();
    registry_.RegisterComponent<Component::HitBox>();
    registry_.RegisterComponent<Component::Solid>();
    registry_.RegisterComponent<Component::Inputs>();
    registry_.RegisterComponent<Component::PlayerTag>();
    registry_.RegisterComponent<Component::AnimationEnterPlayer>();
    registry_.RegisterComponent<Component::EnemyTag>();
    registry_.RegisterComponent<Component::TimedEvents>();
    registry_.RegisterComponent<Component::FrameEvents>();
    registry_.RegisterComponent<Component::EnemyShootTag>();
    registry_.RegisterComponent<Component::Projectile>();
    registry_.RegisterComponent<Component::Health>();
    registry_.RegisterComponent<Component::StatsGame>();
    registry_.RegisterComponent<Component::ParrallaxLayer>();
    registry_.RegisterComponent<Component::AnimationDeath>();
    registry_.RegisterComponent<Component::NetworkId>();
    registry_.RegisterComponent<Component::PatternMovement>();
    registry_.RegisterComponent<Component::AnimatedSprite>();
    registry_.RegisterComponent<Component::ExplodeOnDeath>();

    std::cout << "Registered all components" << std::endl;
}

}  // namespace server
