/**
 * @file GraphicsPluginLoader.hpp
 * @brief Runtime loader for graphics backend plugins.
 *
 * This loader enables the engine to load graphics backends as shared libraries
 * at runtime, following the plugin ABI defined in plugin_api.h.
 *
 * DESIGN:
 * - Single-threaded: All calls from game loop only
 * - No unloading: Loaded plugins are kept in memory forever
 * - Error handling: Graceful degradation to fallback backend
 * - Platform abstraction: dlopen/dlsym (Unix) vs LoadLibrary/GetProcAddress
 * (Windows)
 *
 * USAGE:
 *   if (!GraphicsPluginLoader::LoadPlugin("./plugins/libgraphics_sfml.so",
 * "sfml")) { LOG_WARN("Plugin load failed, falling back to static backend");
 *   }
 */

#ifndef CLIENT_GRAPHICS_GRAPHICSPLUGINLOADER_HPP_
#define CLIENT_GRAPHICS_GRAPHICSPLUGINLOADER_HPP_

#include <string>

namespace Rtype {
namespace Client {
namespace Graphics {

/**
 * @brief Loads and registers graphics backend plugins.
 *
 * This class provides static methods to load graphics backend plugins
 * from shared libraries and automatically register them in the
 * GraphicsBackendFactory for use.
 *
 * THREAD SAFETY: NOT thread-safe. Call during engine initialization only.
 *
 * LIFETIME: Loaded plugins are kept in memory indefinitely. There is no
 * unloading mechanism - the shared library remains loaded until the process
 * exits. This prevents issues with plugin destruction during engine shutdown.
 */
class GraphicsPluginLoader {
 public:
    /**
     * @brief Load a graphics backend plugin from a shared library.
     *
     * Loads the specified shared library, resolves the plugin entry points
     * (create_graphics_backend_v1, destroy_graphics_backend_v1, etc.),
     * creates a factory creator callback, and registers it in
     * GraphicsBackendFactory with the given name.
     *
     * @param plugin_path Path to the shared library file.
     *                     Examples:
     *                     - "./plugins/libgraphics_sfml.so" (Unix)
     *                     - "./plugins/graphics_sfml.dll" (Windows)
     *                     - "@loader_path/graphics.dylib" (macOS)
     *
     * @param backend_name Name to register the backend under.
     *                      Examples: "sfml", "opengl", "sdl"
     *                      This name is used when creating GameWorld:
     *                      GameWorld gw(window, "sfml", server_ip, tcp_port,
     * udp_port)
     *
     * @return True if plugin was successfully loaded and registered.
     *         False if load failed (file not found, invalid symbols, etc.).
     *
     * FAILURE MODES (returns false):
     * - Plugin file not found
     * - Plugin is not a valid shared library
     * - Plugin missing required symbol (create_graphics_backend_v1)
     * - Plugin missing required symbol (destroy_graphics_backend_v1)
     * - Backend name already registered (caller is responsible for unique
     * names)
     *
     * SIDE EFFECTS ON SUCCESS:
     * - Shared library loaded and kept in memory forever
     * - Entry point symbols resolved
     * - PluginBackendWrapper registered in GraphicsBackendFactory
     * - Backend available via GraphicsBackendFactory::Create(backend_name,
     * window)
     *
     * ERROR LOGGING:
     * All errors are logged with context (dlopen error messages, missing
     * symbols, etc.) Caller should check return value and fall back to static
     * backend if needed.
     *
     * EXCEPTIONS: May throw if std::string allocation fails (rare).
     *             Should not throw due to plugin issues (catches internally).
     */
    static bool LoadPlugin(
        const std::string &plugin_path, const std::string &backend_name);

 private:
    /**
     * @brief Internal helper to create a factory callback from plugin symbols.
     *
     * This function is NOT directly used by callers. It's part of the internal
     * implementation that hooks plugin symbols into GraphicsBackendFactory.
     *
     * @param create_func Function pointer to create_graphics_backend_v1
     * @param destroy_func Function pointer to destroy_graphics_backend_v1
     * @return Factory callback suitable for GraphicsBackendFactory::Register
     */
    // Implementation detail - see .cpp for this internal helper
};

}  // namespace Graphics
}  // namespace Client
}  // namespace Rtype

#endif  // CLIENT_GRAPHICS_GRAPHICSPLUGINLOADER_HPP_
