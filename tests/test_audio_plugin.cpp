/**
 * @file test_audio_plugin.cpp
 * @brief Test for dynamically loading the SFML audio plugin.
 */

#include <gtest/gtest.h>

#include <memory>
#include <string>

#include <audio/IAudioModule.hpp>
#include <loader/DLLoader.hpp>

class AudioPluginTest : public ::testing::Test {
 protected:
    void SetUp() override {
        // Plugin path relative to build directory
        plugin_path_ = "lib/sfml_audio_module.so";
    }

    std::string plugin_path_;
};

TEST_F(AudioPluginTest, LoadPlugin) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;

    ASSERT_NO_THROW(loader.open(plugin_path_));
}

TEST_F(AudioPluginTest, GetEntryPoint) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;
    loader.open(plugin_path_);

    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);
}

TEST_F(AudioPluginTest, GetModuleName) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;
    loader.open(plugin_path_);

    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    std::string name = module->GetModuleName();
    EXPECT_EQ(name, "SFML Audio Module");
}

TEST_F(AudioPluginTest, InitializeModule) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;
    loader.open(plugin_path_);

    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    EXPECT_TRUE(module->Initialize());
}

TEST_F(AudioPluginTest, ShutdownModule) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;
    loader.open(plugin_path_);

    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    module->Initialize();
    EXPECT_NO_THROW(module->Shutdown());
}

TEST_F(AudioPluginTest, LoadSound) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;
    loader.open(plugin_path_);

    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);

    module->Initialize();

    // Test loading a non-existent file (should fail gracefully)
    bool result = module->LoadSound("test", "nonexistent.ogg");
    EXPECT_FALSE(result);

    module->Shutdown();
}

TEST_F(AudioPluginTest, MultipleInstances) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader1;
    Engine::DLLoader<Engine::Audio::IAudioModule> loader2;

    loader1.open(plugin_path_);
    loader2.open(plugin_path_);

    auto module1 = loader1.getInstance("entryPoint");
    auto module2 = loader2.getInstance("entryPoint");

    ASSERT_NE(module1, nullptr);
    ASSERT_NE(module2, nullptr);
    EXPECT_NE(module1, module2);  // Different instances
}
