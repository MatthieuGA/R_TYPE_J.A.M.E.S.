# Plugin API Reference

Complete API documentation for the R-TYPE J.A.M.E.S. plugin system.

## DLLoader Template Class

**Header:** `engine/include/loader/DLLoader.hpp`

Generic template class for loading dynamic libraries at runtime.

### Template Parameters

| Parameter | Description |
|-----------|-------------|
| `T` | Interface type that the plugin implements |

### Constructors

#### `DLLoader()`

Default constructor. Creates an unloaded loader.

```cpp
Engine::DLLoader<Engine::Audio::IAudioModule> loader;
```

#### `DLLoader(const std::string &path)`

Constructs and immediately loads a library.

**Parameters:**
- `path` - Path to the shared library file

**Throws:**
- `DLLoaderException` - If the library cannot be loaded

```cpp
Engine::DLLoader<IAudioModule> loader("lib/plugin.so");
```

### Methods

#### `void open(const std::string &path)`

Loads a shared library from the specified path.

**Parameters:**
- `path` - Absolute or relative path to `.so` (Linux) or `.dll` (Windows) file

**Throws:**
- `DLLoaderException` - If the library cannot be opened

**Example:**
```cpp
loader.open("lib/sfml_audio_module.so");
```

**Notes:**
- Closes any previously loaded library
- Path is relative to the executable's working directory

---

#### `void close()`

Unloads the currently loaded library.

**Throws:**
- None (safe to call multiple times)

**Example:**
```cpp
loader.close();
```

**Notes:**
- Automatically called by destructor
- All pointers/references to plugin objects must be released first

---

#### `template<typename Func> Func getSymbol(const std::string &symbolName)`

Retrieves a symbol (function pointer) from the loaded library.

**Template Parameters:**
- `Func` - Function pointer type to cast the symbol to

**Parameters:**
- `symbolName` - Name of the symbol to retrieve

**Returns:**
- Function pointer of type `Func`

**Throws:**
- `DLLoaderException` - If symbol not found or library not loaded

**Example:**
```cpp
using CreateFunc = std::shared_ptr<IAudioModule>(*)();
auto create_fn = loader.getSymbol<CreateFunc>("entryPoint");
auto module = create_fn();
```

---

#### `std::shared_ptr<T> getInstance(const std::string &creatorFuncName = "create")`

Loads and invokes a factory function to create a plugin instance.

**Parameters:**
- `creatorFuncName` - Name of the factory function (default: `"create"`)

**Returns:**
- `std::shared_ptr<T>` - Smart pointer to the plugin instance

**Throws:**
- `DLLoaderException` - If factory function not found or returns null

**Example:**
```cpp
auto module = loader.getInstance("entryPoint");
```

**Factory Function Signature:**
```cpp
extern "C" {
    std::shared_ptr<T> factoryFunction();
}
```

---

#### `bool isLoaded() const`

Checks if a library is currently loaded.

**Returns:**
- `true` if a library is loaded, `false` otherwise

**Example:**
```cpp
if (loader.isLoaded()) {
    // Use loader...
}
```

---

#### `std::string getPath() const`

Gets the path of the currently loaded library.

**Returns:**
- Path string if loaded, empty string otherwise

**Example:**
```cpp
std::string path = loader.getPath();
std::cout << "Loaded from: " << path << std::endl;
```

### Exceptions

#### `DLLoaderException`

Exception thrown for dynamic loading errors.

**Base Class:** `std::exception`

**Methods:**
- `const char* what() const noexcept` - Returns error message

**Common Causes:**
- Library file not found
- Symbol not found in library
- ABI incompatibility
- Missing dependencies

