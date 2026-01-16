/**
 * @file IGraphicsBackend.hpp
 * @brief Abstract graphics backend interface for rendering operations.
 *
 * This interface defines the minimal contract for graphics backends.
 * It provides frame management without exposing backend-specific details.
 *
 * NOTE: This is a MINIMAL interface for PR 1.7 (plumbing only).
 * Future PRs will expand this with texture loading, sprite rendering, etc.
 */

#ifndef ENGINE_INCLUDE_GRAPHICS_IGRAPHICSBACKEND_HPP_
#define ENGINE_INCLUDE_GRAPHICS_IGRAPHICSBACKEND_HPP_

#include "graphics/Types.hpp"

namespace Engine::Graphics {

/**
 * @brief Abstract interface for graphics backend implementations.
 *
 * Provides minimal rendering operations without backend dependencies.
 * Backend implementations (SFML, SDL, etc.) handle the actual rendering.
 *
 * SCOPE: PR 1.7 - Frame lifecycle only. Resource management comes later.
 */
class IGraphicsBackend {
 public:
    virtual ~IGraphicsBackend() = default;

    /**
     * @brief Begin a new frame.
     *
     * Prepares the backend for rendering a new frame.
     * Should be called before any draw operations.
     *
     * @param clear_color Background color to clear the screen
     */
    virtual void BeginFrame(const Color &clear_color) = 0;

    /**
     * @brief End the current frame.
     *
     * Finalizes rendering for this frame.
     * Should be called after all draw operations.
     */
    virtual void EndFrame() = 0;

    // TODO(PR 1.8): Add resource loading (LoadTexture, LoadFont, etc.)
    // TODO(PR 1.9): Add drawing operations (DrawSprite, DrawText, etc.)
};

}  // namespace Engine::Graphics

#endif  // ENGINE_INCLUDE_GRAPHICS_IGRAPHICSBACKEND_HPP_
