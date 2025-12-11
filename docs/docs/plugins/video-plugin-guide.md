# Video Plugin Development Guide

This comprehensive guide walks you through creating a custom video/graphics plugin for the R-TYPE J.A.M.E.S. engine from scratch.

## Prerequisites

Before starting, ensure you have:

- C++23 compatible compiler (GCC 12+, Clang 15+, MSVC 2022+)
- CMake 3.23 or higher
- Basic understanding of C++ virtual interfaces
- Familiarity with dynamic library concepts
- Understanding of graphics rendering concepts

## What You'll Build

By the end of this guide, you'll have created a complete video plugin that:
- Loads dynamically at runtime
- Implements the `IVideoModule` interface
- Handles window creation and event processing
- Renders sprites, text, and applies shaders
- Compiles to a shared library (`.so` or `.dll`)
- Integrates seamlessly with the engine

## Step 1: Understanding the Interface

The `IVideoModule` interface defines the contract for all video plugins:

```cpp
namespace Engine::Video {

// Core types (backend-agnostic)
struct Color {
    uint8_t r, g, b, a;
    static const Color White;
    static const Color Black;
    // ... other colors
};

struct Vector2f {
    float x, y;
};

struct FloatRect {
    float left, top, width, height;
};

struct Transform {
    Vector2f position;
    float rotation;  // degrees
    Vector2f scale;
    Vector2f origin;
};

enum class EventType {
    Closed,
    KeyPressed,
    KeyReleased,
    MouseMoved,
    // ... other event types
};

struct Event {
    EventType type;
    union {
        int key_code;
        struct { int mouse_x, mouse_y; };
        float mouse_wheel_delta;
        int mouse_button;
    } data;
};

class IVideoModule {
 public:
    virtual ~IVideoModule() = default;

    // Lifecycle
    virtual bool Initialize(unsigned int width, unsigned int height,
                          const std::string &title) = 0;
    virtual void Shutdown() = 0;

    // Window Management
    virtual bool IsOpen() const = 0;
    virtual void Close() = 0;
    virtual bool PollEvent(Event &event) = 0;

    // Rendering
    virtual void Clear(const Color &color) = 0;
    virtual void Display() = 0;
    virtual void DrawSprite(const std::string &texture_id,
                          const Transform &transform,
                          const FloatRect *texture_rect = nullptr,
                          const Color &color = Color::White,
                          const std::string *shader_id = nullptr) = 0;
    virtual void DrawText(const std::string &font_id,
                        const std::string &text,
                        const Transform &transform,
                        unsigned int character_size,
                        const Color &color = Color::White) = 0;

    // Texture Management
    virtual bool LoadTexture(const std::string &id,
                           const std::string &path) = 0;
    virtual const void *GetTexture(const std::string &id) const = 0;
    virtual Vector2f GetTextureSize(const std::string &id) const = 0;

    // Font Management
    virtual bool LoadFont(const std::string &id,
                        const std::string &path) = 0;
    virtual const void *GetFont(const std::string &id) const = 0;
    virtual FloatRect GetTextBounds(const std::string &text,
                                   const std::string &font_id,
                                   unsigned int character_size) const = 0;

    // Shader Management
    virtual bool LoadShader(const std::string &id,
                          const std::string &fragment_path) = 0;
    virtual void SetShaderParameter(const std::string &shader_id,
                                   const std::string &param_name,
                                   float value) = 0;

    // Identification
    virtual std::string GetModuleName() const = 0;
};

}  // namespace Engine::Video
```

**Key Points:**
- All methods are pure virtual - you must implement every one
- The interface is backend-agnostic (no SFML/SDL types exposed)
- Uses custom types (`Color`, `Vector2f`, etc.) for portability
- Manages resources via string IDs (textures, fonts, shaders)

## Step 2: Create the Plugin Directory Structure

Create your plugin in the appropriate location:

