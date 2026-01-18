/**
 * @file template_plugin.cpp
 * @brief Template graphics backend plugin implementation.
 *
 * This is a minimal, fully-commented example of a graphics backend plugin.
 * It implements a dummy IRenderContext that logs all method calls.
 *
 * To create your own plugin:
 * 1. Rename "template" to your backend name (e.g., "custom", "opengl")
 * 2. Replace the dummy IRenderContext implementation with real rendering code
 * 3. Handle configuration JSON if needed
 * 4. Test with: ./r-type_client 127.0.0.1 50000 user --graphics-backend=<name>
 *
 * KEY PRINCIPLES:
 * - All code is in this .cpp file or private headers (avoid exposing C++
 * details)
 * - No global state or static initialization (all state goes in IRenderContext
 * subclass)
 * - The three C ABI functions are the ONLY exports from this library
 * - Use PLUGIN_EXPORT macro to ensure correct symbol visibility
 * - Exception-safe: catch and handle errors before returning to C code
 */

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

// Engine interfaces (read-only, no linking)
#include "graphics/IRenderContext.hpp"

#include "graphics/plugin_api.h"

namespace {

/**
 * @brief Dummy implementation of IRenderContext for demonstration.
 *
 * A real plugin would:
 * - Create a window or rendering surface
 * - Load shaders and textures
 * - Implement high-performance rendering
 * - Handle platform-specific GPU operations
 *
 * This template just logs method calls.
 * Use this as a starting point to understand the interface.
 */
class TemplateRenderContext : public Engine::Graphics::IRenderContext {
 public:
    /**
     * @brief Constructor - initialize state here, not in global ctors.
     *
     * WHY: Global constructors can crash before the engine is ready.
     * All initialization happens in create_graphics_backend_v1(), which
     * gives you a safe, predictable execution environment.
     */
    explicit TemplateRenderContext(const char *config_json)
        : config_string_(config_json ? config_json : "{}") {
        std::cout << "[TemplateRenderContext] Created with config: "
                  << config_string_ << std::endl;
    }

    ~TemplateRenderContext() override {
        std::cout << "[TemplateRenderContext] Destroyed" << std::endl;
    }

    // ========================================================================
    // IRenderContext Interface Implementation
    // ========================================================================
    // Below are the 6 methods you MUST implement. Each one corresponds to a
    // rendering operation. In a real plugin, these would issue GPU commands.
    // For now, they just log calls.
    // ========================================================================

    /**
     * @brief Draw a sprite (texture + transform).
     *
     * Called for each sprite in the scene. Examples:
     * - Player ship
     * - Enemies
     * - Bullets
     * - Background elements
     *
     * @param sprite Sprite data (texture path, position, rotation, color,
     * etc.)
     * @param shader Optional shader to apply (can be nullptr for default
     * rendering)
     *
     * COMMON PATTERN:
     * - Load/cache texture from sprite.texture_path
     * - Apply sprite.color tint (with alpha blending)
     * - Apply sprite.rotation_degrees rotation
     * - Draw at sprite.position
     */
    void DrawSprite(const Engine::Graphics::DrawableSprite &sprite,
        const Engine::Graphics::DrawableShader *shader = nullptr) override {
        std::cout << "[TemplateRenderContext::DrawSprite] texture="
                  << sprite.texture_path << " pos=(" << sprite.position.x
                  << "," << sprite.position.y << ")";
        if (shader != nullptr) {
            std::cout << " shader=" << shader->shader_path;
        }
        std::cout << std::endl;
    }

    /**
     * @brief Draw text (font + string + position).
     *
     * Called for UI text, labels, debug output.
     *
     * @param text Text data (font path, string, size, position, color)
     *
     * COMMON PATTERN:
     * - Load/cache font from text.font_path
     * - Render string at text.position with text.size and text.color
     * - Apply text.scale if not (1.0, 1.0)
     */
    void DrawText(const Engine::Graphics::DrawableText &text) override {
        std::cout << "[TemplateRenderContext::DrawText] text=\"" << text.text
                  << "\" pos=(" << text.position.x << "," << text.position.y
                  << ")" << std::endl;
    }

