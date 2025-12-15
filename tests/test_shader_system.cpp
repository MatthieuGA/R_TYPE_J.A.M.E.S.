#include <gtest/gtest.h>

#include "engine/systems/InitRegistrySystems.hpp"
#include "include/components/RenderComponent.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

using Rtype::Client::InitializeShaderSystem;

// TODO(plugin-refactor): This test needs GameWorld with rendering_engine mock
// Shader loading now requires the plugin to be initialized
/*
TEST(ShaderSystem, LoadsShaderFromAssets) {
    Eng::registry reg;

    Eng::sparse_array<Com::Shader> shaders;

    // Use the existing test shader placed in client/assets/shaders/wave.frag
    Com::Shader shade("error.frag");
    shaders.insert_at(0, shade);

    // Initially not loaded
    EXPECT_FALSE(shaders[0]->is_loaded);

    // Run the initialize system which should load the shader
    // InitializeShaderSystem now requires GameWorld parameter
    // InitializeShaderSystem(reg, game_world, shaders);

    // The shader asset should not exist, so loading should fail
    EXPECT_FALSE(shaders[0]->is_loaded);
}
*/
