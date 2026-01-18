/**
 * @file SFMLWindow.cpp
 * @brief Implementation of SFMLWindow.
 */

#include "platform/SFMLWindow.hpp"

#include <iostream>
#include <string>

#include <SFML/Window/VideoMode.hpp>

namespace Rtype::Client::Platform {

SFMLWindow::SFMLWindow(
    unsigned int width, unsigned int height, const std::string &title)
    : window_(sf::VideoMode({width, height}), title),
      game_width_(width),
      game_height_(height) {
    // Set initial view
    ApplyGameView();
}

bool SFMLWindow::IsOpen() const {
    return window_.isOpen();
}

void SFMLWindow::Close() {
    window_.close();
}

void SFMLWindow::Recreate(unsigned int width, unsigned int height,
    const std::string &title, bool fullscreen, unsigned int aa_level) {
    // Close existing window
    window_.close();

    // Create settings with anti-aliasing
    sf::ContextSettings settings;
    settings.antialiasingLevel = aa_level;

    // Recreate window with new parameters
    sf::VideoMode mode({width, height});

    try {
        if (fullscreen) {
            // For fullscreen, SFML works best with the desktop resolution
            // Get the desktop video mode
            sf::VideoMode desktop_mode = sf::VideoMode::getDesktopMode();
            std::cout << "[SFMLWindow] Desktop resolution: "
                      << desktop_mode.width << "x" << desktop_mode.height
                      << std::endl;

            // Create window at desktop resolution in fullscreen
            window_.create(
                desktop_mode, title, sf::Style::Fullscreen, settings);

            // If successful, verify it opened
            if (!window_.isOpen()) {
                std::cerr
                    << "[SFMLWindow] Failed to create fullscreen window, "
                    << "falling back to windowed mode" << std::endl;
                window_.create(mode, title, sf::Style::Default, settings);
            }
        } else {
            window_.create(mode, title, sf::Style::Default, settings);
        }
    } catch (const std::exception &e) {
        std::cerr << "[SFMLWindow] Exception during window recreation: "
                  << e.what() << ", falling back to windowed mode"
                  << std::endl;
        // Close and recreate in windowed mode
        window_.close();
        window_.create(mode, title, sf::Style::Default, settings);
    } catch (...) {
        std::cerr
            << "[SFMLWindow] Unknown exception during window recreation, "
            << "falling back to windowed mode" << std::endl;
        // Close and recreate in windowed mode
        window_.close();
        window_.create(mode, title, sf::Style::Default, settings);
    }

    // Store the game resolution and apply the view
    if (window_.isOpen()) {
        game_width_ = width;
        game_height_ = height;
        ApplyGameView();

        // Debug output
        auto window_size = window_.getSize();
        auto view = window_.getView();
        std::cout << "[SFMLWindow] Window size: " << window_size.x << "x"
                  << window_size.y << std::endl;
        std::cout << "[SFMLWindow] Game resolution (view): "
                  << view.getSize().x << "x" << view.getSize().y << std::endl;
        std::cout << "[SFMLWindow] This creates automatic letterboxing/scaling"
                  << std::endl;
    }
}

Engine::Graphics::Vector2i SFMLWindow::GetSize() const {
    sf::Vector2u size = window_.getSize();
    return Engine::Graphics::Vector2i(
        static_cast<int>(size.x), static_cast<int>(size.y));
}

void SFMLWindow::SetTitle(const std::string &title) {
    window_.setTitle(title);
}

void SFMLWindow::Display() {
    // Ensure view is applied before display (in case something reset it)
    ApplyGameView();
    window_.display();
}

bool SFMLWindow::HasFocus() const {
    return window_.hasFocus();
}

Engine::Graphics::Vector2f SFMLWindow::MapPixelToCoords(
    const Engine::Graphics::Vector2i &pixel) const {
    sf::Vector2i sf_pixel(pixel.x, pixel.y);
    sf::Vector2f sf_coords = window_.mapPixelToCoords(sf_pixel);
    return Engine::Graphics::Vector2f(sf_coords.x, sf_coords.y);
}

sf::RenderWindow &SFMLWindow::GetNativeWindow() {
    return window_;
}

void SFMLWindow::ApplyGameView() {
    // Create view that shows the game resolution but scales to window size
    sf::View game_view(sf::FloatRect(
        0.0f, 0.0f, static_cast<float>(1920), static_cast<float>(1080)));

    // Set viewport to use entire window (default is (0,0,1,1) but let's be
    // explicit)
    game_view.setViewport(sf::FloatRect(0.0f, 0.0f, 1.0f, 1.0f));

    window_.setView(game_view);
}

}  // namespace Rtype::Client::Platform
