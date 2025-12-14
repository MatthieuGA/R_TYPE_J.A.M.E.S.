#include <algorithm>
#include <iostream>
#include <memory>
#include <vector>

#include "engine/systems/InitRegistrySystems.hpp"

namespace Rtype::Client {
/**
 * @brief Load and initialize a shader for a Shader component via video
 * backend.
 *
 * @param shader_comp Shader component to initialize
 * @param game_world Game world containing video backend
 */
void InitializeShader(Com::Shader &shader_comp, GameWorld &game_world) {
    if (!game_world.rendering_engine_) {
        std::cerr << "[InitializeShader] ERROR: video_backend is null!"
                  << std::endl;
        return;
    }

    std::cout << "[InitializeShader] Loading shader: '"
              << shader_comp.shader_path << "' with ID: '"
              << shader_comp.shader_id << "'" << std::endl;

    // Load shader via video backend (fragment shader only for now)
    bool loaded = game_world.rendering_engine_->LoadShader(
        shader_comp.shader_id, "", shader_comp.shader_path);

    if (!loaded) {
        std::cerr << "[InitializeShader] ERROR: Failed to load shader: "
                  << shader_comp.shader_path
                  << " (ID: " << shader_comp.shader_id << ")" << std::endl;
    } else {
        std::cout << "[InitializeShader] Successfully loaded shader: "
                  << shader_comp.shader_id << std::endl;
        shader_comp.is_loaded = true;
    }
}

/**
 * @brief System that initializes all Shader components that are not loaded.
 *
 * @param reg Engine registry (unused)
 * @param game_world Game world containing video backend
 * @param shaders Sparse array of Shader components
 */
void InitializeShaderSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Shader> &shaders) {
    (void)reg;

    for (auto &&[i, shader] : make_indexed_zipper(shaders)) {
        if (!shader.is_loaded && !shader.shader_path.empty()) {
            InitializeShader(shader, game_world);
        }
    }
}
}  // namespace Rtype::Client
