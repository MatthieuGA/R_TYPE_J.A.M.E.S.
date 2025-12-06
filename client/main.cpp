#include <cstdio>
#include <iostream>
#include <vector>

#include <SFML/Graphics.hpp>

#include "Engine/gameWorld.hpp"
#include "Game/initRegistry.hpp"
#include "Game/initScenes.hpp"

using Engine::registry;
namespace RC = Rtype::Client;

int main() {
    try {
        RC::GameWorld game_world;

        RC::init_registry(game_world);
        RC::init_scene_level(game_world.registry_);
        while (game_world.window_.isOpen()) {
            sf::Event event;
            while (game_world.window_.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    game_world.window_.close();
            }
            // Calculate delta time at the beginning of the frame
            game_world.last_delta_ =
                game_world.delta_time_clock_.restart().asSeconds();

            game_world.window_.clear(sf::Color::Black);
            game_world.registry_.RunSystems();
            game_world.window_.display();
        }
        return 0;
    } catch (const std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}
