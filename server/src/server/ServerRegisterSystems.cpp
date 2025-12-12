#include <iostream>

#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/Server.hpp"

#include "include/indexed_zipper.tpp"

namespace server {

void Server::RegisterSystems() {
    // Convert Inputs -> Velocity using PlayerTag speed
    registry_.AddSystem<Engine::sparse_array<Component::Inputs>,
        Engine::sparse_array<Component::PlayerTag>,
        Engine::sparse_array<Component::Velocity>>(
        [](Engine::registry &reg,
            Engine::sparse_array<Component::Inputs> &inputs,
            Engine::sparse_array<Component::PlayerTag> &player_tags,
            Engine::sparse_array<Component::Velocity> &velocities) {
            // Iterate over inputs and apply to velocities using player speed
            for (auto &&[i, inp, pt] :
                make_indexed_zipper(inputs, player_tags)) {
                // Ensure velocity component exists for this entity index
                if (!velocities.has(i)) {
                    reg.AddComponent<Component::Velocity>(
                        reg.EntityFromIndex(i), Component::Velocity{});
                }

                auto &maybe_inp = inputs[i];
                auto &maybe_pt = player_tags[i];
                auto &maybe_vel = velocities[i];

                if (maybe_inp.has_value() && maybe_pt.has_value() &&
                    maybe_vel.has_value()) {
                    float speed = maybe_pt->speed_max;
                    maybe_vel->vx = maybe_inp->horizontal * speed;
                    maybe_vel->vy = maybe_inp->vertical * speed;
                }
            }
        });

    // Debug: print Inputs state each frame for troubleshooting
    registry_.AddSystem<Engine::sparse_array<Component::Inputs>>(
        [](Engine::registry &reg,
            Engine::sparse_array<Component::Inputs> &inputs) {
            for (std::size_t i = 0; i < inputs.size(); ++i) {
                const auto &maybe = inputs[i];
                if (maybe.has_value()) {
                    const auto &inp = *maybe;
                    std::cout << "[Inputs Debug] entity_index=" << i
                              << " horiz=" << inp.horizontal
                              << " vert=" << inp.vertical
                              << " shoot=" << (inp.shoot ? 1 : 0) << std::endl;
                }
            }
        });

    registry_.AddSystem<Engine::sparse_array<Component::Transform>,
        Engine::sparse_array<Component::Velocity>>(
        [](Engine::registry &reg,
            Engine::sparse_array<Component::Transform> &transforms,
            Engine::sparse_array<Component::Velocity> const &velocities) {
            for (auto &&[i, pos, vel] :
                make_indexed_zipper(transforms, velocities)) {
                auto &pos = transforms[i];
                auto &vel = velocities[i];
                pos->x += vel->vx * TICK_RATE_MS / 1000.0f;
                pos->y += vel->vy * TICK_RATE_MS / 1000.0f;
            }
        });

    std::cout << "Registered all systems" << std::endl;
}

}  // namespace server