    /**
     * @brief Draw a filled rectangle.
     *
     * Called for UI elements, debug visuals, or simple geometry.
     *
     * @param rect Rectangle data (position, size, color)
     */
    void DrawRectangle(
        const Engine::Graphics::DrawableRectangle &rect) override {
        std::cout << "[TemplateRenderContext::DrawRectangle] pos=("
                  << rect.position.x << "," << rect.position.y << ") size=("
                  << rect.size.x << "," << rect.size.y << ")" << std::endl;
    }

    /**
     * @brief Draw a vertex array (custom geometry).
     *
     * Called for rendering arbitrary vertex data (lines, polygons, etc).
     *
     * @param vertices Vertex array data with count and primitive type
     */
    void DrawVertexArray(
        const Engine::Graphics::VertexArray &vertices) override {
        std::cout << "[TemplateRenderContext::DrawVertexArray] vertices="
                  << vertices.vertex_count << std::endl;
    }

    /**
     * @brief Query texture dimensions.
     *
     * Used by systems to determine sprite bounds and frame setup.
     * Backend is responsible for caching textures and returning their
     * dimensions.
     *
     * @param texture_path Path to texture file
     * @return Texture dimensions {width, height} in pixels, or {0, 0} if not
     * found
     *
     * COMMON PATTERN:
     * - Load/cache texture from file
     * - Return {texture.width, texture.height}
     * - Return {0, 0} if file doesn't exist
     */
    Engine::Graphics::Vector2f GetTextureSize(
        const char *texture_path) override {
        if (texture_path == nullptr) {
            return {0.0f, 0.0f};
        }
        std::cout << "[TemplateRenderContext::GetTextureSize] texture="
                  << texture_path << std::endl;
        // Return a dummy size
        return {256.0f, 256.0f};
    }

    /**
     * @brief Query text bounds.
     *
     * Used by text rendering systems to calculate centering and layout.
     * Returns the bounding box of rendered text.
     *
     * @param font_path Path to font file
     * @param text Text string to measure
     * @param character_size Font size in pixels
     * @return Bounding box {x, y, width, height} of the text (width/height
     * only needed)
     *
     * COMMON PATTERN:
     * - Load/cache font from file
     * - Measure text with given size
     * - Return measured dimensions
     */
    Engine::Graphics::Vector2f GetTextBounds(const char *font_path,
        const char *text, unsigned int character_size) override {
        if (font_path == nullptr || text == nullptr) {
            return {0.0f, 0.0f};
        }
        std::cout << "[TemplateRenderContext::GetTextBounds] font="
                  << font_path << " text=\"" << text
                  << "\" size=" << character_size << std::endl;
        // Return a dummy size (width = text length * char width, height = char
        // size)
        return {static_cast<float>(std::strlen(text) * character_size / 2),
            static_cast<float>(character_size)};
    }

    /**
     * @brief Query grid frame size for animation systems.
     *
     * For grid-based sprite sheets, calculates dimensions of a single frame
     * given the grid layout.
     *
     * @param texture_path Path to texture file
     * @param grid_cols Number of columns in the grid
     * @param frame_width Width of one frame in pixels
     * @return Frame dimensions {width, height} in pixels
     *
     * COMMON PATTERN:
     * - Load/cache texture from file
     * - Calculate frame_height = (texture_height / (texture_width /
     * frame_width))
     * - Return {frame_width, frame_height}
     */
    Engine::Graphics::Vector2i GetGridFrameSize(
        const char *texture_path, int grid_cols, int frame_width) override {
        if (texture_path == nullptr || grid_cols <= 0 || frame_width <= 0) {
            return {0, 0};
        }
        std::cout << "[TemplateRenderContext::GetGridFrameSize] texture="
                  << texture_path << " grid_cols=" << grid_cols
                  << " frame_width=" << frame_width << std::endl;
        // Return a dummy size
        return {frame_width, 64};
    }

 private:
    // Store any configuration for reference
    // In a real plugin, this might configure graphics settings
    std::string config_string_;
};

}  // namespace

// ============================================================================
// C ABI EXPORTS (Plugin Entry Points)
// ============================================================================
// These three functions are the ONLY symbols exported by this plugin.
// The engine uses dlsym() / GetProcAddress() to find them by name.
// ============================================================================

