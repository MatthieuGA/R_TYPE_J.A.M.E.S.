/**
 * @file BackendResolver.hpp
 * @brief Graphics backend resolution logic with plugin loading support.
 *
 * This module handles the resolution of graphics backends:
 * 1. Check if backend exists in static registry
 * 2. Attempt plugin load if not found
 * 3. Fallback to default with warning if plugin load fails
 * 4. Fatal error if no backend available
 *
 */

#ifndef CLIENT_GRAPHICS_BACKENDRESOLVER_HPP_
#define CLIENT_GRAPHICS_BACKENDRESOLVER_HPP_

#include <string>

namespace sf {
class RenderWindow;
}

namespace Rtype::Client::Graphics {

/**
 * @brief Resolve a graphics backend by name, with plugin loading fallback.
 *
 * Resolution order:
 * 1. If backend_name exists in GraphicsBackendFactory, use it (static or
 *    previously loaded)
 * 2. Otherwise, attempt to load as a plugin:
 *    - Construct platform-specific plugin filename
 *    - Call GraphicsPluginLoader::LoadPlugin
 * 3. If plugin load succeeds, backend is now registered in factory
 * 4. If plugin load fails:
 *    - If a default static backend exists, fallback with warning
 *    - Otherwise, fatal error: print to stderr and return false
 *
 * @param backend_name Requested backend name (e.g., "sfml", "opengl")
 * @param plugins_dir Directory where plugins are located
 *                     (e.g., "./plugins")
 * @param fallback_name Default backend to fallback to if plugin load fails
 *                      (e.g., "sfml")
 * @return True if backend was successfully resolved (either static or plugin)
 *         False if backend could not be resolved (fatal error logged to
 * stderr)
 *
 * BEHAVIOR:
 * - If backend_name is empty, returns true without doing anything
 *   (caller should use fallback default)
 * - If backend_name matches an existing static backend, returns true
 * immediately
 * - If backend_name doesn't exist, attempts plugin load:
 *   - On success: registers plugin in factory, returns true
 *   - On failure:
 *     - If fallback exists: logs warning, returns true (fallback will be used)
 *     - If no fallback: logs error, returns false
 *
 * THREAD SAFETY: NOT thread-safe. Call only during initialization.
 * EXCEPTIONS: May throw std::bad_alloc (rare). Does not throw for plugin
 * issues.
 *
 * PLUGIN FILENAME RULES:
 * - Linux/macOS: lib<name>.so / .dylib
 * - Windows: <name>.dll
 * Example: backend_name="sfml" → "./plugins/libgraphics_sfml.so"
 */
bool ResolveGraphicsBackend(const std::string &backend_name,
    const std::string &plugins_dir = "./plugins",
    const std::string &fallback_name = "sfml");

}  // namespace Rtype::Client::Graphics

#endif  // CLIENT_GRAPHICS_BACKENDRESOLVER_HPP_