```bash
mkdir -p client/plugins/video/my_video_plugin
cd client/plugins/video/my_video_plugin
```

You'll create three files:
1. `MyVideoModule.hpp` - Header declaration
2. `MyVideoModule.cpp` - Implementation
3. `CMakeLists.txt` - Build configuration

## Step 3: Implement the Header File

**File:** `client/plugins/video/my_video_plugin/MyVideoModule.hpp`

```cpp
#pragma once
#include <video/IVideoModule.hpp>
#include <map>
#include <memory>
#include <string>

// Include your graphics library headers here
// For SFML example:
#include <SFML/Graphics.hpp>

namespace Engine::Video {

/**
 * @brief Video module implementation using SFML.
 * 
 * This can be adapted for other backends (SDL2, Raylib, etc.)
 */
class MyVideoModule : public IVideoModule {
 public:
    MyVideoModule() = default;
    ~MyVideoModule() override = default;

    // Lifecycle methods
    bool Initialize(unsigned int width, unsigned int height,
                   const std::string &title) override;
    void Shutdown() override;

    // Window management
    bool IsOpen() const override;
    void Close() override;
    bool PollEvent(Event &event) override;

    // Rendering
    void Clear(const Color &color) override;
    void Display() override;
    void DrawSprite(const std::string &texture_id,
                   const Transform &transform,
                   const FloatRect *texture_rect = nullptr,
                   const Color &color = Color::White,
                   const std::string *shader_id = nullptr) override;
    void DrawText(const std::string &font_id,
                 const std::string &text,
                 const Transform &transform,
                 unsigned int character_size,
                 const Color &color = Color::White) override;

    // Texture management
    bool LoadTexture(const std::string &id,
                    const std::string &path) override;
    const void *GetTexture(const std::string &id) const override;
    Vector2f GetTextureSize(const std::string &id) const override;

    // Font management
    bool LoadFont(const std::string &id,
                 const std::string &path) override;
    const void *GetFont(const std::string &id) const override;
    FloatRect GetTextBounds(const std::string &text,
                           const std::string &font_id,
                           unsigned int character_size) const override;

    // Shader management
    bool LoadShader(const std::string &id,
                   const std::string &fragment_path) override;
    void SetShaderParameter(const std::string &shader_id,
                           const std::string &param_name,
                           float value) override;

    // Identification
    std::string GetModuleName() const override;

 private:
    // Helper functions for type conversion
    static sf::Color ToSFMLColor(const Color &color);
    static Event FromSFMLEvent(const sf::Event &sf_event);

    // Backend state
    std::unique_ptr<sf::RenderWindow> window_;
    std::map<std::string, std::shared_ptr<sf::Texture>> textures_;
    std::map<std::string, std::shared_ptr<sf::Font>> fonts_;
    std::map<std::string, std::shared_ptr<sf::Shader>> shaders_;
};

}  // namespace Engine::Video

// C-style entry point (required for dynamic loading)
extern "C" {
    std::shared_ptr<Engine::Video::IVideoModule> entryPoint();
}
```

**Critical Details:**

1. **Namespace:** Must be `Engine::Video`
2. **Entry Point:** The `extern "C"` block prevents name mangling
3. **Return Type:** Must return `std::shared_ptr<IVideoModule>`
4. **Resource Management:** Use smart pointers for RAII

## Step 4: Implement the Source File

**File:** `client/plugins/video/my_video_plugin/MyVideoModule.cpp`

