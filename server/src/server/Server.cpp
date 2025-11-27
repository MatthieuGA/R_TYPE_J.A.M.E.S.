#include "server/Server.hpp"
#include "server/Components.hpp"
#include <iostream>

namespace server {

Server::Server(Config &config, boost::asio::io_context &io_context)
    : config_(config),
      io_context_(io_context),
      network_(std::make_unique<Network>(config, io_context)),
      registry_(),
      tick_timer_(io_context),
      running_(false) {
}

Server::~Server() {
    running_ = false;
}

void Server::initialize() {
    std::cout << "Initializing server..." << std::endl;
    registerComponents();
    registerSystems();
    std::cout << "Server initialized successfully" << std::endl;
}

void Server::registerComponents() {
    registry_.register_component<Component::Position>();
    registry_.register_component<Component::Velocity>();
    registry_.register_component<Component::Health>();
    registry_.register_component<Component::NetworkId>();
    registry_.register_component<Component::Player>();
    registry_.register_component<Component::Enemy>();

    std::cout << "Registered all components" << std::endl;
}

void Server::registerSystems() {
    registry_.add_system<Engine::sparse_array<Component::Position>,
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

void Server::start() {
    std::cout << "Starting server..." << std::endl;
    running_ = true;
    setupGameTick();
}

void Server::setupGameTick() {
    if (!running_) {
        return;
    }

    tick_timer_.expires_after(std::chrono::milliseconds(TICK_RATE_MS));
    tick_timer_.async_wait([this](const boost::system::error_code &ec) {
        if (!ec && running_) {
            update();
            setupGameTick();
        }
    });
}

void Server::update() {
    registry_.run_systems();

    // TODO: Process network messages from the SPSC queue
    // TODO: Send state updates to clients
}

Engine::registry &Server::getRegistry() {
    return registry_;
}

}  // namespace server
