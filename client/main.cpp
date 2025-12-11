#include <iostream>

#include <SFML/Graphics.hpp>

#include "engine/GameWorld.hpp"
#include "game/InitRegistry.hpp"
#include "game/factory/factory_ennemies/FactoryActors.hpp"
#include "game/scenes_management/InitScenes.hpp"

namespace RC = Rtype::Client;

int main() {
    try {
        RC::GameWorld game_world;

        RC::FactoryActors::GetInstance().InitializeEnemyInfoMap("assets/data");
        RC::InitRegistry(game_world);
        RC::InitSceneLevel(game_world.registry_);
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
