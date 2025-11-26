#include "Engine/initRegisterySystems.hpp"
#include "include/indexed_zipper.hpp"
#include <iostream>

using namespace Engine;

namespace Rtype::Client {
    using namespace Component;
void positionSystem(registry &reg, sparse_array<Transform> const &positions,
sparse_array<RigidBody> const &velocities) {
    for (auto &&[i, pos, vel] :
        make_indexed_zipper(positions, velocities)) {
        //  std::cerr << "Entity " << i << " Position: ("
        //     << pos.x << ", " << pos.y << ") "
        //     << "Velocity: (" << vel.vx << ", " << vel.vy << ")\n";
    }
}
}  // namespace Rtype::Client
