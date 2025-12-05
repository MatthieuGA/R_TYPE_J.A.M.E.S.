#include <iostream>

#include "Engine/initRegistrySystems.hpp"
#include "indexed_zipper.hpp"

namespace Eng = Engine;

namespace Rtype::Client {
namespace Com = Component;

void positionSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Transform> const &positions,
    Eng::sparse_array<Com::RigidBody> const &velocities) {
    for (auto &&[i, pos, vel] : make_indexed_zipper(positions, velocities)) {
        //  std::cerr << "Entity " << i << " Position: ("
        //     << pos.x << ", " << pos.y << ") "
        //     << "Velocity: (" << vel.vx << ", " << vel.vy << ")\n";
    }
}
}  // namespace Rtype::Client