```cpp
#include "MyVideoModule.hpp"  // NOLINT(build/include_subdir)

#include <iostream>
#include <memory>

namespace Engine::Video {

// ===== Lifecycle =====

bool MyVideoModule::Initialize(unsigned int width, unsigned int height,
                               const std::string &title) {
    std::cout << "[MyVideoModule] Initializing " << width << "x" << height 
              << " - " << title << std::endl;

    window_ = std::make_unique<sf::RenderWindow>(
        sf::VideoMode(width, height), title);
    
    if (!window_) {
        std::cerr << "[MyVideoModule] Failed to create window" << std::endl;
        return false;
    }

    // Set the view to match window dimensions exactly (prevents offset)
    sf::View view(sf::FloatRect(0, 0, 
        static_cast<float>(width), 
        static_cast<float>(height)));
    window_->setView(view);

    window_->setFramerateLimit(60);

    std::cout << "[MyVideoModule] Initialized successfully" << std::endl;
    return true;
}

void MyVideoModule::Shutdown() {
    std::cout << "[MyVideoModule] Shutting down..." << std::endl;
    
    if (window_) {
        window_->close();
        window_.reset();
    }
    
    textures_.clear();
    fonts_.clear();
    shaders_.clear();
    
    std::cout << "[MyVideoModule] Shutdown complete" << std::endl;
}

// ===== Window Management =====

bool MyVideoModule::IsOpen() const {
    return window_ && window_->isOpen();
}

void MyVideoModule::Close() {
    if (window_) {
        window_->close();
    }
}

bool MyVideoModule::PollEvent(Event &event) {
    if (!window_) return false;

    sf::Event sf_event;
    if (!window_->pollEvent(sf_event)) {
        return false;
    }

    // Convert SFML event to engine event
    event = FromSFMLEvent(sf_event);
    return true;
}

// ===== Rendering =====

void MyVideoModule::Clear(const Color &color) {
    if (window_) {
        window_->clear(ToSFMLColor(color));
    }
}

void MyVideoModule::Display() {
    if (window_) {
        window_->display();
    }
}

void MyVideoModule::DrawSprite(const std::string &texture_id,
                               const Transform &transform,
                               const FloatRect *texture_rect,
                               const Color &color,
                               const std::string *shader_id) {
    auto it = textures_.find(texture_id);
    if (it == textures_.end() || !window_) {
        return;
    }

    sf::Sprite sprite;
    sprite.setTexture(*it->second);

    // Set texture rectangle if provided (for sprite sheets)
    if (texture_rect) {
        sprite.setTextureRect(sf::IntRect(
            static_cast<int>(texture_rect->left),
            static_cast<int>(texture_rect->top),
            static_cast<int>(texture_rect->width),
            static_cast<int>(texture_rect->height)
        ));
    }

    // Apply transform
    sprite.setPosition(transform.position.x, transform.position.y);
    sprite.setRotation(transform.rotation);
    sprite.setScale(transform.scale.x, transform.scale.y);
    sprite.setOrigin(transform.origin.x, transform.origin.y);
    sprite.setColor(ToSFMLColor(color));

    // Draw with optional shader
    if (shader_id) {
        auto shader_it = shaders_.find(*shader_id);
        if (shader_it != shaders_.end()) {
            window_->draw(sprite, sf::RenderStates(shader_it->second.get()));
            return;
        }
    }

    window_->draw(sprite);
}

void MyVideoModule::DrawText(const std::string &font_id,
                             const std::string &text,
                             const Transform &transform,
                             unsigned int character_size,
                             const Color &color) {
    auto it = fonts_.find(font_id);
    if (it == fonts_.end() || !window_) {
        return;
    }

    sf::Text sf_text;
    sf_text.setFont(*it->second);
    sf_text.setString(text);
    sf_text.setCharacterSize(character_size);
    sf_text.setFillColor(ToSFMLColor(color));
    
    // Apply transform
    sf_text.setPosition(transform.position.x, transform.position.y);
    sf_text.setRotation(transform.rotation);
    sf_text.setScale(transform.scale.x, transform.scale.y);
    sf_text.setOrigin(transform.origin.x, transform.origin.y);

    window_->draw(sf_text);
}

// ===== Texture Management =====

bool MyVideoModule::LoadTexture(const std::string &id,
                                const std::string &path) {
    // Check if already loaded (prevent reloading)
    if (textures_.find(id) != textures_.end()) {
        return true;
    }

    auto texture = std::make_shared<sf::Texture>();
    if (!texture->loadFromFile(path)) {
        std::cerr << "[MyVideoModule] Failed to load texture: " 
                  << path << std::endl;
        return false;
    }

    textures_[id] = texture;
    std::cout << "[MyVideoModule] Loaded texture: " << id 
              << " from " << path << std::endl;
    return true;
}

const void *MyVideoModule::GetTexture(const std::string &id) const {
    auto it = textures_.find(id);
    if (it == textures_.end()) {
        return nullptr;
    }
    return it->second.get();
}

Vector2f MyVideoModule::GetTextureSize(const std::string &id) const {
    auto it = textures_.find(id);
    if (it == textures_.end()) {
        return {0.0f, 0.0f};
    }
    auto size = it->second->getSize();
    return {static_cast<float>(size.x), static_cast<float>(size.y)};
}

// ===== Font Management =====

bool MyVideoModule::LoadFont(const std::string &id,
                             const std::string &path) {
    auto font = std::make_shared<sf::Font>();
    if (!font->loadFromFile(path)) {
        std::cerr << "[MyVideoModule] Failed to load font: " 
                  << path << std::endl;
        return false;
    }

    fonts_[id] = font;
    std::cout << "[MyVideoModule] Loaded font: " << id 
              << " from " << path << std::endl;
    return true;
}

const void *MyVideoModule::GetFont(const std::string &id) const {
    auto it = fonts_.find(id);
    if (it == fonts_.end()) {
        return nullptr;
    }
    return it->second.get();
}

FloatRect MyVideoModule::GetTextBounds(const std::string &text,
                                       const std::string &font_id,
                                       unsigned int character_size) const {
    auto it = fonts_.find(font_id);
    if (it == fonts_.end()) {
        return {0.0f, 0.0f, 0.0f, 0.0f};
    }

    sf::Text sf_text;
    sf_text.setFont(*it->second);
    sf_text.setString(text);
    sf_text.setCharacterSize(character_size);

    auto bounds = sf_text.getLocalBounds();
    return {bounds.left, bounds.top, bounds.width, bounds.height};
}

// ===== Shader Management =====

bool MyVideoModule::LoadShader(const std::string &id,
                               const std::string &fragment_path) {
    auto shader = std::make_shared<sf::Shader>();
    
    // Load fragment shader only (vertex shader uses default)
    if (!shader->loadFromFile(fragment_path, sf::Shader::Fragment)) {
        std::cerr << "[MyVideoModule] Failed to load shader: " 
                  << fragment_path << std::endl;
        return false;
    }

    shaders_[id] = shader;
    std::cout << "[MyVideoModule] Loaded shader: " << id << std::endl;
    return true;
}

void MyVideoModule::SetShaderParameter(const std::string &shader_id,
                                       const std::string &param_name,
                                       float value) {
    auto it = shaders_.find(shader_id);
    if (it != shaders_.end()) {
        it->second->setUniform(param_name, value);
    }
}

// ===== Identification =====

std::string MyVideoModule::GetModuleName() const {
    return "My Video Module (SFML)";
}

// ===== Helper Functions =====

sf::Color MyVideoModule::ToSFMLColor(const Color &color) {
    return sf::Color(color.r, color.g, color.b, color.a);
}

Event MyVideoModule::FromSFMLEvent(const sf::Event &sf_event) {
    Event event;
    
    // Map SFML event types to engine event types
    switch (sf_event.type) {
        case sf::Event::Closed:
            event.type = EventType::Closed;
            break;
        case sf::Event::KeyPressed:
            event.type = EventType::KeyPressed;
            event.data.key_code = static_cast<int>(sf_event.key.code);
            break;
        case sf::Event::KeyReleased:
            event.type = EventType::KeyReleased;
            event.data.key_code = static_cast<int>(sf_event.key.code);
            break;
        case sf::Event::MouseMoved:
            event.type = EventType::MouseMoved;
            event.data.mouse_x = sf_event.mouseMove.x;
            event.data.mouse_y = sf_event.mouseMove.y;
            break;
        case sf::Event::MouseButtonPressed:
            event.type = EventType::MouseButtonPressed;
            event.data.mouse_button = static_cast<int>(sf_event.mouseButton.button);
            event.data.mouse_x = sf_event.mouseButton.x;
            event.data.mouse_y = sf_event.mouseButton.y;
            break;
        case sf::Event::MouseButtonReleased:
            event.type = EventType::MouseButtonReleased;
            event.data.mouse_button = static_cast<int>(sf_event.mouseButton.button);
            event.data.mouse_x = sf_event.mouseButton.x;
            event.data.mouse_y = sf_event.mouseButton.y;
            break;
        case sf::Event::MouseWheelScrolled:
            event.type = EventType::MouseWheelScrolled;
            event.data.mouse_wheel_delta = sf_event.mouseWheelScroll.delta;
            break;
        default:
            event.type = EventType::Unknown;
            break;
    }

    return event;
}

}  // namespace Engine::Video

// ===== Plugin Entry Point =====

extern "C" {
    std::shared_ptr<Engine::Video::IVideoModule> entryPoint() {
        return std::make_shared<Engine::Video::MyVideoModule>();
    }
}
```

