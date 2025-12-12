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
        entity_state.angle = static_cast<uint16_t>(
            std::fmod(transform.rotationDegrees, 360.0f));

        packet_sender_.SendSnapshot(entity_state);
    }
}
}  // namespace server
