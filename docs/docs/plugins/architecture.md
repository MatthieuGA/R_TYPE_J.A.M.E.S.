# Plugin System Architecture

This document provides an in-depth technical overview of the R-TYPE J.A.M.E.S. plugin system architecture, design decisions, and implementation details.

## Design Philosophy

The plugin system was designed with the following principles:

1. **Type Safety**: Leverage C++ templates for compile-time type checking
2. **RAII**: Automatic resource management via smart pointers and destructors
3. **Interface Segregation**: Clear separation between interface and implementation
4. **Dependency Inversion**: High-level code depends on abstractions, not concrete implementations
5. **Single Responsibility**: Each component has one clear purpose

## Component Hierarchy

### Audio Plugin Example

```
┌─────────────────────────────────────────┐
│         Application (main.cpp)          │
│  - Loads plugin via DLLoader            │
│  - Creates AudioManager with backend    │
└───────────────┬─────────────────────────┘
                │
                ├─────────────────────────┐
                │                         │
        ┌───────▼────────┐        ┌──────▼──────────┐
        │   DLLoader<T>  │        │  AudioManager   │
        │  (Template)    │        │ (Engine API)    │
        └───────┬────────┘        └──────┬──────────┘
                │                        │
                │ getInstance()          │ uses
                │ returns shared_ptr     │
                │                        │
        ┌───────▼────────┐        ┌──────▼──────────────┐
        │ IAudioModule   │◄───────┤ PluginAudioBackend  │
        │  (Interface)   │        │    (Adapter)        │
        └───────△───────┘        └─────────────────────┘
                │
                │ implements
                │
        ┌───────┴────────┐
        │ SFMLAudioModule│
        │   (.so file)   │
        └────────────────┘
```

### Video Plugin Example

```
┌─────────────────────────────────────────┐
│         Application (main.cpp)          │
│  - Loads plugin via DLLoader            │
│  - Uses IVideoModule for rendering      │
└───────────────┬─────────────────────────┘
                │
                ├─────────────────────────┐
                │                         │
        ┌───────▼────────┐        ┌──────▼──────────┐
        │   DLLoader<T>  │        │   GameWorld     │
        │  (Template)    │        │   (Client)      │
        └───────┬────────┘        └──────┬──────────┘
                │                        │
                │ getInstance()          │ uses
                │ returns shared_ptr     │
                │                        │
        ┌───────▼────────┐               │
        │ IVideoModule   │◄──────────────┘
        │  (Interface)   │
        └───────△───────┘
                │
                │ implements
                │
        ┌───────┴────────┐
        │ SFMLVideoModule│
        │   (.so file)   │
        └────────────────┘
```

## DLLoader: Dynamic Library Loader

### Design Rationale

The `DLLoader` is a generic template class that abstracts platform-specific dynamic loading APIs. This design allows:

- **Portability**: Single API works on Linux (dlopen) and Windows (LoadLibrary)
- **Type Safety**: Template parameter enforces interface at compile time
- **Reusability**: Can load any type of plugin, not just audio

### Implementation Details

**Location:** `engine/include/loader/DLLoader.hpp`, `engine/include/loader/DLLoader.tpp`

#### Key Methods

```cpp
template <typename T>
class DLLoader {
public:
    DLLoader();                                  // Default constructor
    explicit DLLoader(const std::string &path);  // Load on construction
    ~DLLoader();                                 // RAII: auto-close

    void open(const std::string &path);          // Load library
    void close();                                 // Unload library
    
    template <typename Func>
    Func getSymbol(const std::string &symbolName);  // Get function pointer
    
    std::shared_ptr<T> getInstance(
        const std::string &creatorFuncName = "create");  // Factory method
    
    bool isLoaded() const;                       // Query state
    std::string getPath() const;                 // Get loaded path

private:
    void *_handle;    // Platform-specific handle
    std::string _path;
};
```

#### Platform Abstraction

**Linux/POSIX:**
```cpp
_handle = dlopen(path.c_str(), RTLD_LAZY);
auto symbol = dlsym(_handle, symbolName.c_str());
dlclose(_handle);
```

