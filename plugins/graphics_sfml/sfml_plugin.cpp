/**
 * @file sfml_plugin.cpp
 * @brief Graphics backend plugin implementing the Phase A C ABI.
 *
 * This plugin wraps the SFMLRenderContext implementation as a standalone
 * shared library, demonstrating the plugin loading architecture.
 *
 * DESIGN NOTE:
 * The plugin_api.h interface passes only config_json to
 * create_graphics_backend_v1(). However, SFMLRenderContext requires an
 * sf::RenderWindow reference for initialization.
 *
 * SOLUTION:
 * This plugin uses a static pointer to store the window, set by a
 * plugin-specific initialization function. In the current architecture,
 * GameWorld calls:
 *   GraphicsPluginLoader::LoadPlugin("./plugins/libgraphics_sfml.so", "sfml")
 *
 * The loader then calls the factory's lambda with the window. The lambda must
 * set the window before calling create_graphics_backend_v1.
 *
 * This limitation could be addressed in a future ABI version by passing the
 * window through config_json (encoded as pointer hex) or extending the ABI.
 */

#include <cstring>
#include <iostream>
#include <memory>

// Engine/Plugin interface
#include "graphics/IRenderContext.hpp"

#include "graphics/plugin_api.h"

// SFML backend implementation (reused from client)
#include "graphics/SFMLRenderContext.hpp"

// Forward declaration
namespace sf {
class RenderWindow;
}

namespace {

// Global storage for the render window.
// This is set by a plugin initialization function before create is called.
//
// THREAD SAFETY: NOT thread-safe. This plugin is designed for single-threaded
// game loop usage. If threading is needed, use thread-local storage.
//
// BETTER DESIGN: Future ABI versions should pass the window through the config
// JSON or extend the function signature. For now, this is a pragmatic
// reference implementation.
thread_local sf::RenderWindow *g_render_window = nullptr;

}  // namespace

// ============================================================================
// Plugin Initialization (Plugin-Specific, Not Part of Standard ABI)
// ============================================================================

/**
 * @brief Set the render window for this plugin.
 *
 * IMPORTANT: This must be called before create_graphics_backend_v1().
 * This is a plugin-specific initialization function, not part of the
 * standard plugin_api.h ABI.
 *
 * @param window Pointer to the sf::RenderWindow. Must remain valid until
 *               destroy_graphics_backend_v1() is called.
 *
 * FUTURE: This function should be replaced by passing the window through
 * the standard ABI (e.g., config_json or extended signature).
 */
extern "C" void graphics_sfml_set_window(sf::RenderWindow *window) {
    g_render_window = window;
}

// ============================================================================
// Standard Plugin ABI (plugin_api.h)
// ============================================================================

/**
 * @brief Create a graphics backend instance.
 *
 * Allocates and initializes an SFMLRenderContext wrapping the global
 * render window set by graphics_sfml_set_window().
 *
 * PRECONDITION: graphics_sfml_set_window() must have been called first.
 * EXCEPTION SAFETY: Catches all exceptions and returns nullptr.
 * THREAD SAFETY: Uses thread-local storage for the window pointer.
 *
 * @param config_json Configuration string (currently unused). Provided for
 *                     future extensibility. May be NULL.
 * @return Pointer to allocated IRenderContext on success, nullptr on failure.
 */
extern "C" PLUGIN_EXPORT Engine::Graphics::IRenderContext *
create_graphics_backend_v1(const char *config_json) {
    try {
        // Validate precondition: window must be set
        if (!g_render_window) {
            std::cerr
                << "[graphics_sfml] ERROR: Window not set. "
                << "Call graphics_sfml_set_window() before creating backend."
                << std::endl;
            return nullptr;
        }

        // Allocate backend (caller will wrap this in PluginBackendWrapper)
        auto *backend =
            new Rtype::Client::Graphics::SFMLRenderContext(*g_render_window);

        return backend;
    } catch (const std::exception &e) {
        std::cerr
            << "[graphics_sfml] EXCEPTION in create_graphics_backend_v1: "
            << e.what() << std::endl;
        return nullptr;
    } catch (...) {
        std::cerr << "[graphics_sfml] UNKNOWN EXCEPTION in "
                     "create_graphics_backend_v1"
                  << std::endl;
        return nullptr;
    }
}

/**
 * @brief Destroy a graphics backend instance.
 *
 * Deallocates an IRenderContext previously allocated by
 * create_graphics_backend_v1().
 *
 * EXCEPTION SAFETY: Never throws. Exceptions are caught and logged.
 * IDEMPOTENCY: Safe to call multiple times (checks for nullptr).
 *
 * @param ctx Pointer to backend instance. May be nullptr (ignored safely).
 */
extern "C" PLUGIN_EXPORT void destroy_graphics_backend_v1(
    Engine::Graphics::IRenderContext *ctx) {
    try {
        if (ctx) {
            delete ctx;
        }
    } catch (const std::exception &e) {
        std::cerr
            << "[graphics_sfml] EXCEPTION in destroy_graphics_backend_v1: "
            << e.what() << std::endl;
    } catch (...) {
        std::cerr << "[graphics_sfml] UNKNOWN EXCEPTION in "
                     "destroy_graphics_backend_v1"
                  << std::endl;
    }
}

/**
 * @brief Query the backend's human-readable name.
 *
 * Returns a static string describing this plugin. Used for logging and
 * debugging during plugin discovery.
 *
 * EXCEPTION SAFETY: Never throws.
 * THREAD SAFETY: Const, safe to call from any thread.
 *
 * @return Non-null pointer to a null-terminated string.
 */
extern "C" PLUGIN_EXPORT const char *graphics_backend_name_v1() {
    return "SFML 2.6+ Graphics Backend Plugin";
}
