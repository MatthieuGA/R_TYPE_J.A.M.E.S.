#include <iostream>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

#include "engine/GameWorld.hpp"
#include "engine/audio/AudioManager.hpp"
#include "engine/audio/SFMLAudioBackend.hpp"
#include "game/InitRegistry.hpp"
#include "game/scenes_management/InitScenes.hpp"
#include "include/registry.hpp"

namespace RC = Rtype::Client;
namespace Audio = Rtype::Client::Audio;

int main() {
    try {
        RC::GameWorld game_world;

        // Initialize audio subsystem
        auto audio_backend = std::make_unique<Audio::SFMLAudioBackend>();
        Audio::AudioManager audio_manager(std::move(audio_backend));
        game_world.audio_manager_ = &audio_manager;

        RC::InitRegistry(game_world, audio_manager);
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