**Implementation Notes:**

1. **Resource Caching:** `LoadTexture` checks if texture already exists to prevent reloading
2. **View Setup:** Sets explicit view in `Initialize()` to prevent screen offset issues
3. **Type Conversion:** Helper functions convert between engine types and SFML types
4. **Error Handling:** Logs errors but doesn't crash on missing resources
5. **Smart Pointers:** Uses `std::shared_ptr` for automatic memory management

## Step 5: Configure CMake Build

**File:** `client/plugins/video/my_video_plugin/CMakeLists.txt`

```cmake
# Find SFML (or your graphics library)
find_package(SFML 2.6 COMPONENTS graphics window system REQUIRED)

# Create shared library
add_library(my_video_module SHARED
    MyVideoModule.cpp
)

# Set output name
set_target_properties(my_video_module PROPERTIES
    OUTPUT_NAME "my_video_module"
    PREFIX ""  # No 'lib' prefix on Linux
)

# Link dependencies
target_link_libraries(my_video_module
    PRIVATE
        sfml-graphics
        sfml-window
        sfml-system
)

# Include engine headers
target_include_directories(my_video_module
    PRIVATE
        ${CMAKE_SOURCE_DIR}/engine/include
        ${CMAKE_SOURCE_DIR}/client/include
)

# C++23 standard
target_compile_features(my_video_module PRIVATE cxx_std_23)

# Install plugin to output directory
install(TARGETS my_video_module
    LIBRARY DESTINATION ${CMAKE_BINARY_DIR}/lib
    RUNTIME DESTINATION ${CMAKE_BINARY_DIR}/lib
)
```

