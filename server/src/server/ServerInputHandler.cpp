#include <iostream>
#include <utility>
#include <vector>

#include "server/CoreComponents.hpp"
#include "server/NetworkComponents.hpp"
#include "server/Server.hpp"

namespace server {

bool Server::HandleUdpPlayerInput(
    const boost::asio::ip::udp::endpoint &endpoint,
    const std::vector<uint8_t> &data) {
    // data.size() already validated by caller; payload at offset 12
    uint8_t payload = data[12];

    // If non-zero, this MAY be a discovery packet (client telling us its
    // UDP port). Treat as discovery only when the client's stored UDP port
    // still equals the server UDP port (i.e., not yet updated). Otherwise
    // treat the payload as a normal input bitfield.
    uint8_t input_flags = payload;  // default: treat as input
    if (payload != 0) {
        uint8_t player_id = payload;
        ClientConnection *conn =
            connection_manager_.FindClientByPlayerId(player_id);
        if (conn) {
            // If stored port equals server's UDP port, this is discovery
            if (conn->udp_endpoint_.port() == network_.GetUdpPort() &&
                endpoint.port() != network_.GetUdpPort()) {
                connection_manager_.UpdateClientUdpEndpoint(
                    conn->client_id_, endpoint);
                return true;
            }
            // else fallthrough and treat payload as input_flags
            input_flags = payload;
        }
    }

    // Find client by matching udp endpoint
    ClientConnection *client_match = nullptr;
    for (const auto &p : connection_manager_.GetClients()) {
        const auto &c = p.second;
        try {
            if (c.udp_endpoint_.address() == endpoint.address() &&
                c.udp_endpoint_.port() == endpoint.port()) {
                client_match = const_cast<ClientConnection *>(&p.second);
                break;
            }
        } catch (...) {
            continue;
        }
    }

    if (!client_match) {
        client_match = connection_manager_.FindClientByIp(endpoint.address());
    }

    if (!client_match || client_match->player_id_ == 0) {
        return false;  // not handled
    }

    uint8_t pid = client_match->player_id_;

    // Find entity index with matching NetworkId
    std::optional<size_t> entity_index;
    auto &network_ids = registry_.GetComponents<Component::NetworkId>();
    size_t idx = 0;
    for (const auto &net_id : network_ids) {
        if (net_id->id == static_cast<int>(pid)) {
            entity_index = idx;
            break;
        }
        ++idx;
    }

    if (!entity_index.has_value()) {
        return false;
    }

    size_t eidx = entity_index.value();

    // Ensure Inputs exists
    auto &inputs = registry_.GetComponents<Component::Inputs>();
    if (!inputs.has(eidx)) {
        registry_.AddComponent<Component::Inputs>(
            registry_.EntityFromIndex(eidx), Component::Inputs{});
    }

    auto &maybe_inp = inputs[eidx];
    if (!maybe_inp.has_value()) {
        return false;
    }

    bool up = (input_flags & (1 << 0)) != 0;
    bool down = (input_flags & (1 << 1)) != 0;
    bool left = (input_flags & (1 << 2)) != 0;
    bool right = (input_flags & (1 << 3)) != 0;
    bool shoot = (input_flags & (1 << 4)) != 0;

    float horizontal = 0.0f;
    float vertical = 0.0f;
    if (right)
        horizontal += 1.0f;
    if (left)
        horizontal -= 1.0f;
    if (down)
        vertical += 1.0f;
    if (up)
        vertical -= 1.0f;

    maybe_inp->horizontal = horizontal;
    maybe_inp->vertical = vertical;
    maybe_inp->shoot = shoot;

    return true;
}

}  // namespace server