**Example:**
```cpp
try {
    loader.open("nonexistent.so");
} catch (const Engine::DLLoaderException &e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

---

## IAudioModule Interface

**Header:** `engine/include/audio/IAudioModule.hpp`

**Namespace:** `Engine::Audio`

Pure virtual interface that all audio plugins must implement.

### Data Structures

#### `PlaybackRequest`

Structure containing playback parameters.

```cpp
struct PlaybackRequest {
    std::string id;      // Asset identifier
    float volume;        // Volume multiplier (0.0 - 1.0)
    bool loop;           // Loop playback
    Category category;   // SFX or MUSIC
};
```

**Members:**

| Member | Type | Default | Description |
|--------|------|---------|-------------|
| `id` | `std::string` | - | Unique identifier for the asset |
| `volume` | `float` | `1.0f` | Volume level (0.0 = silent, 1.0 = full) |
| `loop` | `bool` | `false` | Whether to loop playback |
| `category` | `Category` | `SFX` | Asset category (SFX or MUSIC) |

**Example:**
```cpp
PlaybackRequest request{
    .id = "explosion",
    .volume = 0.8f,
    .loop = false,
    .category = PlaybackRequest::Category::SFX
};
```

#### `PlaybackRequest::Category`

Enumeration for audio categories.

```cpp
enum class Category {
    SFX,    // Sound effects
    MUSIC   // Background music
};
```

### Methods

#### `virtual bool Initialize() = 0`

Initializes the audio backend.

**Returns:**
- `true` on success, `false` on failure

**Called:**
- Once after plugin is loaded, before any other methods

**Implementation Requirements:**
- Initialize audio device/context
- Allocate necessary resources
- Return `false` if initialization fails

**Example:**
```cpp
bool MyAudioModule::Initialize() {
    if (!InitAudioDevice()) {
        return false;
    }
    return true;
}
```

---

#### `virtual void Shutdown() = 0`

Shuts down the audio backend and releases resources.

**Called:**
- Once before plugin is unloaded

**Implementation Requirements:**
- Stop all playing audio
- Release all loaded assets
- Close audio device/context
- Clean up all resources

**Example:**
```cpp
void MyAudioModule::Shutdown() {
    sounds_.clear();
    music_tracks_.clear();
    CloseAudioDevice();
}
```

---

#### `virtual void Update(float delta_time) = 0`

Updates the audio system (called every frame).

**Parameters:**
- `delta_time` - Time elapsed since last update (in seconds)

**Called:**
- Every frame by the engine

**Implementation Requirements:**
- Update streaming audio
- Remove finished sounds
- Process audio events

**Example:**
```cpp
void MyAudioModule::Update(float delta_time) {
    UpdateMusicStream(current_music_);
    RemoveFinishedSounds();
}
```

---

#### `virtual bool LoadSound(const std::string &id, const std::string &path) = 0`

Loads a sound effect from file.

**Parameters:**
- `id` - Unique identifier for this sound
- `path` - File path to the audio asset

**Returns:**
- `true` on success, `false` on failure

**Implementation Requirements:**
- Load entire file into memory
- Store for fast playback
- Return `false` if file not found or invalid

**Example:**
```cpp
bool MyAudioModule::LoadSound(const std::string &id, const std::string &path) {
    auto buffer = LoadAudioFile(path);
    if (!buffer) return false;
    
    sounds_[id] = std::move(buffer);
    return true;
}
```

---

#### `virtual bool LoadMusic(const std::string &id, const std::string &path) = 0`

Loads background music from file.

**Parameters:**
- `id` - Unique identifier for this music track
- `path` - File path to the audio asset

**Returns:**
- `true` on success, `false` on failure

**Implementation Requirements:**
- Open file for streaming (don't load entirely)
- Store reference for playback
- Return `false` if file not found or invalid

**Example:**
```cpp
bool MyAudioModule::LoadMusic(const std::string &id, const std::string &path) {
    auto stream = OpenMusicStream(path);
    if (!stream) return false;
    
    music_tracks_[id] = std::move(stream);
    return true;
}
```

---

#### `virtual void Play(const PlaybackRequest &request) = 0`

Plays a sound effect or music track.

**Parameters:**
- `request` - Playback parameters (id, volume, loop, category)

**Implementation Requirements:**
- Find asset by `request.id`
- Apply volume: `request.volume * category_volume * (muted ? 0.0f : 1.0f)`
- Set loop: `request.loop`
- For MUSIC: Stop current music first
- For SFX: Play alongside other sounds

**Example:**
```cpp
void MyAudioModule::Play(const PlaybackRequest &request) {
    if (request.category == PlaybackRequest::Category::MUSIC) {
        StopMusic();
        PlayMusicTrack(music_tracks_[request.id], request);
    } else {
        PlaySoundEffect(sounds_[request.id], request);
    }
}
```

---

#### `virtual void StopMusic() = 0`

Stops currently playing music.

**Implementation Requirements:**
- Stop music playback
- Reset music position (optional)
- Safe to call if no music is playing

**Example:**
```cpp
void MyAudioModule::StopMusic() {
    if (current_music_) {
        current_music_->stop();
        current_music_ = nullptr;
    }
}
```

---

#### `virtual void SetSfxVolume(float volume) = 0`

Sets the master volume for sound effects.

**Parameters:**
- `volume` - Volume level (0.0 = silent, 1.0 = full)

**Implementation Requirements:**
- Store volume for future SFX playback
- Update volume of currently playing SFX
- Combine with per-sound volume and mute state

**Example:**
```cpp
void MyAudioModule::SetSfxVolume(float volume) {
    sfx_volume_ = volume * 100.0f;
    for (auto &[id, sound] : sounds_) {
        UpdateSoundVolume(sound, sfx_volume_);
    }
}
```

---

#### `virtual void SetMusicVolume(float volume) = 0`

Sets the master volume for music.

**Parameters:**
- `volume` - Volume level (0.0 = silent, 1.0 = full)

**Implementation Requirements:**
- Store volume for music playback
- Update volume of currently playing music
- Combine with mute state

**Example:**
```cpp
void MyAudioModule::SetMusicVolume(float volume) {
    music_volume_ = volume * 100.0f;
    if (current_music_) {
        current_music_->setVolume(music_volume_ * (music_muted_ ? 0.0f : 1.0f));
    }
}
```

---

#### `virtual void MuteSfx(bool mute) = 0`

Mutes or unmutes sound effects.

**Parameters:**
- `mute` - `true` to mute, `false` to unmute

**Implementation Requirements:**
- Store mute state
- Set SFX volume to 0 when muted
- Restore volume when unmuted

**Example:**
```cpp
void MyAudioModule::MuteSfx(bool mute) {
    sfx_muted_ = mute;
    SetSfxVolume(sfx_volume_ / 100.0f);
}
```

---

#### `virtual void MuteMusic(bool mute) = 0`

Mutes or unmutes music.

**Parameters:**
- `mute` - `true` to mute, `false` to unmute

**Implementation Requirements:**
- Store mute state
- Set music volume to 0 when muted
- Restore volume when unmuted

**Example:**
```cpp
void MyAudioModule::MuteMusic(bool mute) {
    music_muted_ = mute;
    SetMusicVolume(music_volume_ / 100.0f);
}
```

---

#### `virtual std::string GetModuleName() const = 0`

Returns the plugin's display name.

**Returns:**
- Human-readable plugin name

**Example:**
```cpp
std::string MyAudioModule::GetModuleName() const {
    return "OpenAL Audio Module v1.0";
}
```

---

---

## IVideoModule Interface

**Header:** `engine/include/graphics/IVideoModule.hpp`

**Namespace:** `Engine::Graphics`

Pure virtual interface that all video/graphics plugins must implement.

### Core Types

#### `Color`

RGBA color representation.

```cpp
struct Color {
    uint8_t r, g, b, a;  // Red, Green, Blue, Alpha (0-255)
};
```

**Examples:**
```cpp
Color red{255, 0, 0, 255};
Color transparent_blue{0, 0, 255, 128};
```

#### `Vector2f`

2D floating-point vector.

```cpp
struct Vector2f {
    float x, y;
};
```

#### `Transform`

2D transformation matrix (position, rotation, scale).

```cpp
class Transform {
    void SetPosition(const Vector2f& position);
    void SetRotation(float angle);
    void SetScale(const Vector2f& scale);
    // Matrix operations...
};
```

#### `Event`

Window and input events.

```cpp
struct Event {
    enum class Type {
        Closed, KeyPressed, KeyReleased,
        MouseButtonPressed, MouseButtonReleased,
        MouseMoved, Resized
    };
    
