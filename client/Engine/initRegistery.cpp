#include "Engine/initRegistery.hpp"

using Rtype::Client::Component::Transform;
using Rtype::Client::Component::Drawable;
using Rtype::Client::Component::RigidBody;
using Rtype::Client::Component::Controllable;

namespace Rtype::Client {
void init_registry_components(Engine::registry &reg) {
    reg.register_component<Transform>();
    reg.register_component<Drawable>();
    reg.register_component<RigidBody>();
    reg.register_component<Controllable>();
}
}  // namespace Rtype::Client