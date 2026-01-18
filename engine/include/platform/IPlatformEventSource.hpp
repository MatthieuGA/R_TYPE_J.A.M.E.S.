/**
 * @file IPlatformEventSource.hpp
 * @brief Interface for backend-agnostic OS event polling.
 *
 * Defines the contract for platform event sources (SFML, SDL, etc.)
 * to provide OS events to the engine without exposing backend details.
 */

#ifndef ENGINE_INCLUDE_PLATFORM_IPLATFORMEVENTSOURCE_HPP_
#define ENGINE_INCLUDE_PLATFORM_IPLATFORMEVENTSOURCE_HPP_

#include "platform/OSEvent.hpp"

namespace Engine {
namespace Platform {

/**
 * @brief Interface for polling platform/OS events.
 *
 * Concrete implementations wrap specific backends (SFML, SDL, etc.)
 * and translate their native events into Engine::Platform::OSEvent.
 *
 * This interface allows the engine to remain backend-agnostic while
 * supporting plugin-based event sources in the future.
 */
class IPlatformEventSource {
 public:
    /**
     * @brief Virtual destructor for proper cleanup.
     */
    virtual ~IPlatformEventSource() = default;

    /**
     * @brief Poll the next OS event from the platform.
     *
     * This method queries the underlying backend (e.g., SFML window)
     * for pending events and translates them into OSEvent format.
     *
     * @param out The OSEvent structure to populate with event data
     * @return true if an event was available and populated, false otherwise
     *
     * @note This is a synchronous polling interface. The caller should
     *       call this in a loop until it returns false to process all
     *       pending events.
     */
    virtual bool Poll(OSEvent &out) = 0;
};

}  // namespace Platform
}  // namespace Engine

#endif  // ENGINE_INCLUDE_PLATFORM_IPLATFORMEVENTSOURCE_HPP_
