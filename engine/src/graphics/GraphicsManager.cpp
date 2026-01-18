/**
 * @file GraphicsManager.cpp
 * @brief Implementation of GraphicsManager.
 */

#include "graphics/GraphicsManager.hpp"

#include <utility>

namespace Engine::Graphics {

GraphicsManager::GraphicsManager(std::unique_ptr<IGraphicsBackend> backend)
    : backend_(std::move(backend)) {}

GraphicsManager::~GraphicsManager() = default;

GraphicsManager::GraphicsManager(GraphicsManager &&) noexcept = default;

GraphicsManager &GraphicsManager::operator=(
    GraphicsManager &&) noexcept = default;

void GraphicsManager::BeginFrame(const Color &clear_color) {
    if (backend_) {
        backend_->BeginFrame(clear_color);
    }
}

void GraphicsManager::EndFrame() {
    if (backend_) {
        backend_->EndFrame();
    }
}

}  // namespace Engine::Graphics
