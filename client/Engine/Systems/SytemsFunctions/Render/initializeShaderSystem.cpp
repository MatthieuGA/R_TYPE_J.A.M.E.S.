#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {
/**
 * @brief Load and initialize a fragment shader for a Shader component.
 *
 * Loads the shader from disk, sets the `texture` uniform and applies any
 * preconfigured float uniforms.
 *
 * @param shader_comp Shader component to initialize
 */
void InitializeShader(Com::Shader &shader_comp) {
    if (!shader_comp.shaderPath.empty()) {
        shader_comp.shader = std::make_shared<sf::Shader>();
        if (!shader_comp.shader->loadFromFile(shader_comp.shaderPath,
            sf::Shader::Type::Fragment)) {
            std::cerr << "ERROR: Failed to load shader from "
                << shader_comp.shaderPath << "\n";
            shader_comp.shader = nullptr;
        } else {
            shader_comp.shader->setUniform("texture",
                sf::Shader::CurrentTexture);
            for (auto& [name, value] : shader_comp.uniforms_float)
                shader_comp.shader->setUniform(name, value);
            shader_comp.isLoaded = true;
        }
    }
}

/**
 * @brief System that initializes all Shader components that are not loaded.
 *
 * Iterates over the shader components and calls `InitializeShader` for
 * components that have a `shaderPath` and are not yet loaded.
 *
 * @param reg Engine registry (unused)
 * @param shaders Sparse array of Shader components
 */
void InitializeShaderSystem(Eng::registry &reg,
Eng::sparse_array<Com::Shader> &shaders) {
    std::vector<int> draw_order;

    for (auto &&[i, shader] : make_indexed_zipper(shaders)) {
        if (!shader.isLoaded && !shader.shaderPath.empty())
            InitializeShader(shader);
    }
}
}  // namespace Rtype::Client
