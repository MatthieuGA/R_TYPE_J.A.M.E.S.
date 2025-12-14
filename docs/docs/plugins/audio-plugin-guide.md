# Audio Plugin Development Guide

This comprehensive guide walks you through creating a custom audio plugin for the R-TYPE J.A.M.E.S. engine from scratch.

## Prerequisites

Before starting, ensure you have:

- C++23 compatible compiler (GCC 12+, Clang 15+, MSVC 2022+)
- CMake 3.23 or higher
- Basic understanding of C++ virtual interfaces
- Familiarity with dynamic library concepts

## What You'll Build

By the end of this guide, you'll have created a complete audio plugin that:
- Loads dynamically at runtime
- Implements the `IAudioModule` interface
- Compiles to a shared library (`.so` or `.dll`)
- Integrates seamlessly with the engine

## Step 1: Understanding the Interface

The `IAudioModule` interface defines the contract for all audio plugins:

```cpp
namespace Engine::Audio {

struct PlaybackRequest {
    std::string id;
    float volume = 1.0f;
    bool loop = false;
    
    enum class Category {
        SFX,
        MUSIC
    } category = Category::SFX;
};

class IAudioModule {
 public:
    virtual ~IAudioModule() = default;

    // Lifecycle
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float delta_time) = 0;

    // Asset Loading
    virtual bool LoadSound(const std::string &id, const std::string &path) = 0;
    virtual bool LoadMusic(const std::string &id, const std::string &path) = 0;

    // Playback Control
    virtual void Play(const PlaybackRequest &request) = 0;
    virtual void StopMusic() = 0;

    // Volume Control
    virtual void SetSfxVolume(float volume) = 0;
    virtual void SetMusicVolume(float volume) = 0;
    virtual void MuteSfx(bool mute) = 0;
    virtual void MuteMusic(bool mute) = 0;

    // Identification
    virtual std::string GetModuleName() const = 0;
};

}  // namespace Engine::Audio
```

**Key Points:**
- All methods are pure virtual - you must implement every one
- The interface is in `engine/include/audio/IAudioModule.hpp`
- `PlaybackRequest` encapsulates playback parameters

## Step 2: Create the Plugin Directory Structure

Create your plugin in the appropriate location:

```bash
mkdir -p client/plugins/audio/my_audio_plugin
cd client/plugins/audio/my_audio_plugin
```

You'll create three files:
1. `MyAudioModule.hpp` - Header declaration
2. `MyAudioModule.cpp` - Implementation
3. `CMakeLists.txt` - Build configuration

## Step 3: Implement the Header File

**File:** `client/plugins/audio/my_audio_plugin/MyAudioModule.hpp`

```cpp
#pragma once
#include <audio/IAudioModule.hpp>
#include <map>
#include <memory>
#include <string>

// Include your audio library headers here
// #include <YourAudioLibrary.hpp>

namespace Engine::Audio {

/**
 * @brief Custom audio module implementation.
 * 
 * Replace this with your actual audio backend (OpenAL, miniaudio, etc.)
 */
class MyAudioModule : public IAudioModule {
 public:
    MyAudioModule() = default;
    ~MyAudioModule() override = default;

    // Lifecycle methods
    bool Initialize() override;
    void Shutdown() override;
    void Update(float delta_time) override;

    // Asset loading
    bool LoadSound(const std::string &id, const std::string &path) override;
    bool LoadMusic(const std::string &id, const std::string &path) override;

    // Playback control
    void Play(const PlaybackRequest &request) override;
    void StopMusic() override;

    // Volume control
    void SetSfxVolume(float volume) override;
    void SetMusicVolume(float volume) override;
    void MuteSfx(bool mute) override;
    void MuteMusic(bool mute) override;

    // Identification
    std::string GetModuleName() const override;

 private:
    // Your internal state here
    std::map<std::string, /* YourSoundType */void*> sounds_;
    std::map<std::string, /* YourMusicType */void*> music_tracks_;
    
    float sfx_volume_ = 100.0f;
    float music_volume_ = 100.0f;
    bool sfx_muted_ = false;
    bool music_muted_ = false;
};

}  // namespace Engine::Audio

// C-style entry point (required for dynamic loading)
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint();
}
```

