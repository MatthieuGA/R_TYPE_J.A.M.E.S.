# Plugin System Troubleshooting

Common issues and solutions when working with the R-TYPE J.A.M.E.S. plugin system.

## Build Issues

### Error: "undefined symbol: entryPoint"

**Symptom:**
```
[DLLoader] Error: Failed to get symbol 'create': lib/my_plugin_module.so: undefined symbol: entryPoint.
```

**Cause:** Missing or incorrectly declared entry point function.

**Solution:**

Ensure your plugin has the correct entry point:

```cpp
// ✅ Correct - Audio Plugin
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
        return std::make_shared<MyAudioModule>();
    }
}

// ✅ Correct - Video Plugin
extern "C" {
    std::shared_ptr<Engine::Graphics::IVideoModule> entryPoint() {
        return std::make_shared<MyVideoModule>();
    }
}

// ❌ Wrong - missing extern "C"
std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
    return std::make_shared<MyAudioModule>();
}

// ❌ Wrong - name mangling
namespace MyNamespace {
    extern "C" {
        std::shared_ptr<IAudioModule> entryPoint() { ... }
    }
}
```

**Verification:**

Check symbols in your plugin:

```bash
# Linux
nm -D lib/my_plugin_module.so | grep entryPoint

# Should show:
# 0000000000001234 T entryPoint

# Windows
dumpbin /EXPORTS my_plugin_module.dll
```

---

### Error: "cannot open shared object file: No such file or directory"

**Symptom:**
```
[DLLoader] Error: Failed to load library: lib/my_plugin_module.so: cannot open shared object file: No such file or directory.
```

**Cause:** Incorrect path or plugin not built.

**Solution:**

1. **Verify plugin exists:**
```bash
ls -la build/lib/
# Should show your_module.so (e.g., sfml_audio_module.so, sfml_video_module.so)
```

2. **Check path is relative to executable:**
```bash
# If client is in build/client/
# Plugin is in build/lib/
# Use relative path: "../lib/my_plugin_module.so"
```

3. **Use absolute path for testing:**
```cpp
std::filesystem::path plugin_path = 
    std::filesystem::absolute("build/lib/my_plugin_module.so");
loader.open(plugin_path.string());
```

4. **Verify plugin was built:**
```bash
cmake --build build --target my_plugin_module -j$(nproc)
```

---

### Error: "error while loading shared libraries: libXXX.so: cannot open shared object file"

**Symptom:**
```
error while loading shared libraries: libsfml-audio.so.2.6: cannot open shared object file: No such file or directory
```

**Cause:** Plugin's dependencies not found at runtime.

**Solution:**

1. **Check plugin dependencies:**
```bash
# Linux
ldd build/lib/my_audio_module.so

# Look for "not found" entries
```

2. **Set LD_LIBRARY_PATH:**
```bash
export LD_LIBRARY_PATH=/path/to/libs:$LD_LIBRARY_PATH
```

3. **Install missing libraries:**
```bash
# vcpkg packages
cmake --build build  # Reinstalls dependencies

# System packages
sudo apt install libsfml-dev  # Ubuntu/Debian
sudo dnf install SFML-devel   # Fedora
```

4. **Use RPATH in CMake:**
```cmake
set_target_properties(my_audio_module PROPERTIES
    INSTALL_RPATH "$ORIGIN/../lib"
    BUILD_WITH_INSTALL_RPATH TRUE
)
```

---

### Error: "undefined reference to interface methods"

**Symptom:**
```
undefined reference to `Engine::Audio::IAudioModule::~IAudioModule()'
undefined reference to `Engine::Graphics::IVideoModule::~IVideoModule()'
```

**Cause:** Interface header not properly included or linked.

**Solution:**

1. **Include interface header:**
```cpp
// Audio plugin
#include <audio/IAudioModule.hpp>  // Note: angle brackets, not quotes

// Video plugin
#include <graphics/IVideoModule.hpp>
```

2. **Add engine include path in CMakeLists.txt:**
```cmake
target_include_directories(my_plugin_module
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/include  # Add this
)
```

3. **Ensure C++23:**
```cmake
target_compile_features(my_audio_module PRIVATE cxx_std_23)
```

---

### Error: "multiple definition of entryPoint"

**Symptom:**
```
multiple definition of `entryPoint'
```

**Cause:** Entry point defined in multiple files or included in header.

**Solution:**

1. **Define in .cpp only:**
```cpp
// ✅ MyAudioModule.cpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
        return std::make_shared<MyAudioModule>();
    }
}

