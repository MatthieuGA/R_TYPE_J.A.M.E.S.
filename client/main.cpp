#include <iostream>
#include <cstdio>
#include <SFML/Graphics.hpp>

#include "include/registry.hpp"
#include "include/indexed_zipper.hpp"
#include "Engine/initRegistery.hpp"

using Rtype::Client::Component::Controllable;
using Rtype::Client::Component::Drawable;
using Rtype::Client::Component::Transform;
using Rtype::Client::Component::RigidBody;

namespace Rtype::Client {
void positionSystem(Engine::registry &reg,
Engine::sparse_array<Component::Transform> const &positions,
Engine::sparse_array<Component::RigidBody> const &velocities) {
    for (auto &&[i, pos, vel] :
        Engine::make_indexed_zipper(positions, velocities)) {
        //  std::cerr << "Entity " << i << " Position: ("
        //     << pos.x << ", " << pos.y << ") "
        //     << "Velocity: (" << vel.vx << ", " << vel.vy << ")\n";
    }
}

void controllableSystem(Engine::registry &reg,
Engine::sparse_array<Component::Controllable> const &controls) {
    for (auto &&[i, control] : Engine::make_indexed_zipper(controls)) {
        if (control.isControllable) {
            //  std::cerr << "Entity " << i << " is controllable.\n";
        }
    }
}

void drawableSystem(Engine::registry &reg, sf::RenderWindow &window,
Engine::sparse_array<Component::Transform> const &transforms,
Engine::sparse_array<Component::Drawable> &drawables) {
    for (auto &&[i, tranform, drawable] :
    Engine::make_indexed_zipper(transforms, drawables)) {
        drawable.sprite.setPosition(sf::Vector2f(tranform.x, tranform.y));
        drawable.sprite.setScale(sf::Vector2f(tranform.scale, tranform.scale));
        drawable.sprite.setRotation(sf::radians(tranform.rotationDegrees * (180.0f / 3.14159265f)));
        window.draw(drawable.sprite);
    }
}

void init_registry(Engine::registry &reg, sf::RenderWindow &window) {
    init_registry_components(reg);
    // Set up systems
    reg.add_system<Engine::sparse_array<Controllable>>
        (Rtype::Client::controllableSystem);
    reg.add_system<Engine::sparse_array<Transform>,
        Engine::sparse_array<RigidBody>>(Rtype::Client::positionSystem);
    reg.add_system<Engine::sparse_array<Transform>,
        Engine::sparse_array<Drawable>>(
        [&window](Engine::registry &r,
                   Engine::sparse_array<Transform> const &positions,
                   Engine::sparse_array<Drawable> &drawables) {
            Rtype::Client::drawableSystem(r, window, positions, drawables);
        });
}
}  // namespace Rtype::Client

int main() {
    Engine::registry reg;
    sf::RenderWindow window(sf::VideoMode({800, 600}), "SFML");
    Rtype::Client::init_registry(reg, window);

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
