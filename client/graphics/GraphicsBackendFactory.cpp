/**
 * @file GraphicsBackendFactory.cpp
 * @brief Factory implementation for creating graphics backends.
 */

#include "graphics/GraphicsBackendFactory.hpp"

#include <memory>
#include <string>
#include <unordered_map>

namespace Rtype::Client::Graphics {

std::unordered_map<std::string, GraphicsBackendFactory::BackendCreator> &
GraphicsBackendFactory::GetRegistry() {
    // Static registry - shared across all calls
    static std::unordered_map<std::string, BackendCreator> registry;
    return registry;
}

void GraphicsBackendFactory::Register(
    const std::string &name, BackendCreator creator) {
    GetRegistry()[name] = creator;
}

std::unique_ptr<Engine::Graphics::IRenderContext>
GraphicsBackendFactory::Create(
    const std::string &name, sf::RenderWindow &window) {
    auto &registry = GetRegistry();
    auto it = registry.find(name);

    if (it == registry.end()) {
        return nullptr;  // Backend not registered
    }

    return it->second(window);
}

bool GraphicsBackendFactory::IsRegistered(const std::string &name) {
    return GetRegistry().find(name) != GetRegistry().end();
}

}  // namespace Rtype::Client::Graphics