// ❌ MyAudioModule.hpp - Don't define here
```

2. **Declare in header if needed:**
```cpp
// MyAudioModule.hpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint();  // Declaration only
}
```

---

## Runtime Issues

### Plugin loads but crashes immediately

**Symptom:**
Application crashes when calling `getInstance()` or `Initialize()`.

**Cause:** ABI incompatibility or constructor exception.

**Solution:**

1. **Check compiler compatibility:**
```bash
# Plugin and main app must use same compiler
g++ --version  # Check version matches
```

2. **Verify C++ standard:**
```cmake
# Both must use same standard
target_compile_features(r-type_client PRIVATE cxx_std_23)
target_compile_features(my_audio_module PRIVATE cxx_std_23)
```

3. **Add try-catch in entry point:**
```cpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
        try {
            return std::make_shared<MyAudioModule>();
        } catch (const std::exception &e) {
            std::cerr << "Plugin creation failed: " << e.what() << std::endl;
            return nullptr;
        }
    }
}
```

4. **Check for exceptions in constructor:**
```cpp
class MyAudioModule : public IAudioModule {
public:
    MyAudioModule() {
        // ❌ Don't throw here - DLLoader can't catch
        // if (!Init()) throw std::runtime_error("Failed");
        
        // ✅ Defer to Initialize()
    }
    
    bool Initialize() override {
        // ✅ Return false on error
        if (!Init()) return false;
        return true;
    }
};
```

---

### Initialize() returns false

**Symptom:**
```cpp
if (!module->Initialize()) {
    // This branch is taken
}
```

**Cause:** Device or system initialization failed.

**Solution:**

1. **Check device availability (Audio):**
```cpp
bool MyAudioModule::Initialize() {
    auto devices = GetAudioDevices();
    if (devices.empty()) {
        std::cerr << "No audio devices found" << std::endl;
        return false;
    }
    
    std::cout << "Available audio devices:" << std::endl;
    for (const auto &device : devices) {
        std::cout << "  - " << device << std::endl;
    }
    
    return true;
}
```

2. **Check display availability (Video):**
```cpp
bool MyVideoModule::Initialize() {
    if (!CreateWindow(800, 600, "Test")) {
        std::cerr << "Failed to create window" << std::endl;
        return false;
    }
    return true;
}
```

3. **Test without hardware (headless mode):**
```cpp
// Add dummy mode for headless testing
bool MyPluginModule::Initialize() {
    if (std::getenv("HEADLESS")) {
        std::cout << "Running in headless mode" << std::endl;
        return true;
    }
    return InitializeDevice();
}
```

4. **Check permissions (Linux):**
```bash
# Audio: User must be in 'audio' group
groups $USER
sudo usermod -a -G audio $USER

# Video: User must have access to display
echo $DISPLAY  # Should show :0 or similar
```

---

### Assets fail to load

**Symptom:**
```cpp
// Audio plugin
audioModule->LoadSound("laser", "assets/sounds/laser.ogg");  // Returns false

// Video plugin
videoModule->LoadTexture("ship", "assets/sprites/ship.png");  // Returns false
```

**Cause:** File not found or unsupported format.

**Solution:**

1. **Verify file exists:**
```cpp
bool MyPluginModule::LoadAsset(const std::string &id, const std::string &path) {
    if (!std::filesystem::exists(path)) {
        std::cerr << "File not found: " << path << std::endl;
        std::cerr << "Current directory: " 
                  << std::filesystem::current_path() << std::endl;
        return false;
    }
    // Load file...
}
```

2. **Use absolute paths:**
```cpp
std::string assets_dir = std::filesystem::absolute("assets").string();

// Audio
module->LoadSound("laser", assets_dir + "/sounds/laser.ogg");

// Video
module->LoadTexture("ship", assets_dir + "/sprites/ship.png");
```

3. **Check file format support:**
```cpp
// Audio formats
std::vector<std::string> audio_formats = {".ogg", ".wav", ".mp3", ".flac"};

// Graphics formats
std::vector<std::string> image_formats = {".png", ".jpg", ".bmp", ".tga"};

auto ext = std::filesystem::path(path).extension().string();
if (std::find(supported.begin(), supported.end(), ext) == supported.end()) {
    std::cerr << "Unsupported format: " << ext << std::endl;
    return false;
}
```

---

### No sound output

**Symptom:**
Everything loads successfully but no audio plays.

**Cause:** Volume set to 0, muted, or playback not triggered.

**Solution:**

1. **Check volume settings:**
```cpp
module->SetSfxVolume(1.0f);      // Full volume
module->SetMusicVolume(1.0f);
module->MuteSfx(false);           // Unmute
module->MuteMusic(false);
```

2. **Verify playback is called:**
```cpp
void MyAudioModule::Play(const PlaybackRequest &request) {
    std::cout << "Playing: " << request.id 
              << " volume=" << request.volume 
              << " loop=" << request.loop << std::endl;
    
    // Actual playback code...
}
```

3. **Check audio device output:**
```bash
# Linux: Test audio system
speaker-test -t sine -f 440 -c 2

