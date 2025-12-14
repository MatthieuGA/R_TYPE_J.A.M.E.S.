/**
 * Minimal SFML test - verify SFML rendering works
 */
#include <iostream>

#include <SFML/Graphics.hpp>

int main() {
    std::cout << "Creating SFML window..." << std::endl;
    sf::RenderWindow window(sf::VideoMode(800, 600), "SFML Test");

    std::cout << "Loading texture..." << std::endl;
    sf::Texture texture;
    if (!texture.loadFromFile("assets/images/ui/button.png")) {
        std::cerr << "Failed to load texture!" << std::endl;
        return 1;
    }
    std::cout << "Texture loaded: " << texture.getSize().x << "x"
              << texture.getSize().y << std::endl;

    sf::Sprite sprite;
    sprite.setTexture(texture);
    sprite.setPosition(100, 100);
    sprite.setScale(3, 3);

    std::cout << "Entering render loop..." << std::endl;
    int frame = 0;
    while (window.isOpen() && frame < 300) {  // 5 seconds at 60fps
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        if (frame % 60 == 0) {
            std::cout << "Frame " << frame << ": Drawing sprite..."
                      << std::endl;
        }

        window.clear(sf::Color(30, 30, 80));
        window.draw(sprite);
        window.display();

        frame++;
    }

    std::cout << "Test complete. Drew " << frame << " frames." << std::endl;
    return 0;
}