**Windows:**
```cpp
_handle = LoadLibraryA(path.c_str());
auto symbol = GetProcAddress(_handle, symbolName.c_str());
FreeLibrary(_handle);
```

The template implementation (`DLLoader.tpp`) handles these differences transparently.

#### Memory Management

The loader uses RAII principles:

1. Constructor opens the library (if path provided)
2. Destructor automatically closes it
3. `getInstance()` returns `std::shared_ptr` - automatic cleanup
4. No manual `delete` needed

```cpp
{
    DLLoader<IAudioModule> loader("plugin.so");
    auto module = loader.getInstance();
    // Use module...
}  // Automatic cleanup here
```

## Interface Design

### Audio Interface: IAudioModule

The `IAudioModule` interface defines the complete audio backend API:

**Location:** `engine/include/audio/IAudioModule.hpp`

```cpp
namespace Engine::Audio {

struct PlaybackRequest {
    std::string id;
    float volume = 1.0f;
    bool loop = false;
    enum class Category { SFX, MUSIC } category = Category::SFX;
};

class IAudioModule {
public:
    virtual ~IAudioModule() = default;

    // Lifecycle Management
    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float delta_time) = 0;

    // Asset Management
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

    // Metadata
    virtual std::string GetModuleName() const = 0;
};

}  // namespace Engine::Audio
```

### Design Decisions

#### Why Pure Virtual?

- **Enforces implementation**: Plugin must implement every method
- **No coupling**: Interface has zero knowledge of implementation details
- **ABI stability**: Virtual dispatch through vtable is ABI-stable across compilers

#### Why separate SFX and Music?

Different use cases and optimizations:

- **SFX**: Typically small, loaded into memory, played simultaneously
- **Music**: Large files, streamed from disk, only one at a time
- Separate volume controls for game design flexibility

#### Why `PlaybackRequest` struct?

- **Extensibility**: Easy to add new parameters without breaking API
- **Type safety**: Enum for category prevents errors
- **Readability**: Named parameters in struct vs. long parameter lists

```cpp
// Clear and extensible
Play({.id = "laser", .volume = 0.8f, .category = Category::SFX});

// vs. unclear parameter soup
Play("laser", 0.8f, false, SFX);
```

## Plugin Implementation Examples

### Audio Plugin: SFMLAudioModule

**Files:**
- `client/plugins/audio/sfml/SFMLAudioModule.hpp` - Declaration
- `client/plugins/audio/sfml/SFMLAudioModule.cpp` - Implementation
- `client/plugins/audio/sfml/CMakeLists.txt` - Build configuration

**Entry Point:**

```cpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
        return std::make_shared<Engine::Audio::SFMLAudioModule>();
    }
}
```

### Video Plugin: SFMLVideoModule

**Files:**
- `client/plugins/video/sfml/SFMLVideoModule.hpp` - Declaration
- `client/plugins/video/sfml/SFMLVideoModule.cpp` - Implementation
- `client/plugins/video/sfml/CMakeLists.txt` - Build configuration

**Entry Point:**

```cpp
extern "C" {
    std::shared_ptr<Engine::Graphics::IVideoModule> entryPoint() {
        return std::make_shared<Engine::Graphics::SFMLVideoModule>();
    }
}
```

### Key Implementation Details

#### Entry Point Pattern

**Why `extern "C"`?**

C++ name mangling makes symbols unpredictable:
```
C++:  entryPoint() → _Z10entryPointv  (mangled, compiler-specific)
C:    entryPoint() → entryPoint       (predictable, stable)
```

#### Resource Management

The plugin manages resources in maps:

```cpp
private:
    std::map<std::string, std::unique_ptr<sf::SoundBuffer>> sound_buffers_;
    std::map<std::string, std::unique_ptr<sf::Sound>> sounds_;
    std::map<std::string, std::unique_ptr<sf::Music>> music_tracks_;
```

- **`unique_ptr`**: Exclusive ownership, automatic cleanup
- **Maps by ID**: Fast lookup by asset identifier
- **Separate buffers and sounds**: SFML requires buffers to outlive sounds

#### Volume Calculation

Volume combines multiple layers:

