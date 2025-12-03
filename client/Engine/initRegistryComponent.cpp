#include "Engine/initRegistryComponent.hpp"

namespace Rtype::Client {
void InitRegistryComponents(Engine::registry &reg) {
    // Register core components
    reg.RegisterComponent<Rtype::Client::Component::Transform>();
    reg.RegisterComponent<Rtype::Client::Component::Drawable>();
    reg.RegisterComponent<Rtype::Client::Component::Velocity>();
    reg.RegisterComponent<Rtype::Client::Component::Controllable>();
    reg.RegisterComponent<Rtype::Client::Component::InputState>();
    reg.RegisterComponent<Rtype::Client::Component::HitBox>();
    reg.RegisterComponent<Rtype::Client::Component::AnimatedSprite>();
    reg.RegisterComponent<Rtype::Client::Component::Solid>();
    // Register gameplay components
    reg.RegisterComponent<Rtype::Client::Component::PlayerTag>();
    reg.RegisterComponent<Rtype::Client::Component::EnemyTag>();
    reg.RegisterComponent<Rtype::Client::Component::Projectile>();
    reg.RegisterComponent<Rtype::Client::Component::Health>();
    reg.RegisterComponent<Rtype::Client::Component::StatsGame>();
    reg.RegisterComponent<Rtype::Client::Component::ParrallaxLayer>();
    // Register networking components
    reg.RegisterComponent<Rtype::Client::Component::NetworkId>();
    reg.RegisterComponent<Rtype::Client::Component::InterpolatedPosition>();
}
}  // namespace Rtype::Client
