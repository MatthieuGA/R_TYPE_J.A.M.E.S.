#include <gtest/gtest.h>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/RenderComponent.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

using Rtype::Client::InitializeShaderSystem;

TEST(ShaderSystem, LoadsShaderFromAssets) {
    Eng::registry reg;

    Eng::sparse_array<Com::Shader> shaders;

    // Create a shader component with a path
    Com::Shader shade("wave.frag");
    shaders.insert_at(0, shade);

    // Initially not loaded
    EXPECT_FALSE(shaders[0]->isLoaded);

    // Run the initialize system which marks shaders as ready for backend
    // loading
    InitializeShaderSystem(reg, shaders);

    // After initialization, shader should be marked as loaded
    // (Backend will handle actual file loading and error cases)
    EXPECT_TRUE(shaders[0]->isLoaded);
}

TEST(ShaderSystem, SkipsAlreadyLoadedShaders) {
    Eng::registry reg;

    Eng::sparse_array<Com::Shader> shaders;

    // Create a shader that's already marked as loaded
    Com::Shader shade("wave.frag");
    shade.isLoaded = true;  // Already loaded
    shaders.insert_at(0, shade);

    // Mark it as loaded
    EXPECT_TRUE(shaders[0]->isLoaded);

    // Run the initialize system - should not re-initialize already loaded
    // shaders
    InitializeShaderSystem(reg, shaders);

    // Should still be loaded
    EXPECT_TRUE(shaders[0]->isLoaded);
}