**Critical Details:**

1. **Namespace:** Must be `Engine::Audio`
2. **Entry Point:** The `extern "C"` block prevents name mangling
3. **Return Type:** Must return `std::shared_ptr<IAudioModule>`
4. **Entry Point Name:** Default is `entryPoint` (configurable in DLLoader)

## Step 4: Implement the Source File

**File:** `client/plugins/audio/my_audio_plugin/MyAudioModule.cpp`

```cpp
#include "MyAudioModule.hpp"  // NOLINT(build/include_subdir)

#include <iostream>
#include <memory>
#include <string>

namespace Engine::Audio {

bool MyAudioModule::Initialize() {
    std::cout << "[MyAudioModule] Initializing..." << std::endl;
    
    // Initialize your audio backend here
    // Example: Open audio device, set up audio context, etc.
    
    std::cout << "[MyAudioModule] Initialized successfully" << std::endl;
    return true;
}

void MyAudioModule::Shutdown() {
    std::cout << "[MyAudioModule] Shutting down..." << std::endl;
    
    // Clean up resources
    sounds_.clear();
    music_tracks_.clear();
    
    // Close audio device, destroy context, etc.
    
    std::cout << "[MyAudioModule] Shut down complete" << std::endl;
}

void MyAudioModule::Update(float delta_time) {
    // Update streaming audio, check for finished sounds, etc.
    // Called every frame by the engine
}

bool MyAudioModule::LoadSound(const std::string &id, const std::string &path) {
    std::cout << "[MyAudioModule] Loading sound: " << id 
              << " from " << path << std::endl;
    
    // Load the sound file
    // Example:
    // auto sound = LoadAudioFile(path);
    // if (!sound) {
    //     std::cerr << "[MyAudioModule] Failed to load: " << path << std::endl;
    //     return false;
    // }
    // sounds_[id] = sound;
    
    return true;
}

bool MyAudioModule::LoadMusic(const std::string &id, const std::string &path) {
    std::cout << "[MyAudioModule] Loading music: " << id 
              << " from " << path << std::endl;
    
    // Load the music file (typically streamed, not fully buffered)
    // Example:
    // auto music = OpenMusicStream(path);
    // if (!music) {
    //     std::cerr << "[MyAudioModule] Failed to load: " << path << std::endl;
    //     return false;
    // }
    // music_tracks_[id] = music;
    
    return true;
}

void MyAudioModule::Play(const PlaybackRequest &request) {
    if (request.category == PlaybackRequest::Category::MUSIC) {
        // Play music
        auto it = music_tracks_.find(request.id);
        if (it == music_tracks_.end()) {
            std::cerr << "[MyAudioModule] Music not found: " << request.id << std::endl;
            return;
        }
        
        // Stop current music, start new one
        // SetMusicVolume(request.volume * music_volume_ * (music_muted_ ? 0.0f : 1.0f));
        // SetMusicLoop(request.loop);
        // PlayMusic(it->second);
        
    } else {
        // Play sound effect
        auto it = sounds_.find(request.id);
        if (it == sounds_.end()) {
            std::cerr << "[MyAudioModule] Sound not found: " << request.id << std::endl;
            return;
        }
        
        // Play the sound
        // SetSoundVolume(request.volume * sfx_volume_ * (sfx_muted_ ? 0.0f : 1.0f));
        // SetSoundLoop(request.loop);
        // PlaySound(it->second);
    }
}

void MyAudioModule::StopMusic() {
    // Stop currently playing music
    // StopMusicPlayback();
}

void MyAudioModule::SetSfxVolume(float volume) {
    sfx_volume_ = volume * 100.0f;
    
    // Update volume for all active sound effects
    // for (auto &sound : sounds_) {
    //     UpdateSoundVolume(sound.second, sfx_volume_);
    // }
}

void MyAudioModule::SetMusicVolume(float volume) {
    music_volume_ = volume * 100.0f;
    
    // Update music volume
    // UpdateMusicVolume(music_volume_);
}

void MyAudioModule::MuteSfx(bool mute) {
    sfx_muted_ = mute;
    SetSfxVolume(sfx_volume_ / 100.0f);
}

void MyAudioModule::MuteMusic(bool mute) {
    music_muted_ = mute;
    SetMusicVolume(music_volume_ / 100.0f);
}

std::string MyAudioModule::GetModuleName() const {
    return "My Custom Audio Module";
}

}  // namespace Engine::Audio

// C-style entry point implementation
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
        return std::make_shared<Engine::Audio::MyAudioModule>();
    }
}
```