**Key CMake Settings:**

- `SHARED`: Creates a dynamic library (`.so` on Linux, `.dll` on Windows)
- `PREFIX ""`: Removes `lib` prefix (produces `my_video_module.so` not `libmy_video_module.so`)
- `PRIVATE`: Links SFML privately (not exposed to plugin users)

## Step 6: Integrate with Parent CMake

Add to `client/plugins/video/CMakeLists.txt`:

```cmake
# Add your plugin subdirectory
add_subdirectory(my_video_plugin)
```

## Step 7: Build the Plugin

```bash
cd build
cmake ..
cmake --build . --target my_video_module
```

The plugin will be output to `build/lib/my_video_module.so` (or `.dll` on Windows).

## Step 8: Load and Use the Plugin

**In your application:**

```cpp
#include <loader/DLLoader.hpp>
#include <video/IVideoModule.hpp>
#include <iostream>

int main() {
    // Load the plugin
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open("lib/my_video_module.so");
    
    if (!loader.isLoaded()) {
        std::cerr << "Failed to load video plugin!" << std::endl;
        return 1;
    }

    // Get module instance
    auto video_module = loader.getInstance("entryPoint");
    
    if (!video_module) {
        std::cerr << "Failed to get video module instance!" << std::endl;
        return 1;
    }

    std::cout << "Loaded: " << video_module->GetModuleName() << std::endl;

    // Initialize window
    if (!video_module->Initialize(1920, 1080, "R-Type")) {
        std::cerr << "Failed to initialize video module!" << std::endl;
        return 1;
    }

    // Load resources
    video_module->LoadTexture("player", "assets/images/player.png");
    video_module->LoadFont("main_font", "assets/fonts/arial.ttf");

    // Main game loop
    while (video_module->IsOpen()) {
        // Handle events
        Engine::Video::Event event;
        while (video_module->PollEvent(event)) {
            if (event.type == Engine::Video::EventType::Closed) {
                video_module->Close();
            }
        }

        // Render
        video_module->Clear(Engine::Video::Color::Black);

        // Draw sprite
        Engine::Video::Transform transform;
        transform.position = {100.0f, 100.0f};
        transform.scale = {2.0f, 2.0f};
        transform.rotation = 45.0f;
        transform.origin = {16.0f, 16.0f};  // Center of 32x32 sprite
        
        video_module->DrawSprite("player", transform);

        // Draw text
        Engine::Video::Transform text_transform;
        text_transform.position = {10.0f, 10.0f};
        
        video_module->DrawText("main_font", "Hello, World!", 
                              text_transform, 24, Engine::Video::Color::White);

        video_module->Display();
    }

    // Cleanup
    video_module->Shutdown();
    
    return 0;
}
```

