/**
 * @file PluginBackendWrapper.hpp
 * @brief Wrapper class that adapts plugin raw pointers to std::unique_ptr
 * semantics.
 *
 * PROBLEM: Plugin ABI returns raw IRenderContext* and requires explicit
 * destroy_graphics_backend_v1() call. GraphicsBackendFactory expects
 * std::function<std::unique_ptr<IRenderContext>>. These patterns are
 * incompatible without this adapter.
 *
 * SOLUTION (Option A): PluginBackendWrapper inherits from IRenderContext,
 * forwards all virtual methods to the plugin's IRenderContext*, and calls
 * destroy_graphics_backend_v1() in its destructor. This allows the wrapper
 * to be managed with std::unique_ptr using the default deleter (calls delete),
 * which triggers the destructor and cleanup.
 *
 * OWNERSHIP FLOW:
 * 1. Plugin's create_graphics_backend_v1() returns IRenderContext*
 * 2. GraphicsPluginLoader wraps it: new PluginBackendWrapper(ptr, destroy_fn)
 * 3. GraphicsBackendFactory receives:
 * std::make_unique<PluginBackendWrapper>(...)
 * 4. GameWorld owns: std::unique_ptr<IRenderContext> = unique_ptr(wrapper)
 * 5. On GameWorld destruction, unique_ptr calls delete wrapper
 * 6. Wrapper destructor calls destroy_graphics_backend_v1(plugin_ptr)
 */

#ifndef ENGINE_INCLUDE_GRAPHICS_PLUGINBACKENDWRAPPER_HPP_
#define ENGINE_INCLUDE_GRAPHICS_PLUGINBACKENDWRAPPER_HPP_

#include <cassert>
#include <functional>

#include "graphics/IRenderContext.hpp"

#include "graphics/plugin_api.h"

namespace Engine {
namespace Graphics {

/**
 * @brief Wrapper that adapts plugin raw pointers to std::unique_ptr semantics.
 *
 * This class bridges the gap between:
 * - Plugin ABI: IRenderContext* with explicit destroy function
 * - Factory pattern: std::function<std::unique_ptr<IRenderContext>>
 *
 * The wrapper forwards all virtual methods to the plugin's IRenderContext*
 * and calls the destroy function in its destructor.
 *
 * THREAD SAFETY: Not thread-safe. All calls must be from the game thread.
 * EXCEPTIONS: Forwards any exceptions from plugin methods.
 */
class PluginBackendWrapper : public IRenderContext {
 public:
    /// Type for the destroy function pointer
    using DestroyFunction = std::function<void(IRenderContext *)>;

    /**
     * @brief Construct wrapper with plugin pointer and destroy function.
     *
     * @param plugin_ptr Non-NULL pointer from plugin's create function.
     * @param destroy_fn Function to call when wrapper is destroyed.
     *
     * PRECONDITIONS:
     * - plugin_ptr must be non-NULL and valid
     * - destroy_fn must be valid (will be called in destructor)
     * - plugin_ptr must remain valid until destructor is called
     *
     * ASSERTION FAILURES: Invalid preconditions cause assert() failures in
     * debug builds. In release builds, behavior is undefined.
     */
    explicit PluginBackendWrapper(
        IRenderContext *plugin_ptr, DestroyFunction destroy_fn)
        : plugin_ptr_(plugin_ptr), destroy_fn_(destroy_fn) {
        assert(plugin_ptr != nullptr);
        assert(destroy_fn != nullptr);
    }

    /**
     * @brief Destructor - calls plugin's destroy function.
     *
     * This is the key mechanism: the wrapper is destroyed (via delete from
     * std::unique_ptr), which triggers this destructor, which calls the
     * plugin's destroy function, which releases the plugin's resources.
     *
     * EXCEPTION SAFETY: If destroy_fn throws, std::terminate is called
     * (noexcept specification). This is intentional - we're in a destructor.
     */
    ~PluginBackendWrapper() noexcept {
        if (plugin_ptr_ && destroy_fn_) {
            try {
                destroy_fn_(plugin_ptr_);
            } catch (...) {
                // Log but don't propagate - we're in a destructor
                // In a real implementation, use engine's logger here
            }
        }
    }

    // Disable copying - each wrapper is tied to a specific plugin instance
    PluginBackendWrapper(const PluginBackendWrapper &) = delete;
    PluginBackendWrapper &operator=(const PluginBackendWrapper &) = delete;

    // Disable moving - maintains invariant that plugin_ptr_ never changes
    PluginBackendWrapper(PluginBackendWrapper &&) = delete;
    PluginBackendWrapper &operator=(PluginBackendWrapper &&) = delete;

    // ========== IRenderContext Virtual Methods (Forwarded) ==========

    /**
     * @brief Draw a sprite (forwarded to plugin).
     */
    void DrawSprite(const DrawableSprite &sprite,
        const DrawableShader *shader = nullptr) override {
        assert(plugin_ptr_ != nullptr);
        plugin_ptr_->DrawSprite(sprite, shader);
    }

    /**
     * @brief Draw text (forwarded to plugin).
     */
    void DrawText(const DrawableText &text) override {
        assert(plugin_ptr_ != nullptr);
        plugin_ptr_->DrawText(text);
    }

    /**
     * @brief Draw a rectangle (forwarded to plugin).
     */
    void DrawRectangle(const DrawableRectangle &rect) override {
        assert(plugin_ptr_ != nullptr);
        plugin_ptr_->DrawRectangle(rect);
    }

    /**
     * @brief Draw a vertex array (forwarded to plugin).
     */
    void DrawVertexArray(const VertexArray &vertices) override {
        assert(plugin_ptr_ != nullptr);
        plugin_ptr_->DrawVertexArray(vertices);
    }

    /**
     * @brief Query texture size (forwarded to plugin).
     */
    Vector2f GetTextureSize(const char *texture_path) override {
        assert(plugin_ptr_ != nullptr);
        return plugin_ptr_->GetTextureSize(texture_path);
    }

    /**
     * @brief Query text bounds (forwarded to plugin).
     */
    Vector2f GetTextBounds(const char *font_path, const char *text,
        unsigned int character_size) override {
        assert(plugin_ptr_ != nullptr);
        return plugin_ptr_->GetTextBounds(font_path, text, character_size);
    }

    /**
     * @brief Query grid frame size (forwarded to plugin).
     */
    Vector2i GetGridFrameSize(
        const char *texture_path, int grid_cols, int frame_width) override {
        assert(plugin_ptr_ != nullptr);
        return plugin_ptr_->GetGridFrameSize(
            texture_path, grid_cols, frame_width);
    }

 private:
    /// Pointer to the actual plugin render context
    IRenderContext *plugin_ptr_;

    /// Function to call for cleanup
    DestroyFunction destroy_fn_;
};

}  // namespace Graphics
}  // namespace Engine

#endif  // ENGINE_INCLUDE_GRAPHICS_PLUGINBACKENDWRAPPER_HPP_