```cpp
float effective_volume = request.volume      // Per-sound volume
                       * category_volume     // SFX/Music volume
                       * (muted ? 0.0f : 1.0f);  // Mute state
```

This allows:
- Individual sound volumes
- Category-wide volume control
- Instant muting without losing volume settings

## Adapter Pattern

### Purpose

Adapters bridge plugin interfaces with legacy systems:

- **Audio**: `PluginAudioBackend` adapts `IAudioModule` to `IAudioBackend`
- **Video**: Direct usage of `IVideoModule` (no adapter needed in current design)

This allows existing client code to use plugins without modification.

### Implementation

**Location:** `client/engine/audio/PluginAudioBackend.{hpp,cpp}`

```cpp
class PluginAudioBackend : public IAudioBackend {
public:
    explicit PluginAudioBackend(std::shared_ptr<IAudioModule> module);
    
    // IAudioBackend interface (legacy)
    bool LoadSound(const std::string &id, const std::string &path) override {
        return module_->LoadSound(id, path);  // Delegate to plugin
    }
    
    void Play(const PlaybackRequest &request) override {
        // Convert client PlaybackRequest to plugin PlaybackRequest
        Engine::Audio::PlaybackRequest plugin_request;
        plugin_request.id = request.id;
        plugin_request.volume = request.volume;
        // ... conversion ...
        module_->Play(plugin_request);
    }
    
private:
    std::shared_ptr<IAudioModule> module_;  // Plugin instance
};
```

### Benefits

- **Backwards compatibility**: Existing systems work unchanged
- **Gradual migration**: Can migrate one system at a time
- **Testing**: Can test adapter independently

## Build System Integration

### Plugin CMakeLists.txt

```cmake
add_library(sfml_audio_module SHARED
    SFMLAudioModule.cpp
    SFMLAudioModule.hpp
)

target_link_libraries(sfml_audio_module PRIVATE SFML::Audio)

target_include_directories(sfml_audio_module
    PRIVATE ${CMAKE_SOURCE_DIR}/engine/include
)

target_compile_features(sfml_audio_module PRIVATE cxx_std_23)

set_target_properties(sfml_audio_module PROPERTIES
    PREFIX ""  # Output: sfml_audio_module.so not libsfml_audio_module.so
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)
```

### Key Settings

- **`SHARED`**: Creates `.so`/`.dll`, not static library
- **`PREFIX ""`**: Removes "lib" prefix for cleaner names
- **`LIBRARY_OUTPUT_DIRECTORY`**: Centralizes plugins in `lib/` folder
- **Private includes**: Plugin doesn't expose engine headers to linker

## Loading Flow

### Complete Sequence

```
1. Application starts
   ↓
2. DLLoader.open("lib/sfml_audio_module.so")
   - dlopen() loads .so into memory
   - Returns handle to library
   ↓
3. DLLoader.getInstance("entryPoint")
   - dlsym() finds entry point symbol
   - Calls factory function
   - Returns shared_ptr<IAudioModule>
   ↓
4. Create adapter: PluginAudioBackend(module)
   - Wraps plugin in legacy interface
   ↓
5. Create manager: AudioManager(backend)
   - Engine now uses plugin via adapter
   ↓
6. Normal operation
   - All audio calls go through plugin
   ↓
7. Application exits
   - shared_ptr ref count drops to 0
   - Plugin destructor runs
   - DLLoader destructor calls dlclose()
   - Library unloaded from memory
```

### Error Handling

Each step includes error checking:

```cpp
try {
    loader.open(plugin_path);
} catch (const DLLoaderException &e) {
    std::cerr << "Failed to load plugin: " << e.what() << std::endl;
    // Fall back to default backend or exit
}
```

## Performance Considerations

### Virtual Function Overhead

- **Cost**: One vtable lookup per call (~3-5 CPU cycles)
- **Impact**: Negligible for audio operations (I/O-bound)
- **Benefit**: Clean abstractions worth the tiny overhead

### Dynamic Loading Cost

- **Load time**: ~1-10ms for typical plugin
- **Runtime**: Zero overhead after loading
- **Memory**: Shared libraries use memory-mapped I/O (efficient)

### Shared Pointer Overhead

