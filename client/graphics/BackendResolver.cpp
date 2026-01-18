/**
 * @file BackendResolver.cpp
 * @brief Graphics backend resolution implementation.
 */

#include "graphics/BackendResolver.hpp"

#include <iostream>
#include <string>

#include "graphics/GraphicsBackendFactory.hpp"
#include "graphics/GraphicsPluginLoader.hpp"

namespace Rtype::Client::Graphics {

bool ResolveGraphicsBackend(const std::string &backend_name,
    const std::string &plugins_dir, const std::string &fallback_name) {
    // If no backend requested, return success (caller will use default)
    if (backend_name.empty()) {
        return true;
    }

    // Check if backend already registered (static or previously loaded)
    if (GraphicsBackendFactory::IsRegistered(backend_name)) {
        std::cout << "[Graphics] Using backend: " << backend_name << std::endl;
        return true;
    }

    // Backend not found statically; attempt plugin load
    std::cout << "[Graphics] Backend '" << backend_name
              << "' not found in static registry. Attempting plugin load..."
              << std::endl;

    // Construct platform-specific plugin filename
    std::string plugin_filename;
#ifdef _WIN32
    plugin_filename = backend_name + ".dll";
#elif __APPLE__
    plugin_filename = "lib" + backend_name + ".dylib";
#else  // Linux and other Unix-like
    plugin_filename = "lib" + backend_name + ".so";
#endif

    // Construct full plugin path
    std::string plugin_path = plugins_dir + "/" + plugin_filename;

    // Attempt to load the plugin
    if (GraphicsPluginLoader::LoadPlugin(plugin_path, backend_name)) {
        std::cout << "[Graphics] Successfully loaded plugin backend: "
                  << backend_name << std::endl;
        return true;
    }

    // Plugin load failed
    std::cerr << "[Graphics] ERROR: Failed to load plugin '" << plugin_path
              << "'" << std::endl;

    // Check if fallback is available
    if (!fallback_name.empty() &&
        GraphicsBackendFactory::IsRegistered(fallback_name)) {
        std::cerr << "[Graphics] WARNING: Falling back to '" << fallback_name
                  << "' backend" << std::endl;
        return true;
    }

    // No fallback available - fatal error
    std::cerr << "[Graphics] FATAL: No fallback backend available. Exiting."
              << std::endl;
    return false;
}

}  // namespace Rtype::Client::Graphics
