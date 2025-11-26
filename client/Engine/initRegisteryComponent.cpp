#include "Engine/initRegisteryComponent.hpp"

namespace Rtype::Client {
void init_registry_components(Engine::registry &reg) {
    // Register core components
    reg.register_component<Rtype::Client::Component::Transform>();
    reg.register_component<Rtype::Client::Component::Drawable>();
    reg.register_component<Rtype::Client::Component::RigidBody>();
    reg.register_component<Rtype::Client::Component::Controllable>();
    reg.register_component<Rtype::Client::Component::InputState>();
    reg.register_component<Rtype::Client::Component::HitBox>();
    // Register gameplay components
    reg.register_component<Rtype::Client::Component::PlayerTag>();
    reg.register_component<Rtype::Client::Component::EnemyTag>();
    reg.register_component<Rtype::Client::Component::Projectile>();
    reg.register_component<Rtype::Client::Component::Health>();
    reg.register_component<Rtype::Client::Component::StatsGame>();
    // Register networking components
    reg.register_component<Rtype::Client::Component::NetworkId>();
    reg.register_component<Rtype::Client::Component::InterpolatedPosition>();
}
}  // namespace Rtype::Client
