#include "engine/InitRegistryComponent.hpp"

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
    reg.RegisterComponent<Rtype::Client::Component::Shader>();
    reg.RegisterComponent<Rtype::Client::Component::Inputs>();
    reg.RegisterComponent<Rtype::Client::Component::Clickable>();
    reg.RegisterComponent<Rtype::Client::Component::Text>();
    // Register gameplay components
    reg.RegisterComponent<Rtype::Client::Component::PlayerTag>();
    reg.RegisterComponent<Rtype::Client::Component::EnemyTag>();
    reg.RegisterComponent<Rtype::Client::Component::EnemyShootTag>();
    reg.RegisterComponent<Rtype::Client::Component::Projectile>();
    reg.RegisterComponent<Rtype::Client::Component::Health>();
    reg.RegisterComponent<Rtype::Client::Component::StatsGame>();
    reg.RegisterComponent<Rtype::Client::Component::ParrallaxLayer>();
    reg.RegisterComponent<Rtype::Client::Component::AnimationEnterPlayer>();
    reg.RegisterComponent<Rtype::Client::Component::AnimationDeath>();
    // Register networking components
    reg.RegisterComponent<Rtype::Client::Component::NetworkId>();
    reg.RegisterComponent<Rtype::Client::Component::InterpolatedPosition>();
    // Register scenes components
    reg.RegisterComponent<Rtype::Client::Component::SceneManagement>();
}
}  // namespace Rtype::Client