    Type type;
    // Event-specific data...
};
```

### Methods

#### `virtual bool CreateWindow(uint32_t width, uint32_t height, const std::string &title) = 0`

Creates the application window.

**Parameters:**
- `width` - Window width in pixels
- `height` - Window height in pixels
- `title` - Window title

**Returns:**
- `true` on success, `false` on failure

**Example:**
```cpp
if (!videoModule->CreateWindow(1920, 1080, "R-Type J.A.M.E.S.")) {
    std::cerr << "Failed to create window" << std::endl;
}
```

---

#### `virtual void Clear(const Color &color) = 0`

Clears the window with a solid color.

**Parameters:**
- `color` - RGBA color to fill the window

**Example:**
```cpp
videoModule->Clear(Color{0, 0, 0, 255});  // Black
```

---

#### `virtual void Display() = 0`

Displays rendered content (flips buffers).

**Called:**
- Once per frame after all drawing

**Example:**
```cpp
videoModule->Clear(Color{0, 0, 0, 255});
// ... draw calls ...
videoModule->Display();
```

---

#### `virtual bool IsWindowOpen() const = 0`

Checks if the window is still open.

**Returns:**
- `true` if window is open, `false` if closed

**Example:**
```cpp
while (videoModule->IsWindowOpen()) {
    // Game loop...
}
```

---

#### `virtual bool PollEvent(Event &event) = 0`

Retrieves the next pending event.

**Parameters:**
- `event` - Event structure to fill

**Returns:**
- `true` if event was retrieved, `false` if no more events

**Example:**
```cpp
Event event;
while (videoModule->PollEvent(event)) {
    if (event.type == Event::Type::Closed) {
        videoModule->CloseWindow();
    }
}
```

---

#### `virtual bool LoadTexture(const std::string &id, const std::string &path) = 0`

Loads a texture from file.

**Parameters:**
- `id` - Unique identifier
- `path` - File path to image

**Returns:**
- `true` on success, `false` on failure

**Example:**
```cpp
videoModule->LoadTexture("ship", "assets/sprites/r-typesheet42.gif");
```

---

#### `virtual void DrawSprite(const std::string &texture_id, const Transform &transform, const Color &color) = 0`

Draws a sprite with transformation and tint.

**Parameters:**
- `texture_id` - ID of loaded texture
- `transform` - Position, rotation, scale
- `color` - Color tint (255,255,255,255 for no tint)

**Example:**
```cpp
Transform t;
t.SetPosition({100.0f, 200.0f});
t.SetRotation(45.0f);
videoModule->DrawSprite("ship", t, Color{255, 255, 255, 255});
```

---

## Entry Point Functions

Every plugin must provide a C-style entry point function.

### Audio Plugin Signature

```cpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint();
}
```

### Video Plugin Signature

```cpp
extern "C" {
    std::shared_ptr<Engine::Graphics::IVideoModule> entryPoint();
}
```

### Requirements

- Must be declared with `extern "C"` linkage
- Must return `std::shared_ptr` to the appropriate interface type
- Must not throw exceptions
- Default name is `"entryPoint"` (customizable)

### Example Implementations

**Audio Plugin:**
```cpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> entryPoint() {
        try {
            return std::make_shared<SFMLAudioModule>();
        } catch (const std::exception &e) {
            std::cerr << "Audio plugin creation failed: " << e.what() << std::endl;
            return nullptr;
        }
    }
}
```

**Video Plugin:**
```cpp
extern "C" {
    std::shared_ptr<Engine::Graphics::IVideoModule> entryPoint() {
        try {
            return std::make_shared<SFMLVideoModule>();
        } catch (const std::exception &e) {
            std::cerr << "Video plugin creation failed: " << e.what() << std::endl;
            return nullptr;
        }
    }
}
```

### Custom Entry Point Names

You can use a different name for either plugin type:

**Audio:**
```cpp
extern "C" {
    std::shared_ptr<Engine::Audio::IAudioModule> createMyAudioPlugin() {
        return std::make_shared<MyAudioModule>();
    }
}
```

**Video:**
```cpp
extern "C" {
    std::shared_ptr<Engine::Graphics::IVideoModule> createMyVideoPlugin() {
        return std::make_shared<MyVideoModule>();
    }
}
```

Load with custom name:
```cpp
auto audioModule = audioLoader.getInstance("createMyAudioPlugin");
auto videoModule = videoLoader.getInstance("createMyVideoPlugin");
```

---

## Usage Examples

### Basic Audio Plugin Loading

```cpp
#include <loader/DLLoader.hpp>
#include <audio/IAudioModule.hpp>

