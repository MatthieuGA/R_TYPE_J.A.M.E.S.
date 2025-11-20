#include <iostream>
#include "registry.hpp"
#include "Composant.hpp"

void positionSystem(Engine::registry &reg,
Engine::sparse_array<Rtype::Client::Component::position> const &positions,
Engine::sparse_array<Rtype::Client::Component::velocity> const &velocities) {
    for (auto &&[i, pos, vel] : make_indexed_zipper(positions, velocities)) {
        std::cerr << "Entity " << i << " Position: (" << pos.x << ", " << pos.y << ") "
                  << "Velocity: (" << vel.vx << ", " << vel.vy << ")\n";
    }
}

void controllableSystem(Engine::registry &reg,
Engine::sparse_array<Rtype::Client::Component::controllable> const &controls) {
    for (auto &&[i, control] : make_indexed_zipper(controls)) {
        if (control.isControllable) {
            std::cerr << "Entity " << i << " is controllable.\n";
        }
    }
}

void drawableSystem(Engine::registry &reg,
Engine::sparse_array<Rtype::Client::Component::position> const &positions,
Engine::sparse_array<Rtype::Client::Component::drawable> const &drawables) {
    for (auto &&[i, pos, drawable] : make_indexed_zipper(positions, drawables)) {
        std::cerr << "Drawing entity " << i << " at position (" << pos.x << ", " << pos.y << ") "
                  << "with sprite: " << drawable.sprite << " scaled by " << drawable.scale << "\n";
    }
}

int main() {
    Engine::registry reg;
    printf("Registry created successfully.\n");
    printf("Registering Components...\n");
    reg.register_component<Rtype::Client::Component::position>();
    reg.register_component<Rtype::Client::Component::velocity>();
    reg.register_component<Rtype::Client::Component::drawable>();
    reg.register_component<Rtype::Client::Component::controllable>();
    printf("Components registered successfully.\n");

    Engine::registry::entity_t player = reg.spawn_entity();
    reg.add_component<Rtype::Client::Component::position>(player, Rtype::Client::Component::position{100.0f, 100.0f});
    reg.add_component<Rtype::Client::Component::velocity>(player, Rtype::Client::Component::velocity{0.f, 1.f});
    reg.add_component<Rtype::Client::Component::drawable>(player, Rtype::Client::Component::drawable{"hero.png", 1.0f});
    reg.add_component<Rtype::Client::Component::controllable>(player, Rtype::Client::Component::controllable{true});

    Engine::registry::entity_t enemy1 = reg.spawn_entity();
    reg.add_component<Rtype::Client::Component::position>(enemy1, Rtype::Client::Component::position{400.0f, 100.0f});
    reg.add_component<Rtype::Client::Component::drawable>(enemy1, Rtype::Client::Component::drawable{"enemy.png", 0.5f});
    Engine::registry::entity_t enemy2 = reg.spawn_entity();
    reg.add_component<Rtype::Client::Component::position>(enemy2, Rtype::Client::Component::position{400.0f, 200.0f});
    reg.add_component<Rtype::Client::Component::drawable>(enemy2, Rtype::Client::Component::drawable{"enemy.png", 0.5f});

    reg.add_system<Engine::sparse_array<Rtype::Client::Component::controllable>>(controllableSystem);
    reg.add_system<Engine::sparse_array<Rtype::Client::Component::position>, Engine::sparse_array<Rtype::Client::Component::velocity>>(positionSystem);
    reg.add_system<Engine::sparse_array<Rtype::Client::Component::position>, Engine::sparse_array<Rtype::Client::Component::drawable>>(drawableSystem);

    while (true) {
        reg.run_systems();
        break; // For demonstration, exit after one loop
    }
    return 0;
}