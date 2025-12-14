#include <iostream>

#include <SFML/Graphics.hpp>

int main() {
    // Create window with explicit context settings
    sf::ContextSettings settings;
    settings.depthBits = 24;
    settings.stencilBits = 8;
    settings.majorVersion = 3;
    settings.minorVersion = 0;

    sf::RenderWindow window(
        sf::VideoMode(800, 600), "SFML Test", sf::Style::Default, settings);
    window.setActive(true);

    std::cout << "Window created successfully" << std::endl;

    // Try to load a texture
    sf::Texture texture;
    if (texture.loadFromFile("assets/images/background/level_1/1.png")) {
        std::cout << "SUCCESS: Texture loaded! Size: " << texture.getSize().x
                  << "x" << texture.getSize().y << std::endl;
        std::cout << "Max texture size: " << sf::Texture::getMaximumSize()
                  << std::endl;
    } else {
        std::cerr << "FAILED: Could not load texture" << std::endl;
        std::cerr << "Max texture size: " << sf::Texture::getMaximumSize()
                  << std::endl;
    }

    return 0;
}