// Load plugin
Engine::DLLoader<Engine::Audio::IAudioModule> loader;
loader.open("lib/sfml_audio_module.so");

// Get instance
auto module = loader.getInstance("entryPoint");

// Initialize
if (!module->Initialize()) {
    throw std::runtime_error("Failed to initialize audio module");
}

// Use plugin
module->LoadSound("laser", "assets/sounds/laser.ogg");
module->Play({.id = "laser", .volume = 0.8f});

// Cleanup
module->Shutdown();
loader.close();
```

### Basic Video Plugin Loading

```cpp
#include <loader/DLLoader.hpp>
#include <graphics/IVideoModule.hpp>

// Load plugin
Engine::DLLoader<Engine::Graphics::IVideoModule> loader;
loader.open("lib/sfml_video_module.so");

// Get instance
auto module = loader.getInstance("entryPoint");

// Initialize window
if (!module->CreateWindow(1920, 1080, "My Game")) {
    throw std::runtime_error("Failed to create window");
}

// Use plugin
module->LoadTexture("ship", "assets/sprites/ship.png");

// Game loop
while (module->IsWindowOpen()) {
    Event event;
    while (module->PollEvent(event)) {
        // Handle events...
    }
    
    module->Clear(Color{0, 0, 0, 255});
    // Draw calls...
    module->Display();
}