/**
 * @brief Create a graphics backend instance.
 *
 * CALLED BY: Engine during initialization (once per game instance).
 * PARAMETERS:
 *   config_json: Optional JSON config (e.g., "{\"enable_vsync\": true}")
 *                Can be NULL for default configuration.
 * RETURNS:
 *   Non-NULL IRenderContext* on success
 *   NULL on failure (missing dependencies, bad config, etc.)
 *
 * OWNERSHIP:
 *   You allocate. Engine wraps in unique_ptr with custom deleter that
 *   calls destroy_graphics_backend_v1() when done.
 *
 * THREADING:
 *   Called from main thread during engine init (single-threaded).
 *
 * EXCEPTIONS:
 *   May throw C++ exceptions. Engine catches and logs, then tries fallback.
 */
extern "C" PLUGIN_EXPORT Engine::Graphics::IRenderContext *
create_graphics_backend_v1(const char *config_json) {
    try {
        std::cout << "[Template Plugin] create_graphics_backend_v1 called"
                  << std::endl;

        // Validate input (defensive programming)
        if (config_json == nullptr) {
            std::cout << "[Template Plugin] No config provided, using defaults"
                      << std::endl;
        }

        // Allocate and initialize your render context
        // In a real plugin, this would:
        // - Create a window
        // - Initialize GPU state
        // - Parse config JSON
        // - Load default shaders/textures
        auto *context = new TemplateRenderContext(config_json);

        // Return as IRenderContext* (virtual interface pointer)
        return context;
    } catch (const std::exception &e) {
        // Catch all exceptions (never propagate across C boundary)
        std::cerr
            << "[Template Plugin] Exception in create_graphics_backend_v1: "
            << e.what() << std::endl;
        return nullptr;  // Signal failure to engine
    } catch (...) {
        std::cerr << "[Template Plugin] Unknown exception in "
                     "create_graphics_backend_v1"
                  << std::endl;
        return nullptr;
    }
}

/**
 * @brief Destroy a graphics backend instance.
 *
 * CALLED BY: Engine custom deleter during shutdown (once per game instance).
 * PARAMETERS:
 *   handle: Non-NULL IRenderContext* from create_graphics_backend_v1()
 *           After this call, handle is INVALID.
 * RETURNS:
 *   Nothing.
 *
 * OWNERSHIP:
 *   You free. Engine will NOT call delete or free on the handle.
 *
 * THREADING:
 *   Called from main thread during engine shutdown (single-threaded).
 *
 * SAFETY:
 *   Passing an invalid handle is undefined behavior (likely crash).
 *   Calling twice with same handle causes double-delete (heap corruption).
 *
 * EXCEPTIONS:
 *   Should not throw. If cleanup fails, log to stderr and continue.
 */
extern "C" PLUGIN_EXPORT void destroy_graphics_backend_v1(
    Engine::Graphics::IRenderContext *handle) {
    try {
        std::cout << "[Template Plugin] destroy_graphics_backend_v1 called"
                  << std::endl;

        if (handle == nullptr) {
            std::cerr
                << "[Template Plugin] Warning: destroy called with NULL handle"
                << std::endl;
            return;
        }

        // Delete the context (calls destructor)
        // The virtual destructor ensures proper cleanup
        delete handle;
    } catch (const std::exception &e) {
        std::cerr
            << "[Template Plugin] Exception in destroy_graphics_backend_v1: "
            << e.what() << std::endl;
        // Don't re-throw. Engine has nothing to catch.
    } catch (...) {
        std::cerr << "[Template Plugin] Unknown exception in "
                     "destroy_graphics_backend_v1"
                  << std::endl;
    }
}

/**
 * @brief Get the human-readable name of this graphics backend.
 *
 * CALLED BY: Engine during plugin discovery (once per plugin).
 * PARAMETERS: None.
 * RETURNS:
 *   Non-NULL pointer to null-terminated C string.
 *   Example: "Template Graphics Backend v1.0"
 *
 * OWNERSHIP:
 *   Plugin owns the returned pointer. String must be valid for plugin lifetime
 *   (typically a static string constant).
 *
 * THREADING:
 *   Called from main thread during startup (single-threaded).
 *
 * PURPOSE:
 *   Logging and debugging. User never sees this unless they enable verbose
 * logging.
 *
 * EXCEPTIONS:
 *   Should not throw.
 */
extern "C" PLUGIN_EXPORT const char *graphics_backend_name_v1(void) {
    // Return a static string constant (simplest, safest approach)
    return "Template Graphics Backend v1.0";
}