- **Memory**: 16 bytes (pointer + control block) on 64-bit
- **CPU**: Atomic ref count operations
- **Benefit**: Automatic memory management prevents leaks

## Thread Safety

### DLLoader

- **Not thread-safe**: Load/unload from single thread only
- **Rationale**: Initialization happens once at startup
- **Instance usage**: Safe once loaded (read-only operations)

### Plugin Implementation

- **Responsibility**: Plugin must ensure thread safety
- **SFML**: Not thread-safe, calls from audio thread only
- **Recommendation**: Use mutex for concurrent access

```cpp
class MyAudioModule : public IAudioModule {
private:
    mutable std::mutex mutex_;
    
public:
    void Play(const PlaybackRequest &request) override {
        std::lock_guard<std::mutex> lock(mutex_);
        // Thread-safe implementation
    }
};
```

## Testing Strategy

### Unit Tests

Test each component independently:

```cpp
// Test DLLoader
TEST(DLLoaderTest, LoadValidLibrary) {
    DLLoader<IAudioModule> loader;
    EXPECT_NO_THROW(loader.open("lib/sfml_audio_module.so"));
}

// Test Plugin
TEST(PluginTest, Initialize) {
    auto module = /* load plugin */;
    EXPECT_TRUE(module->Initialize());
}

// Test Adapter
TEST(AdapterTest, ForwardsCall) {
    auto mock_module = std::make_shared<MockAudioModule>();
    PluginAudioBackend adapter(mock_module);
    EXPECT_CALL(*mock_module, LoadSound(_, _));
    adapter.LoadSound("test", "test.ogg");
}
```

### Integration Tests

Test the complete flow:

```cpp
TEST(IntegrationTest, LoadAndPlay) {
    DLLoader<IAudioModule> loader("lib/sfml_audio_module.so");
    auto module = loader.getInstance("entryPoint");
    auto backend = std::make_unique<PluginAudioBackend>(module);
    AudioManager manager(std::move(backend));
    
    EXPECT_TRUE(manager.RegisterAsset("test", "assets/test.ogg", false));
    EXPECT_NO_THROW(manager.PlaySound("test"));
}
```

## Future Enhancements

### Plugin Metadata

Add version and capability queries:

```cpp
class IAudioModule {
    virtual Version GetVersion() const = 0;
    virtual Capabilities GetCapabilities() const = 0;
};
```

### Plugin Registry

Automatic discovery of available plugins:

```cpp
class PluginRegistry {
    std::vector<PluginInfo> DiscoverPlugins(const std::string &directory);
    std::shared_ptr<IAudioModule> LoadPlugin(const std::string &name);
};
```

### Hot Reloading

Support runtime plugin updates:

```cpp
void AudioManager::ReloadPlugin() {
    auto state = SaveState();
    loader_.close();
    loader_.open(plugin_path_);
    RestoreState(state);
}
```

## Comparison with Alternatives

### Static Linking

**Plugin System:**
- ✅ Runtime flexibility
- ✅ Smaller executables
- ❌ Slightly slower load time

**Static Linking:**
- ❌ Must recompile to change backend
- ❌ Larger executable
- ✅ Marginally faster startup

### Factory Pattern

The plugin system could use compile-time factories:

```cpp
std::unique_ptr<IAudioBackend> CreateBackend(BackendType type) {
    switch (type) {
        case SFML: return std::make_unique<SFMLAudioBackend>();
        case OpenAL: return std::make_unique<OpenALBackend>();
    }
}
```

**Why plugins are better:**
- No recompilation needed
- Can add backends without modifying engine
- True modularity and separation

## References

- [Dynamic Loading in C++](https://tldp.org/HOWTO/Program-Library-HOWTO/dl-libraries.html)
- [POSIX dlopen](https://man7.org/linux/man-pages/man3/dlopen.3.html)
- [Modern C++ Design Patterns](https://refactoring.guru/design-patterns/cpp)
- [Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html)

## Conclusion

The plugin system provides a robust, type-safe, and performant architecture for dynamic module loading. It demonstrates modern C++ best practices while maintaining simplicity and ease of use. The design is extensible, allowing future plugins for graphics, physics, networking, and more.