**Implementation Notes:**

1. **Error Handling:** Always check for errors and log meaningful messages
2. **Volume Calculations:** Combine request volume, category volume, and mute state
3. **Resource Management:** Clean up in `Shutdown()`, not in destructor
4. **Entry Point:** Must return a valid `shared_ptr` - the engine owns the lifetime

## Step 5: Configure CMake

**File:** `client/plugins/audio/my_audio_plugin/CMakeLists.txt`

```cmake
# My Audio Plugin
cmake_minimum_required(VERSION 3.23)

# Define the plugin as a shared library
add_library(my_audio_module SHARED
    MyAudioModule.cpp
    MyAudioModule.hpp
)

# Link your audio library
# Example for OpenAL:
# find_package(OpenAL REQUIRED)
# target_link_libraries(my_audio_module PRIVATE OpenAL::OpenAL)

# Or for a custom library:
# target_link_libraries(my_audio_module PRIVATE my_audio_lib)

# Include directories
target_include_directories(my_audio_module
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/client/plugins/audio/my_audio_plugin
)

# Set C++ standard
target_compile_features(my_audio_module PRIVATE cxx_std_23)

# Remove "lib" prefix and set output directory
set_target_properties(my_audio_module PROPERTIES
    PREFIX ""
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Install the plugin
install(TARGETS my_audio_module
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION lib
)
```

**Key CMake Settings:**

- `SHARED` - Creates a shared library (`.so`/`.dll`)
- `PREFIX ""` - Removes "lib" prefix (output: `my_audio_module.so` not `libmy_audio_module.so`)
- `LIBRARY_OUTPUT_DIRECTORY` - Places plugin in `build/lib/`
- `cxx_std_23` - Ensures C++23 compliance

## Step 6: Register Plugin in Parent CMake

Edit `client/CMakeLists.txt` to add your plugin subdirectory:

```cmake
# Build audio plugins
add_subdirectory(plugins/audio/sfml)
add_subdirectory(plugins/audio/my_audio_plugin)  # Add this line
```

## Step 7: Build the Plugin

```bash
cd build
cmake --build . --target my_audio_module -j$(nproc)
```

Verify the plugin was created:

```bash
ls -lh build/lib/my_audio_module.so
```

## Step 8: Load and Use the Plugin

Update `client/main.cpp` to load your plugin:

```cpp
#include <loader/DLLoader.hpp>
#include <audio/IAudioModule.hpp>

// In main():
std::string plugin_path = "../lib/my_audio_module.so";
Engine::DLLoader<Engine::Audio::IAudioModule> audio_loader;
audio_loader.open(plugin_path);
auto audio_module = audio_loader.getInstance("entryPoint");

if (!audio_module) {
    throw std::runtime_error("Failed to load audio plugin");
}

std::cout << "Loaded: " << audio_module->GetModuleName() << std::endl;

// Use the adapter to bridge to AudioManager
auto audio_backend = std::make_unique<Audio::PluginAudioBackend>(audio_module);
Audio::AudioManager audio_manager(std::move(audio_backend));
```

## Step 9: Testing Your Plugin

Create unit tests in `tests/test_my_audio_plugin.cpp`:

