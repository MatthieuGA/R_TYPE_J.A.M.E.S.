#include <cstdio>
#include <iostream>
#include <memory>
#include <utility>

#include <SFML/Graphics.hpp>

#include "registry.hpp"
#include "Engine/Audio/AudioManager.hpp"
#include "Engine/Audio/SFMLAudioBackend.hpp"
#include "Engine/gameWorld.hpp"
#include "Engine/initRegistryComponent.hpp"
#include "Engine/initRegistrySystems.hpp"

using Engine::registry;
namespace RC = Rtype::Client;
namespace Component = Rtype::Client::Component;
namespace Audio = Rtype::Client::Audio;

void init_registry(RC::GameWorld &game_world,
    Audio::AudioManager &audio_manager) {
    RC::InitRegistryComponents(game_world.registry_);
    RC::InitRegistrySystems(game_world, audio_manager);
}

int main() {
    try {
        RC::GameWorld game_world;

        // Initialize audio subsystem
        auto audio_backend = std::make_unique<Audio::SFMLAudioBackend>();
        Audio::AudioManager audio_manager(std::move(audio_backend));

        init_registry(game_world, audio_manager);
        
        // Create some entities (simplified from base branch example)
        for (int i = 0; i < 4; ++i) {
            auto entity = game_world.registry_.SpawnEntity();
            game_world.registry_.EmplaceComponent<Component::Transform>(entity,
                Component::Transform{(i + 1) * 150.0f, 100.0f, i * 10.f, 0.2f});
            game_world.registry_.EmplaceComponent<Component::Drawable>(entity,
                Component::Drawable("Logo.png", 0, Component::Drawable::CENTER));
        }

        while (game_world.window_.isOpen()) {
            sf::Event event;
            while (game_world.window_.pollEvent(event)) {
                if (event.type == sf::Event::Closed)
                    game_world.window_.close();
            }
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
