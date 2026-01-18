/**
 * @file GraphicsBackendFactory.hpp
 * @brief Factory for creating graphics backend implementations.
 *
 * This factory provides a registry-based mechanism for creating and selecting
 * graphics backend implementations at runtime. It decouples backend creation
 * from the main application code, enabling flexible backend swapping and
 * facilitating testing with mock backends.
 *
 * SCOPE: PR 1.9 - Backend ownership and lifecycle management.
 */

#ifndef CLIENT_GRAPHICS_GRAPHICSBACKENDFACTORY_HPP_
#define CLIENT_GRAPHICS_GRAPHICSBACKENDFACTORY_HPP_

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include "graphics/IRenderContext.hpp"

// Forward declaration to avoid circular includes
namespace sf {
class RenderWindow;
}

namespace Rtype::Client::Graphics {

/**
 * @brief Factory for creating graphics backend implementations.
 *
 * Provides a static registry where backend creators are registered.
 * Enables runtime backend selection by name without main.cpp coupling.
 *
 * Usage:
 *   // Register backends (typically in main.cpp at startup)
 *   GraphicsBackendFactory::Register("sfml",
 *       [](sf::RenderWindow &window) {
 *           return std::make_unique<SFMLRenderContext>(window);
 *       });
 *
 *   // Create a backend by name
 *   auto backend = GraphicsBackendFactory::Create("sfml", window);
 */
class GraphicsBackendFactory {
 public:
    /// Signature for backend creator functions
    using BackendCreator =
        std::function<std::unique_ptr<Engine::Graphics::IRenderContext>(
            sf::RenderWindow &)>;

    /**
     * @brief Register a backend creator with the factory.
     *
     * @param name Backend identifier (e.g., "sfml", "sdl", "test")
     * @param creator Function that creates the backend from an
     * sf::RenderWindow
     */
    static void Register(const std::string &name, BackendCreator creator);

    /**
     * @brief Create a backend instance by registered name.
     *
     * @param name Backend identifier
     * @param window SFML render window to pass to creator
     * @return Unique pointer to created backend, or nullptr if name not found
     */
    static std::unique_ptr<Engine::Graphics::IRenderContext> Create(
        const std::string &name, sf::RenderWindow &window);

    /**
     * @brief Check if a backend is registered.
     *
     * @param name Backend identifier
     * @return True if registered, false otherwise
     */
    static bool IsRegistered(const std::string &name);

 private:
    // Static registry of backend creators
    static std::unordered_map<std::string, BackendCreator> &GetRegistry();
};

}  // namespace Rtype::Client::Graphics

#endif  // CLIENT_GRAPHICS_GRAPHICSBACKENDFACTORY_HPP_
