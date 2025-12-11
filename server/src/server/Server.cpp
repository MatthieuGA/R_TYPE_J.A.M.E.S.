#include "server/Server.hpp"

#include <iostream>
#include <utility>

#include "server/Components.hpp"

namespace server {

Server::Server(Config &config, boost::asio::io_context &io_context)
    : config_(config),
      io_context_(io_context),
      network_(config, io_context),
      registry_(),
      tick_timer_(io_context),
      running_(false),
      connection_manager_(config.GetMaxPlayers()),
      packet_sender_(connection_manager_),
      packet_handler_(connection_manager_, packet_sender_) {
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

    std::cout << "Server initialized successfully" << std::endl;
}

void Server::RegisterComponents() {
    registry_.RegisterComponent<Component::Position>();
    registry_.RegisterComponent<Component::Velocity>();
    registry_.RegisterComponent<Component::Health>();
    registry_.RegisterComponent<Component::NetworkId>();
    registry_.RegisterComponent<Component::Player>();
    registry_.RegisterComponent<Component::Enemy>();

    std::cout << "Registered all components" << std::endl;
}

void Server::RegisterSystems() {
    registry_.AddSystem<Engine::sparse_array<Component::Position>,
        Engine::sparse_array<Component::Velocity>>(
        [](Engine::registry &reg,
            Engine::sparse_array<Component::Position> &positions,
            Engine::sparse_array<Component::Velocity> &velocities) {
            for (size_t i = 0; i < positions.size() && i < velocities.size();
                ++i) {
                if (positions.has(i) && velocities.has(i)) {
                    auto &pos = positions[i];
                    auto &vel = velocities[i];
                    pos->x += vel->vx;
                    pos->y += vel->vy;
                }
            }
        });

    std::cout << "Registered all systems" << std::endl;
}

void Server::Start() {
    if (running_) {
        return;
    }
    std::cout << "Starting game..." << std::endl;
    running_ = true;
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
