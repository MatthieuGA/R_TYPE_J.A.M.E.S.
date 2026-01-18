/**
 * @file plugin_api.h
 * @brief C ABI (Application Binary Interface) for graphics backend plugins.
 *
 * This header defines the contract that graphics backend plugins must
 * implement. Plugins are loaded at runtime as shared libraries
 * (.so/.dylib/.dll) and communicate with the engine through this C-linkage
 * interface.
 *
 * CRITICAL CONSTRAINTS:
 * - All extern "C" symbols must be mangled safely for C++ linkage
 * - Plugins return bare IRenderContext* pointers (not unique_ptr)
 * - Plugins must provide a destroy function (not rely on delete)
 * - Plugins MUST NOT be unloaded (no dlclose/FreeLibrary calls)
 * - Threading: All calls are single-threaded from game loop
 *
 * VERSION: 1
 * This is the versioned plugin interface. Future breaking changes should
 * increment to v2, v3, etc., allowing old plugins to coexist.
 */

#ifndef ENGINE_INCLUDE_GRAPHICS_PLUGIN_API_H_
#define ENGINE_INCLUDE_GRAPHICS_PLUGIN_API_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Opaque handle to a render context instance created by plugin.
 *
 * Defined as void* to maintain C ABI compatibility. The actual type is
 * Engine::Graphics::IRenderContext*, but we use void* to avoid exposing
 * C++ type information to C code.
 */
typedef void *PluginRenderContextHandle;

/**
 * @brief Create a graphics backend instance with configuration.
 *
 * OWNERSHIP: Plugin allocates; caller must eventually call
 * destroy_graphics_backend_v1() to release.
 *
 * @param config_json JSON configuration string (backend-specific).
 *                     May be NULL for default config.
 *                     Example: "{\"enable_vsync\": true, \"samples\": 4}"
 * @return Non-NULL PluginRenderContextHandle on success, NULL on failure.
 *         The handle is an opaque wrapper around
 * Engine::Graphics::IRenderContext*.
 *
 * THREADING: Must be called once during engine initialization
 * (single-threaded). EXCEPTIONS: Plugin may throw; loader catches and logs.
 *
 * CONTRACT VIOLATION = ENGINE CRASH:
 * Returning NULL or invalid pointer WILL crash the engine during rendering.
 * Plugins must validate their internal state before returning.
 */
typedef PluginRenderContextHandle (*CreateGraphicsBackendFunc)(
    const char *config_json);

/**
 * @brief Destroy a graphics backend instance.
 *
 * OWNERSHIP: Releases resources allocated by create_graphics_backend_v1().
 * After this call, the handle is INVALID and must not be used.
 *
 * @param handle Non-NULL handle returned by create_graphics_backend_v1().
 *
 * THREADING: Called during engine shutdown (single-threaded).
 * EXCEPTIONS: Plugin may throw; loader logs but doesn't propagate.
 * SAFETY: Passing invalid handle is undefined behavior.
 *
 * CONTRACT VIOLATION = ENGINE CRASH OR MEMORY LEAK:
 * - Failing to release resources → Memory leak
 * - Double-delete → Heap corruption
 * - Invalid handle → Segfault
 */
typedef void (*DestroyGraphicsBackendFunc)(PluginRenderContextHandle handle);

/**
 * @brief Query the human-readable name of the graphics backend.
 *
 * OWNERSHIP: Plugin owns returned pointer; must remain valid for the
 * entire plugin lifetime.
 *
 * @return Non-NULL pointer to null-terminated string (e.g., "SFML Graphics").
 *
 * THREADING: Called once during plugin discovery (single-threaded).
 * EXCEPTIONS: Should not throw.
 *
 * PURPOSE: Used for logging and debugging. Example output:
 *   "SFML 2.6.1 Graphics Backend"
 *   "OpenGL 4.6 Graphics Backend"
 */
typedef const char *(*GraphicsBackendNameFunc)(void);

/**
 * @brief Plugin entry point v1 - Create function.
 *
 * Symbol name: create_graphics_backend_v1
 * Expected signature: CreateGraphicsBackendFunc
 *
 * Loader will call dlsym() / GetProcAddress() to resolve this symbol.
 * Must be exported from the plugin's .so/.dylib/.dll file.
 *
 * VERSIONING: If the engine needs to load v2 plugins in the future,
 * it will look for "create_graphics_backend_v2" symbols, allowing
 * old v1 plugins to remain compatible indefinitely.
 */
#define PLUGIN_CREATE_SYMBOL_V1 "create_graphics_backend_v1"

/**
 * @brief Plugin entry point v1 - Destroy function.
 *
 * Symbol name: destroy_graphics_backend_v1
 * Expected signature: DestroyGraphicsBackendFunc
 */
#define PLUGIN_DESTROY_SYMBOL_V1 "destroy_graphics_backend_v1"

/**
 * @brief Plugin entry point v1 - Name function.
 *
 * Symbol name: graphics_backend_name_v1
 * Expected signature: GraphicsBackendNameFunc
 * OPTIONAL: Plugin may omit this symbol (loader checks before calling).
 */
#define PLUGIN_NAME_SYMBOL_V1 "graphics_backend_name_v1"

/**
 * @brief Helper macro for plugin authors to export symbols.
 *
 * Use in plugin implementation:
 *   extern "C" {
 *   PLUGIN_EXPORT IRenderContext* create_graphics_backend_v1(...) { ... }
 *   PLUGIN_EXPORT void destroy_graphics_backend_v1(...) { ... }
 *   PLUGIN_EXPORT const char* graphics_backend_name_v1() { ... }
 *   }
 */
#if defined(_WIN32) || defined(_WIN64)
#define PLUGIN_EXPORT __declspec(dllexport)
#else
#define PLUGIN_EXPORT __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ENGINE_INCLUDE_GRAPHICS_PLUGIN_API_H_
