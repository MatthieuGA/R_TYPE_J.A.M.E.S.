#include "server/Server.hpp"

#include <iostream>
#include <utility>
#include <vector>

namespace server {

Server::Server(Config &config, boost::asio::io_context &io_context)
    : config_(config),
      io_context_(io_context),
      network_(config, io_context),
      registry_(),
      tick_timer_(io_context),
      running_(false),
      connection_manager_(config.GetMaxPlayers()),
      packet_sender_(connection_manager_, network_),
      packet_handler_(connection_manager_, packet_sender_, network_) {
    // Set game start callback
    packet_handler_.SetGameStartCallback([this]() { Start(); });
}

Server::~Server() {
    running_ = false;
}

void Server::Initialize() {
    std::cout << "Initializing server..." << std::endl;
    RegisterComponents();
    RegisterSystems();

    // Register packet handlers
    packet_handler_.RegisterHandlers();

    // Register TCP accept callback
    network_.GetTcp().SetAcceptCallback(
        [this](boost::asio::ip::tcp::socket socket) {
            HandleTcpAccept(std::move(socket));
        });

    // Register UDP receive callback to update client endpoints
    network_.SetUdpReceiveCallback(
        [this](const boost::asio::ip::udp::endpoint &endpoint,
            const std::vector<uint8_t> &data) {
            // Update the client's UDP endpoint when we receive from them
            ClientConnection *client =
                connection_manager_.FindClientByIp(endpoint.address());
            if (client) {
                connection_manager_.UpdateClientUdpEndpoint(
                    client->client_id_, endpoint);
            }
        });

    std::cout << "Server initialized successfully" << std::endl;
}

void Server::Start() {
    if (running_) {
        return;
    }
    std::cout << "Starting game..." << std::endl;
    running_ = true;

    SetupEntityiesGame();
    SetupGameTick();
}

void Server::Stop() {
    std::cout << "Stopping game..." << std::endl;
    running_ = false;
    tick_timer_.cancel();
    std::cout << "Game stopped" << std::endl;
}

void Server::Close() {
    std::cout << "Closing server..." << std::endl;
    Stop();
    // Client sockets will be closed automatically when connection_manager_ is
    // destroyed (when the Server object is destroyed)
    std::cout << "Server closed" << std::endl;
}

void Server::SetupGameTick() {
    if (!running_) {
        return;
    }

    tick_timer_.expires_after(std::chrono::milliseconds(TICK_RATE_MS));
    tick_timer_.async_wait([this](const boost::system::error_code &ec) {
        if (!ec && running_) {
            Update();
            SetupGameTick();
        }
    });
}

void Server::Update() {
    registry_.run_systems();

    SendSnapshotsToAllClients();
    // TODO(someone): Process network messages from the SPSC queue
    // TODO(someone): Send state updates to clients
}

Engine::registry &Server::GetRegistry() {
    return registry_;
}

void Server::HandleTcpAccept(boost::asio::ip::tcp::socket socket) {
    // Add client to connection manager
    uint32_t client_id = connection_manager_.AddClient(std::move(socket));

    // Start handling messages immediately
    packet_handler_.StartReceiving(client_id);
}

}  // namespace server