// Cleanup
loader.close();
```

### Error Handling

**Generic pattern (works for both audio and video):**

```cpp
template<typename T>
std::shared_ptr<T> LoadPluginSafe(const std::string& plugin_path) {
    try {
        Engine::DLLoader<T> loader;
        loader.open(plugin_path);
        
        auto module = loader.getInstance("entryPoint");
        if (!module) {
            throw std::runtime_error("Entry point returned null");
        }
        
        return module;
        
    } catch (const Engine::DLLoaderException &e) {
        std::cerr << "Plugin error: " << e.what() << std::endl;
        // Fall back to default backend or null
        return nullptr;
    } catch (const std::exception &e) {
        std::cerr << "Unexpected error: " << e.what() << std::endl;
        return nullptr;
    }
}

// Usage:
auto audioModule = LoadPluginSafe<Engine::Audio::IAudioModule>("lib/sfml_audio_module.so");
auto videoModule = LoadPluginSafe<Engine::Graphics::IVideoModule>("lib/sfml_video_module.so");

if (audioModule && audioModule->Initialize()) {
    // Use audio module...
}

if (videoModule && videoModule->CreateWindow(800, 600, "Game")) {
    // Use video module...
}
```

### Using with RAII

**Audio System Wrapper:**
```cpp
class AudioSystem {
public:
    AudioSystem(const std::string &plugin_path) {
        loader_.open(plugin_path);
        module_ = loader_.getInstance("entryPoint");
        module_->Initialize();
    }
    
    ~AudioSystem() {
        module_->Shutdown();
        loader_.close();
    }
    
    // Prevent copying
    AudioSystem(const AudioSystem&) = delete;
    AudioSystem& operator=(const AudioSystem&) = delete;
    
    Engine::Audio::IAudioModule* Get() { return module_.get(); }
    
private:
    Engine::DLLoader<Engine::Audio::IAudioModule> loader_;
    std::shared_ptr<Engine::Audio::IAudioModule> module_;
};
```

**Graphics System Wrapper:**
```cpp
class GraphicsSystem {
public:
    GraphicsSystem(const std::string &plugin_path, uint32_t width, uint32_t height) {
        loader_.open(plugin_path);
        module_ = loader_.getInstance("entryPoint");
        if (!module_->CreateWindow(width, height, "Game")) {
            throw std::runtime_error("Failed to create window");
        }
    }
    
    ~GraphicsSystem() {
        loader_.close();
    }
    
    GraphicsSystem(const GraphicsSystem&) = delete;
    GraphicsSystem& operator=(const GraphicsSystem&) = delete;
    
    Engine::Graphics::IVideoModule* Get() { return module_.get(); }
    
private:
    Engine::DLLoader<Engine::Graphics::IVideoModule> loader_;
    std::shared_ptr<Engine::Graphics::IVideoModule> module_;
};
```

---

## Type Reference

### Common Types

| Type | Description |
|------|-------------|
| `std::shared_ptr<T>` | Smart pointer with reference counting |
| `std::string` | Standard string class |
| `float` | Single-precision floating point |
| `bool` | Boolean value (true/false) |

### Platform-Specific

| Platform | Library Extension | Loading Function |
|----------|------------------|------------------|
| Linux | `.so` | `dlopen()` |
| Windows | `.dll` | `LoadLibrary()` |
| macOS | `.dylib` | `dlopen()` |

---

## See Also

- [Audio Plugin Development Guide](./audio-plugin-guide.md)
- [Video Plugin Development Guide](./video-plugin-guide.md)
- [Architecture Overview](./architecture.md)
- [Troubleshooting](./troubleshooting.md)
