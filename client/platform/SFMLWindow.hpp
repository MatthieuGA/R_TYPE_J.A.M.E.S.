/**
 * @file SFMLWindow.hpp
 * @brief SFML implementation of the IWindow interface.
 *
 * This class owns and manages the sf::RenderWindow instance.
 * It is the ONLY place where sf::RenderWindow should be directly used.
 */

#ifndef CLIENT_PLATFORM_SFMLWINDOW_HPP_
#define CLIENT_PLATFORM_SFMLWINDOW_HPP_

#include <string>

#include <SFML/Graphics/RenderWindow.hpp>

#include "graphics/IWindow.hpp"

namespace Rtype::Client::Platform {

/**
 * @brief SFML implementation of the window interface.
 *
 * Wraps sf::RenderWindow to provide backend-agnostic window management.
 * This is the ONLY class allowed to own sf::RenderWindow.
 */
class SFMLWindow : public Engine::Graphics::IWindow {
 public:
    /**
     * @brief Construct an SFML window.
     *
     * @param width Window width in pixels
     * @param height Window height in pixels
     * @param title Window title
     */
    SFMLWindow(
        unsigned int width, unsigned int height, const std::string &title);

    ~SFMLWindow() override = default;

    // IWindow interface implementation
    bool IsOpen() const override;
    void Close() override;
    Engine::Graphics::Vector2i GetSize() const override;
    void SetTitle(const std::string &title) override;
    void Display() override;
    bool HasFocus() const override;
    Engine::Graphics::Vector2f MapPixelToCoords(
        const Engine::Graphics::Vector2i &pixel) const override;

    /**
     * @brief Get access to the underlying SFML window.
     *
     * This is needed by SFML-specific backends (event source, graphics
     * backend). Use sparingly - prefer the IWindow interface when possible.
     *
     * @return Reference to the internal sf::RenderWindow
     */
    sf::RenderWindow &GetNativeWindow();

 private:
    sf::RenderWindow window_;
};

}  // namespace Rtype::Client::Platform

#endif  // CLIENT_PLATFORM_SFMLWINDOW_HPP_
