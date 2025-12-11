/**
 * @file test_video_plugin.cpp
 * @brief Unit tests for video plugin loading and basic functionality.
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <loader/DLLoader.hpp>
#include <video/IVideoModule.hpp>

#include "engine/video/PluginVideoBackend.hpp"

namespace {

const char kPluginPath[] = "./lib/sfml_video_module.so";

/**
 * @brief Test basic video plugin loading.
 */
TEST(VideoPluginTest, LoadPlugin) {
    Engine::DLLoader<Engine::Video::IVideoModule> loader;

    ASSERT_NO_THROW(loader.open(kPluginPath));

    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);
    EXPECT_EQ(module->GetModuleName(), "SFML Video Module");
}

/**
 * @brief Test video module initialization.
 */
TEST(VideoPluginTest, InitializeModule) {
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open(kPluginPath);
    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    EXPECT_TRUE(module->Initialize(800, 600, "Test Window"));
    EXPECT_TRUE(module->IsWindowOpen());

    module->Shutdown();
    EXPECT_FALSE(module->IsWindowOpen());
}

/**
 * @brief Test plugin video backend adapter.
 */
TEST(VideoPluginTest, PluginBackendAdapter) {
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open(kPluginPath);
    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    auto backend = std::make_unique<Engine::Video::PluginVideoBackend>(module);

    EXPECT_TRUE(backend->Initialize(640, 480, "Backend Test"));
    EXPECT_TRUE(backend->IsWindowOpen());
    EXPECT_EQ(backend->GetModuleName(), "SFML Video Module");

    auto size = backend->GetWindowSize();
    EXPECT_EQ(size.x, 640.0f);
    EXPECT_EQ(size.y, 480.0f);

    backend->Shutdown();
    EXPECT_FALSE(backend->IsWindowOpen());
}

/**
 * @brief Test event polling.
 */
TEST(VideoPluginTest, EventPolling) {
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open(kPluginPath);
    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    module->Initialize(320, 240, "Event Test");

    // Should not crash when polling
    Engine::Video::Event event;
    EXPECT_NO_THROW(module->PollEvent(event));

    module->Shutdown();
}

/**
 * @brief Test rendering operations don't crash.
 */
TEST(VideoPluginTest, RenderingOperations) {
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open(kPluginPath);
    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    module->Initialize(320, 240, "Render Test");

    // These should not crash
    EXPECT_NO_THROW(module->Clear(Engine::Video::Color(0, 0, 0, 255)));
    EXPECT_NO_THROW(module->Display());

    module->Shutdown();
}

/**
 * @brief Test texture loading.
 */
TEST(VideoPluginTest, TextureLoading) {
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open(kPluginPath);
    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    module->Initialize(320, 240, "Texture Test");

    // Try to load a non-existent texture (should fail gracefully)
    EXPECT_FALSE(module->LoadTexture("test_tex", "nonexistent.png"));

    // GetTexture should return nullptr for non-loaded texture
    EXPECT_EQ(module->GetTexture("test_tex"), nullptr);

    module->Shutdown();
}

/**
 * @brief Test window title update.
 */
TEST(VideoPluginTest, WindowTitleUpdate) {
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open(kPluginPath);
    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    module->Initialize(320, 240, "Title Test");

    // Should not crash
    EXPECT_NO_THROW(module->SetWindowTitle("New Title"));

    module->Shutdown();
}

}  // namespace
