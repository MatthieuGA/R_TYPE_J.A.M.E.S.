#include <gtest/gtest.h>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/RenderComponent.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

using Rtype::Client::InitializeShaderSystem;

TEST(ShaderSystem, LoadsShaderFromAssets) {
    Eng::registry reg;

    Eng::sparse_array<Com::Shader> shaders;

    // Use a non-existent shader file to test failure behavior
    Com::Shader shade("error.frag");
    shaders.insert_at(0, shade);

    // Initially not loaded
    EXPECT_FALSE(shaders[0]->isLoaded);

    // Run the initialize system which should attempt to load the shader
    InitializeShaderSystem(reg, shaders);

    // After loading attempt, isLoaded is set to true to prevent retry spam
    // But the shader pointer should be nullptr since the file doesn't exist
    EXPECT_TRUE(shaders[0]->isLoaded);
    EXPECT_EQ(shaders[0]->shader, nullptr);
}
