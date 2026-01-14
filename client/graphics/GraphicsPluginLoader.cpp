/**
 * @file GraphicsPluginLoader.cpp
 * @brief Implementation of GraphicsPluginLoader.
 *
 * Provides platform-abstracted dynamic library loading for graphics plugins.
 */

#include "graphics/GraphicsPluginLoader.hpp"

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include "graphics/GraphicsBackendFactory.hpp"
#include "graphics/IRenderContext.hpp"
#include "graphics/PluginBackendWrapper.hpp"

#include "graphics/plugin_api.h"

// Platform abstraction for dynamic loading
#ifdef _WIN32
#include <windows.h>
typedef HMODULE ModuleHandle;
#define LOAD_LIBRARY(path) LoadLibraryA(path)
#define GET_SYMBOL(handle, name) GetProcAddress(handle, name)
#define GET_ERROR() GetLastError()
#define GET_ERROR_STR() FormatErrorMessage(GetLastError())

static std::string FormatErrorMessage(DWORD error_code) {
    char buffer[256] = {0};
    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer) - 1,
        nullptr);
    return std::string(buffer);
}
#else
#include <dlfcn.h>
typedef void *ModuleHandle;
#define LOAD_LIBRARY(path) dlopen(path, RTLD_LAZY)
#define GET_SYMBOL(handle, name) dlsym(handle, name)
#define GET_ERROR() nullptr
#define GET_ERROR_STR() (dlerror() ? dlerror() : "Unknown error")
#endif

namespace Rtype {
namespace Client {
namespace Graphics {

bool GraphicsPluginLoader::LoadPlugin(
    const std::string &plugin_path, const std::string &backend_name) {
    // Load the shared library
    ModuleHandle handle = LOAD_LIBRARY(plugin_path.c_str());
    if (!handle) {
        std::cerr << "[GraphicsPluginLoader] Failed to load plugin '"
                  << plugin_path << "': " << GET_ERROR_STR() << std::endl;
        return false;
    }

    // Resolve create_graphics_backend_v1
    auto create_func = reinterpret_cast<CreateGraphicsBackendFunc>(
        GET_SYMBOL(handle, PLUGIN_CREATE_SYMBOL_V1));
    if (!create_func) {
        std::cerr << "[GraphicsPluginLoader] Plugin '" << plugin_path
                  << "' missing symbol '" << PLUGIN_CREATE_SYMBOL_V1
                  << "': " << GET_ERROR_STR() << std::endl;
        return false;
    }

    // Resolve destroy_graphics_backend_v1
    auto destroy_func = reinterpret_cast<DestroyGraphicsBackendFunc>(
        GET_SYMBOL(handle, PLUGIN_DESTROY_SYMBOL_V1));
    if (!destroy_func) {
        std::cerr << "[GraphicsPluginLoader] Plugin '" << plugin_path
                  << "' missing symbol '" << PLUGIN_DESTROY_SYMBOL_V1
                  << "': " << GET_ERROR_STR() << std::endl;
        return false;
    }

    // Optional: Resolve graphics_backend_name_v1
    auto name_func = reinterpret_cast<GraphicsBackendNameFunc>(
        GET_SYMBOL(handle, PLUGIN_NAME_SYMBOL_V1));
    // name_func is optional, don't error if missing

    // Register the backend in GraphicsBackendFactory
    // The lambda captures the function pointers and keeps the library loaded
    // (we never call dlclose/FreeLibrary on handle)
    GraphicsBackendFactory::Register(backend_name,
        [create_func, destroy_func](sf::RenderWindow &window)
            -> std::unique_ptr<Engine::Graphics::IRenderContext> {
            // Call plugin's create function
            PluginRenderContextHandle plugin_handle = create_func(nullptr);
            if (!plugin_handle) {
                std::cerr << "[GraphicsPluginLoader] Plugin create function "
                             "returned NULL"
                          << std::endl;
                return nullptr;
            }

            // Cast opaque handle back to IRenderContext*
            auto *plugin_ptr =
                static_cast<Engine::Graphics::IRenderContext *>(plugin_handle);

            // Wrap with PluginBackendWrapper
            // The wrapper will call destroy_func in its destructor
            auto wrapper =
                std::make_unique<Engine::Graphics::PluginBackendWrapper>(
                    plugin_ptr,
                    Engine::Graphics::PluginBackendWrapper::DestroyFunction(
                        [destroy_func](Engine::Graphics::IRenderContext *ptr) {
                            destroy_func(
                                static_cast<PluginRenderContextHandle>(ptr));
                        }));

            return wrapper;
        });

    std::cout << "[GraphicsPluginLoader] Successfully loaded plugin '"
              << plugin_path << "' as backend '" << backend_name << "'"
              << std::endl;
    if (name_func) {
        std::cout << "  Plugin name: " << name_func() << std::endl;
    }

    return true;
}

}  // namespace Graphics
}  // namespace Client
}  // namespace Rtype
