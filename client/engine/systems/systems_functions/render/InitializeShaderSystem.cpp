#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Load and initialize a fragment shader for a Shader component.
 *
 * Marks the shader as loaded (backend will load via shader_path).
 * Stores preconfigured float uniforms for backend rendering.
 *
 * @param shader_comp Shader component to initialize
 */
void InitializeShader(Com::Shader &shader_comp) {
    if (!shader_comp.shader_path.empty()) {
        // Backend will load shader via shader_path
        // Just mark as loaded for rendering pipeline
        shader_comp.is_loaded = true;
    }
}

/**
 * @brief System that initializes all Shader components that are not loaded.
 *
 * Iterates over the shader components and calls `InitializeShader` for
 * components that have a `shader_path` and are not yet loaded.
 *
 * @param reg Engine registry (unused)
 * @param shaders Sparse array of Shader components
 */
void InitializeShaderSystem(
    Eng::registry &reg, Eng::sparse_array<Com::Shader> &shaders) {
    std::vector<int> draw_order;

    for (auto &&[i, shader] : make_indexed_zipper(shaders)) {
        if (!shader.is_loaded && !shader.shader_path.empty())
            InitializeShader(shader);
    }
}
}  // namespace Rtype::Client
