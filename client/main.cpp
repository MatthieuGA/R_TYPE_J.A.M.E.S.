#include <iostream>
#include <cstdio>
#include <SFML/Graphics.hpp>

#include "include/registry.hpp"
#include "include/indexed_zipper.hpp"
#include "include/Component.hpp"

using Rtype::Client::Component::controllable;
using Rtype::Client::Component::drawable;
using Rtype::Client::Component::position;
using Rtype::Client::Component::velocity;

namespace Rtype::Client {
void positionSystem(Engine::registry &reg,
Engine::sparse_array<Component::position> const &positions,
Engine::sparse_array<Component::velocity> const &velocities) {
    for (auto &&[i, pos, vel] :
        Engine::make_indexed_zipper(positions, velocities)) {
        //  std::cerr << "Entity " << i << " Position: ("
        //     << pos.x << ", " << pos.y << ") "
        //     << "Velocity: (" << vel.vx << ", " << vel.vy << ")\n";
    }
}

void controllableSystem(Engine::registry &reg,
Engine::sparse_array<Component::controllable> const &controls) {
    for (auto &&[i, control] : Engine::make_indexed_zipper(controls)) {
        if (control.isControllable) {
            //  std::cerr << "Entity " << i << " is controllable.\n";
        }
    }
}

void drawableSystem(Engine::registry &reg, sf::RenderWindow &window,
Engine::sparse_array<Component::position> const &positions,
Engine::sparse_array<Component::drawable> const &drawables) {
    for (auto &&[i, pos, drawable] :
    Engine::make_indexed_zipper(positions, drawables)) {
        std::cerr << "Drawing entity " << i
            << " at position (" << pos.x << ", " << pos.y << ") "
            << "with sprite: " << drawable.sprite << " scaled by "
            << drawable.scale << "\n";
        sf::RectangleShape shape(sf::Vector2f(50.0f * drawable.scale,
            50.0f * drawable.scale));
        shape.setPosition(sf::Vector2f(pos.x, pos.y));
        shape.setFillColor(sf::Color::Green);
        window.draw(shape);
    }
}

}  // namespace Rtype::Client

void init_registry(Engine::registry &reg, sf::RenderWindow &window) {
    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<drawable>();
    reg.register_component<controllable>();

    Engine::registry::entity_t player = reg.spawn_entity();
    reg.add_component<position>
        (player, position{100.0f, 100.0f});
    reg.add_component<velocity>
        (player, velocity{0.f, 1.f});
    reg.add_component<drawable>
        (player, drawable{"hero.png", 1.0f});
    reg.add_component<controllable>
        (player, controllable{true});

    Engine::registry::entity_t enemy1 = reg.spawn_entity();
    reg.add_component<position>
        (enemy1, position{400.0f, 100.0f});
    reg.add_component<drawable>
        (enemy1, drawable{"enemy.png", 0.5f});
    Engine::registry::entity_t enemy2 = reg.spawn_entity();
    reg.add_component<position>
        (enemy2, position{400.0f, 200.0f});
    reg.add_component<drawable>
        (enemy2, drawable{"enemy.png", 0.5f});

    reg.add_system<Engine::sparse_array<controllable>>
        (Rtype::Client::controllableSystem);
    reg.add_system<Engine::sparse_array<position>,
        Engine::sparse_array<velocity>>(Rtype::Client::positionSystem);
    reg.add_system<Engine::sparse_array<position>,
        Engine::sparse_array<drawable>>(
        [&window](Engine::registry &r,
                   Engine::sparse_array<position> const &positions,
                   Engine::sparse_array<drawable> const &drawables) {
            Rtype::Client::drawableSystem(r, window, positions, drawables);
        });
}

int main() {
    Engine::registry reg;
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML");
    init_registry(reg, window);

    while (window.isOpen()) {
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        window.clear(sf::Color::Black);
        reg.run_systems();
        window.display();
    }

    return 0;
}
