/**
 * @file SFMLEventSource.hpp
 * @brief SFML implementation of IPlatformEventSource.
 *
 * Wraps SFML window event polling and translates sf::Event
 * to Engine::Platform::OSEvent, isolating SFML dependencies.
 */

#ifndef CLIENT_PLATFORM_SFMLEVENTSOURCE_HPP_
#define CLIENT_PLATFORM_SFMLEVENTSOURCE_HPP_

#include <SFML/Window.hpp>

#include "platform/IPlatformEventSource.hpp"

namespace Rtype::Client::Platform {

/**
 * @brief SFML-based implementation of platform event source.
 *
 * Polls events from an SFML window and translates them into
 * backend-agnostic Engine::Platform::OSEvent structures.
 *
 * This class confines all SFML event handling to the client layer,
 * keeping the engine core independent of SFML.
 */
class SFMLEventSource : public Engine::Platform::IPlatformEventSource {
 public:
    /**
     * @brief Construct an event source for an SFML window.
     *
     * @param window Reference to the SFML window to poll events from
     */
    explicit SFMLEventSource(sf::Window &window);

    /**
     * @brief Destructor.
     */
    ~SFMLEventSource() override = default;

    /**
     * @brief Poll the next OS event from the SFML window.
     *
     * Calls sf::Window::pollEvent() internally and translates
     * the SFML event to an OSEvent structure.
     *
     * @param out The OSEvent structure to populate
     * @return true if an event was available, false if no events pending
     */
    bool Poll(Engine::Platform::OSEvent &out) override;

 private:
    sf::Window &window_;  ///< Reference to the SFML window
};

}  // namespace Rtype::Client::Platform

#endif  // CLIENT_PLATFORM_SFMLEVENTSOURCE_HPP_