```cpp
#include <gtest/gtest.h>
#include <loader/DLLoader.hpp>
#include <audio/IAudioModule.hpp>
#include <memory>
#include <string>

class MyAudioPluginTest : public ::testing::Test {
protected:
    void SetUp() override {
        plugin_path_ = "lib/my_audio_module.so";
    }
    
    std::string plugin_path_;
};

TEST_F(MyAudioPluginTest, LoadPlugin) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;
    ASSERT_NO_THROW(loader.open(plugin_path_));
}

TEST_F(MyAudioPluginTest, GetEntryPoint) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;
    loader.open(plugin_path_);
    
    auto module = loader.getInstance("entryPoint");
    ASSERT_NE(module, nullptr);
}

TEST_F(MyAudioPluginTest, Initialize) {
    Engine::DLLoader<Engine::Audio::IAudioModule> loader;
    loader.open(plugin_path_);
    
    auto module = loader.getInstance("entryPoint");
    EXPECT_TRUE(module->Initialize());
    module->Shutdown();
}
```

Run tests:

```bash
cd build
./tests/engine_tests --gtest_filter="MyAudioPlugin*"
```

## Common Pitfalls and Solutions

### Issue: "undefined symbol: entryPoint"

**Cause:** Missing or incorrect `extern "C"` declaration

**Solution:** Ensure entry point is declared exactly as:
```cpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
        return std::make_shared<MyAudioModule>();
    }
}
```

### Issue: Build succeeds but crashes on load

**Cause:** ABI incompatibility or missing dependencies

**Solution:** 
- Verify plugin and engine use same C++ standard
- Check that all linked libraries are available at runtime
- Use `ldd` (Linux) or `Dependency Walker` (Windows) to verify dependencies

### Issue: "cannot open shared object file"

**Cause:** Plugin path is incorrect relative to executable

**Solution:** 
- Use absolute paths for testing
- Or adjust relative path based on executable location
- Client runs from `build/client/`, plugin is in `build/lib/`
- Use `"../lib/my_audio_module.so"` from client

## Advanced Topics

### Custom Entry Point Names

If you need a different entry point name:

```cpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> createMyPlugin() {
        return std::make_shared<MyAudioModule>();
    }
}
```

Load with custom name:

```cpp
auto module = audio_loader.getInstance("createMyPlugin");
```

### Plugin Versioning

Add version checking to ensure compatibility:

```cpp
extern "C" {
    int getPluginVersion() { return 1; }
    
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
        return std::make_shared<MyAudioModule>();
    }
}
```

### Hot Reloading

The plugin system supports hot reloading:

```cpp
// Unload old plugin
audio_loader.close();

// Rebuild plugin...

// Load new version
audio_loader.open(plugin_path);
auto new_module = audio_loader.getInstance("entryPoint");
```

## Best Practices Checklist

- [ ] All interface methods implemented
- [ ] Entry point uses `extern "C"`
- [ ] Returns `std::shared_ptr`
- [ ] CMakeLists.txt configured correctly
- [ ] Plugin outputs to `build/lib/`
- [ ] Unit tests created and passing
- [ ] Error messages are clear and helpful
- [ ] Resources cleaned up in `Shutdown()`
- [ ] Documentation added to header
- [ ] Code follows Google C++ Style Guide

## Next Steps

- Review the SFML Audio Plugin reference implementation in the project repository (`client/plugins/audio/sfml/`)
- Read the [Plugin Architecture](./architecture.md) deep dive
- Check the [API Reference](./api-reference.md) for detailed interface documentation
- Learn about [Troubleshooting](./troubleshooting.md) common issues

## Example: SFML Audio Plugin

For a complete, production-ready example, see the SFML audio plugin in the project repository:

- **Location:** `client/plugins/audio/sfml/`
- **Header:** `SFMLAudioModule.hpp`
- **Implementation:** `SFMLAudioModule.cpp`
- **Build Configuration:** `CMakeLists.txt`

This serves as the reference implementation for the audio plugin system.