## Advanced Topics

### Sprite Sheet Handling

When rendering from sprite sheets, use the `texture_rect` parameter:

```cpp
// Draw a specific frame from a sprite sheet
Engine::Video::FloatRect sprite_frame{0.0f, 0.0f, 32.0f, 32.0f};  // x, y, w, h
video_module->DrawSprite("spritesheet", transform, &sprite_frame);
```

### Shader Effects

Load and apply fragment shaders for visual effects:

```cpp
// Load shader
video_module->LoadShader("wave", "assets/shaders/wave.frag");

// Set shader parameters
video_module->SetShaderParameter("wave", "time", elapsed_time);
video_module->SetShaderParameter("wave", "amplitude", 0.01f);

// Draw with shader
std::string shader_id = "wave";
video_module->DrawSprite("background", transform, nullptr, 
                        Engine::Video::Color::White, &shader_id);
```

### Origin and Transform Calculations

The origin is in **texture coordinates** (pixels), not world coordinates:

```cpp
// For a 64x64 sprite with CENTER origin:
transform.origin = {32.0f, 32.0f};  // Half width, half height

// Position is where the origin point appears in world space
transform.position = {400.0f, 300.0f};  // Center appears at (400, 300)
```

### Performance Optimization

**Texture Caching:**
```cpp
bool MyVideoModule::LoadTexture(const std::string &id, const std::string &path) {
    // Always check if already loaded first!
    if (textures_.find(id) != textures_.end()) {
        return true;  // Already cached
    }
    // ... load texture
}
```

**Batch Rendering:**
For better performance, sort draw calls by texture/shader to minimize state changes.

## Common Issues and Solutions

### Issue: Screen Offset / Misaligned Rendering

**Cause:** Default SFML view doesn't match window size exactly.

**Solution:** Set explicit view in `Initialize()`:
```cpp
sf::View view(sf::FloatRect(0, 0, static_cast<float>(width), static_cast<float>(height)));
window_->setView(view);
```

### Issue: Sprites Appear at Wrong Position

**Cause:** Origin not calculated correctly for sprite sheets.