# Check volume isn't muted
alsamixer
```

4. **Test with simple example:**
```cpp
// Minimal test
auto module = loader.getInstance("entryPoint");
module->Initialize();
module->LoadSound("test", "test.ogg");
module->Play({.id = "test", .volume = 1.0f});
std::this_thread::sleep_for(std::chrono::seconds(2));  // Let it play
module->Shutdown();
```

---

## Testing Issues

### Tests pass individually but fail in CTest

**Symptom:**
```bash
./tests/engine_tests --gtest_filter="AudioPlugin*"  # ✅ Pass
ctest --test-dir build                               # ❌ Fail
```

**Cause:** CTest runs from different working directory.

**Solution:**

1. **Use absolute paths in tests:**
```cpp
TEST_F(AudioPluginTest, LoadPlugin) {
    // ❌ Relative path
    // std::string path = "lib/sfml_audio_module.so";
    
    // ✅ Absolute path
    std::string path = std::filesystem::absolute("lib/sfml_audio_module.so").string();
    
    loader.open(path);
}
```

2. **Set test working directory in CMake:**
```cmake
add_test(NAME AudioPluginTest COMMAND engine_tests --gtest_filter="AudioPlugin*")
set_tests_properties(AudioPluginTest PROPERTIES
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
```

3. **Copy plugin to test directory:**
```cmake
add_custom_command(TARGET engine_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        $<TARGET_FILE:sfml_audio_module>
        $<TARGET_FILE_DIR:engine_tests>
)
```

---

## Performance Issues

### Plugin loads slowly

**Symptom:**
Plugin takes several seconds to load.

**Cause:** Large dependencies or slow disk I/O.

**Solution:**

1. **Profile loading time:**
```cpp
auto start = std::chrono::high_resolution_clock::now();
loader.open(plugin_path);
auto end = std::chrono::high_resolution_clock::now();
auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
std::cout << "Load time: " << duration.count() << "ms" << std::endl;
```

2. **Check plugin size:**
```bash
ls -lh build/lib/my_audio_module.so

# Large size (&gt;100MB) indicates excessive dependencies
# Consider static linking some libraries
```

3. **Use lazy binding:**
```cpp
// DLLoader already uses RTLD_LAZY on Linux
// Symbols resolved on first use, not at load time
```

4. **Preload plugin:**
```cpp
// Load plugin during splash screen or loading screen
std::thread plugin_loader([&]() {
    loader.open(plugin_path);
    module = loader.getInstance("entryPoint");
    module->Initialize();
});

// Show loading screen...
plugin_loader.join();
```

---

### Memory leaks detected

**Symptom:**
Valgrind or sanitizers report leaks related to plugin.

**Cause:** Improper cleanup in `Shutdown()`.

**Solution:**

1. **Clear all containers:**
```cpp
void MyAudioModule::Shutdown() {
    // Release all resources
    sounds_.clear();
    music_tracks_.clear();
    current_music_ = nullptr;
    
    // Close device
    CloseAudioDevice();
}
```

2. **Use smart pointers:**
```cpp
// ✅ Automatic cleanup
std::map<std::string, std::unique_ptr<Sound>> sounds_;

// ❌ Manual cleanup required
std::map<std::string, Sound*> sounds_;
```

3. **Run with leak detection:**
```bash
# Valgrind
valgrind --leak-check=full ./build/client/r-type_client

# AddressSanitizer
cmake -DCMAKE_CXX_FLAGS="-fsanitize=address" ..
cmake --build . && ./build/client/r-type_client
```

---

## Debugging Tips

### Enable verbose logging

```cpp
#define PLUGIN_DEBUG 1

#ifdef PLUGIN_DEBUG
    #define PLUGIN_LOG(module, msg) std::cout << "[" << module << "] " << msg << std::endl
#else
    #define PLUGIN_LOG(module, msg)
#endif

// Audio plugin
void MyAudioModule::LoadSound(const std::string &id, const std::string &path) {
    PLUGIN_LOG("AudioPlugin", "Loading sound: " << id << " from " << path);
    // ...
}

// Video plugin
void MyVideoModule::LoadTexture(const std::string &id, const std::string &path) {
    PLUGIN_LOG("VideoPlugin", "Loading texture: " << id << " from " << path);
    // ...
}
```

### Use debugger with plugins

```bash
# GDB
gdb ./build/client/r-type_client
(gdb) break main
(gdb) run
(gdb) break MyAudioModule::Initialize
(gdb) continue
```

### Check loaded libraries at runtime

```cpp
#include <link.h>  // Linux

int callback(struct dl_phdr_info *info, size_t size, void *data) {
    std::cout << "Loaded: " << info->dlpi_name << std::endl;
    return 0;
}

// Call in main():
dl_iterate_phdr(callback, nullptr);
```

---

## Getting Help

If you're still stuck:

1. **Check examples:**
   - Audio: Review `client/plugins/audio/sfml/` for reference implementation
   - Video: Review `client/plugins/video/sfml/` for reference implementation
   - Tests: See `tests/test_audio_plugin.cpp` and `tests/test_video_plugin.cpp`

2. **Consult documentation:**
   - [Audio Plugin Guide](./audio-plugin-guide.md)
   - [Video Plugin Guide](./video-plugin-guide.md)
   - [Architecture Overview](./architecture.md)
   - [API Reference](./api-reference.md)

3. **Search existing issues:**
   - GitHub Issues: https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./issues

4. **Ask for help:**
   - Open a new GitHub issue with:
     - Full error message
     - Plugin CMakeLists.txt
     - Output of `ldd` (Linux) or Dependency Walker (Windows)
     - Steps to reproduce
