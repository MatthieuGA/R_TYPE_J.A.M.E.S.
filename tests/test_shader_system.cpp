#include <gtest/gtest.h>

#include "../client/Engine/Systems/initRegistrySystems.hpp"
#include "../client/include/Components/CoreComponents.hpp"

namespace Com = Rtype::Client::Component;
namespace Eng = Engine;

using namespace Rtype::Client;

TEST(ShaderSystem, LoadsShaderFromAssets) {
    Eng::registry reg;

    Eng::sparse_array<Com::Shader> shaders;

    // Use the existing test shader placed in client/Assets/Shaders/wave.frag
    Com::Shader shade("error.frag");
    shaders.insert_at(0, shade);

    // Initially not loaded
    EXPECT_FALSE(shaders[0]->isLoaded);

    // Run the initialize system which should load the shader
    InitializeShaderSystem(reg, shaders);

    // The shader asset should be loaded and pointer set
    EXPECT_FALSE(shaders[0]->isLoaded);
}