**Solution:** Use `texture_rect` size for origin, not full texture size:
```cpp
// WRONG: Uses full texture size
auto texture_size = textures_[id]->getSize();
sprite.setOrigin(texture_size.x / 2, texture_size.y / 2);

// CORRECT: Uses sprite frame size
if (texture_rect) {
    sprite.setOrigin(texture_rect->width / 2, texture_rect->height / 2);
}
```

### Issue: Textures Loaded Every Frame

**Cause:** Missing cache check in `LoadTexture()`.

**Solution:** Always check if texture exists before loading:
```cpp
if (textures_.find(id) != textures_.end()) {
    return true;  // Already loaded
}
```

### Issue: Shaders Not Working

**Cause:** Hardware doesn't support shaders or shader compilation failed.

**Solution:** Check if shaders are supported and handle gracefully:
```cpp
if (!sf::Shader::isAvailable()) {
    std::cerr << "Shaders not supported on this hardware" << std::endl;
    return false;
}
```

## Testing Your Plugin

### Unit Test Example

```cpp
#include <gtest/gtest.h>
#include "MyVideoModule.hpp"

TEST(MyVideoModule, Initialization) {
    auto module = std::make_shared<Engine::Video::MyVideoModule>();
    EXPECT_TRUE(module->Initialize(800, 600, "Test"));
    EXPECT_TRUE(module->IsOpen());
    module->Shutdown();
}

TEST(MyVideoModule, TextureLoading) {
    auto module = std::make_shared<Engine::Video::MyVideoModule>();
    module->Initialize(800, 600, "Test");
    
    EXPECT_TRUE(module->LoadTexture("test", "assets/test.png"));
    
    auto size = module->GetTextureSize("test");
    EXPECT_GT(size.x, 0.0f);
    EXPECT_GT(size.y, 0.0f);
    
    module->Shutdown();
}
```

## Comparison with Other Backends

### SDL2 Backend

For SDL2, the main differences would be:

```cpp
// SDL2 includes
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

// Window creation
SDL_Window* window_ = SDL_CreateWindow(title.c_str(), 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    width, height, SDL_WINDOW_SHOWN);
SDL_Renderer* renderer_ = SDL_CreateRenderer(window_, -1, 
    SDL_RENDERER_ACCELERATED);

// Texture loading
SDL_Surface* surface = IMG_Load(path.c_str());
SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer_, surface);
SDL_FreeSurface(surface);
```

### Raylib Backend

For Raylib, it's even simpler:

```cpp
// Window creation
InitWindow(width, height, title.c_str());
SetTargetFPS(60);

// Texture loading
Texture2D texture = LoadTexture(path.c_str());

// Drawing
DrawTextureEx(texture, position, rotation, scale, WHITE);
```

## Best Practices

1. **Always Validate Pointers:** Check if window/resources exist before using
2. **Use Smart Pointers:** Prefer `std::shared_ptr`/`std::unique_ptr` for RAII
3. **Cache Resources:** Never reload textures/fonts unnecessarily
4. **Handle Errors Gracefully:** Log errors but don't crash the application
5. **Document Public API:** Use Doxygen comments for all public methods
6. **Follow Google C++ Style:** Consistent with the rest of the codebase
7. **Test Thoroughly:** Unit tests for resource loading, integration tests for rendering

## Next Steps

- Review the [Plugin Architecture](./architecture.md) for deeper understanding
- Check the [API Reference](./api-reference.md) for complete interface documentation
- See the [Troubleshooting Guide](./troubleshooting.md) for common issues
- Explore existing implementations in `client/plugins/video/sfml/`

## Additional Resources

- **SFML Documentation:** https://www.sfml-dev.org/documentation/
- **SDL2 Documentation:** https://wiki.libsdl.org/
- **Raylib Documentation:** https://www.raylib.com/cheatsheet/cheatsheet.html
- **OpenGL Tutorial:** https://learnopengl.com/ (for custom rendering)
