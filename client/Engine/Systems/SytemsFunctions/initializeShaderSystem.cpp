#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>

#include "Engine/Systems/initRegistrySystems.hpp"
#include "Engine/originTool.hpp"

namespace Rtype::Client {

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


void InitializeShaderSystem(Eng::registry &reg,
Eng::sparse_array<Com::Shader> &shaders) {
    std::vector<int> draw_order;

    for (auto &&[i, shader] : make_indexed_zipper(shaders)) {
        if (!shader.isLoaded && !shader.shaderPath.empty())
            InitializeShader(shader);
    }
}
}  // namespace Rtype::Client
