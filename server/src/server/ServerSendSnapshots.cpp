#include <iostream>
#include <utility>

#include "include/indexed_zipper.hpp"
#include "include/registry.hpp"
#include "server/CoreComponents.hpp"
#include "server/GameplayComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"

namespace server {

void Server::SendSnapshotsToAllClients() {
    for (const auto &[i, entity_network] :
        make_indexed_zipper(registry_.GetComponents<Component::NetworkId>())) {
        auto &transform = registry_.GetComponent<Component::Transform>(
            registry_.EntityFromIndex(i));

        network::EntityState entity_state;
        entity_state.entity_id =
            network::EntityId{static_cast<uint32_t>(entity_network.id)};
        entity_state.entity_type = 0;
        entity_state.reserved = 0;
        // Normalize position to fit in 16 bits (0..65535 for x, 0..38864 for
        // y)
        entity_state.pos_x =
            static_cast<uint16_t>(std::clamp(transform.x, 0.0f, 65535.0f));
        entity_state.pos_y =
            static_cast<uint16_t>(std::clamp(transform.y, 0.0f, 38864.0f));
        try {
            auto &velocity = registry_.GetComponent<Component::Velocity>(
                registry_.EntityFromIndex(i));
            // Encode velocity with bias: [-32768, 32767] -> [0, 65535]
            int16_t vx_clamped = static_cast<int16_t>(
                std::clamp(velocity.vx, -32768.0f, 32767.0f));
            int16_t vy_clamped = static_cast<int16_t>(
                std::clamp(velocity.vy, -32768.0f, 32767.0f));
            entity_state.velocity_x = static_cast<uint16_t>(
                static_cast<int32_t>(vx_clamped) + 32768);
            entity_state.velocity_y = static_cast<uint16_t>(
                static_cast<int32_t>(vy_clamped) + 32768);
        } catch (const std::exception &e) {
            entity_state.velocity_x = 32768;  // 0 velocity encoded
            entity_state.velocity_y = 32768;
        }
        // Normalize angle to [0, 360) range
        float normalized_angle = transform.rotationDegrees;
        while (normalized_angle < 0.0f)
            normalized_angle += 360.0f;
        while (normalized_angle >= 360.0f)
            normalized_angle -= 360.0f;
        entity_state.angle = static_cast<uint16_t>(normalized_angle * 10.0f);

        packet_sender_.SendSnapshot(entity_state);
    }
}
}  // namespace server
