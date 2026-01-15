/**
 * @file GraphicsManager.hpp
 * @brief High-level graphics manager wrapping a backend implementation.
 *
 * This class provides a simplified API for graphics operations,
 * delegating to a pluggable backend. Mirrors the AudioManager pattern.
 *
 * NOTE: This is a MINIMAL implementation for PR 1.7 (plumbing only).
 * Future PRs will expand with resource management and drawing operations.
 */

#ifndef ENGINE_INCLUDE_GRAPHICS_GRAPHICSMANAGER_HPP_
#define ENGINE_INCLUDE_GRAPHICS_GRAPHICSMANAGER_HPP_

#include <memory>

#include "graphics/IGraphicsBackend.hpp"

namespace Engine::Graphics {

/**
 * @brief High-level graphics manager.
 *
 * Wraps a graphics backend and provides a simplified API.
 * The backend is injected via constructor (dependency injection).
 *
 * SCOPE: PR 1.7 - Frame lifecycle only.
 */
class GraphicsManager {
 public:
    /**
     * @brief Construct GraphicsManager with a backend.
     *
     * @param backend The graphics backend to use (e.g., SFMLGraphicsBackend)
     */
    explicit GraphicsManager(std::unique_ptr<IGraphicsBackend> backend);

    ~GraphicsManager();

    // Prevent copying
    GraphicsManager(const GraphicsManager &) = delete;
    GraphicsManager &operator=(const GraphicsManager &) = delete;

    // Allow moving
    GraphicsManager(GraphicsManager &&) noexcept;
    GraphicsManager &operator=(GraphicsManager &&) noexcept;

    /**
     * @brief Begin a new frame.
     * @param clear_color Background color
     */
    void BeginFrame(const Color &clear_color = Color::Black);

    /**
     * @brief End the current frame.
     */
    void EndFrame();

    // TODO(PR 1.8): Add resource loading methods
    // TODO(PR 1.9): Add drawing methods

 private:
    std::unique_ptr<IGraphicsBackend> backend_;
};

}  // namespace Engine::Graphics

#endif  // ENGINE_INCLUDE_GRAPHICS_GRAPHICSMANAGER_HPP_
