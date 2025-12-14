# Migration Code Examples

## Copy-Paste Ready Transformations

This document provides **complete, side-by-side code examples** for common migration patterns. Use these as templates during merge.

## ⚠️ Before You Start: The Most Important Example

### WRONG: What Happens If You Merge Unmigrated Code

```cpp
// File: DrawableSystem.cpp (AFTER WRONG MERGE)

// Old code path (doesn't work - window_ doesn't exist)
void RenderOneEntity_Old(...) {
    drawable->sprite.setTexture(drawable->texture);
    drawable->sprite.setPosition(x, y);
    game_world.window_.draw(drawable->sprite);  // ❌ FAILS SILENTLY
}

// New code path (correct but never called)
void RenderOneEntity_New(...) {
    game_world.rendering_engine_->RenderSprite(
        drawable->texture_id, position, scale, ...
    );  // ✅ CORRECT but code never gets here
}

// In system: old code path taken
for (auto& drawable : drawables) {
    RenderOneEntity_Old(...);  // ❌ Uses broken path
}
```

**Result:** Blue screen, no rendering, misleading errors

### RIGHT: Fully Migrated Code

```cpp
// File: DrawableSystem.cpp (CORRECT)

// Only new code path exists
void RenderOneEntity(...) {
    game_world.rendering_engine_->RenderSprite(
        drawable->texture_id, position, scale, ...
    );  // ✅ Always called
}

// In system: clean API usage
for (auto& drawable : drawables) {
    RenderOneEntity(...);  // ✅ Uses plugin API
}
```

**Result:** Everything renders correctly

---

## 🎮 Example 1: Basic Sprite Rendering

### Before (Old Engine - SFML Direct)

```cpp
// old_engine/engine/systems/systems_functions/render/DrawableSystem.cpp

#include <SFML/Graphics.hpp>
#include "engine/GameWorld.hpp"
#include "components/Drawable.hpp"
#include "components/Transform.hpp"

void InitializeDrawable(Com::Drawable &drawable, Com::Transform const &transform) {
    // Manual texture loading
    if (!drawable.texture.loadFromFile(drawable.spritePath)) {
        std::cerr << "Failed to load: " << drawable.spritePath << std::endl;
        drawable.isLoaded = false;
        return;
    }
    
    // Manual sprite setup
    drawable.sprite.setTexture(drawable.texture);
    drawable.sprite.setTextureRect(drawable.textureRect);
    drawable.sprite.setOrigin(
        drawable.sprite.getLocalBounds().width / 2,
        drawable.sprite.getLocalBounds().height / 2
    );
    drawable.isLoaded = true;
}

void RenderOneEntity(
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    GameWorld &game_world,
    int index
) {
    auto &drawable = drawables[index];
    auto &transform = transforms[index];
    
    // Manual sprite manipulation
    drawable->sprite.setPosition(transform->position.x, transform->position.y);
    drawable->sprite.setScale(transform->scale.x, transform->scale.y);
    drawable->sprite.setRotation(transform->rotationDegrees);
    
    sf::Color color = drawable->color;
    color.a = static_cast<sf::Uint8>(drawable->opacity * 255);
    drawable->sprite.setColor(color);
    
    // Direct SFML draw
    game_world.window_.draw(drawable->sprite);
}

void DrawableSystem(
    Eng::registry &reg,
    GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables
) {
    for (auto &&[i, transform, drawable] : make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded) {
            InitializeDrawable(drawable, transform);
        }
        RenderOneEntity(transforms, drawables, game_world, i);
    }
}
```

### After (New Engine - RenderingEngine + Plugin)

```cpp
// client/engine/systems/systems_functions/render/DrawableSystem.cpp

#include "rendering/RenderingEngine.hpp"
#include "engine/GameWorld.hpp"
#include "components/Drawable.hpp"
#include "components/Transform.hpp"

// No InitializeDrawable needed - textures loaded in main.cpp

void RenderOneEntity(
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    GameWorld &game_world,
    int index
) {
    auto &drawable = drawables[index];
    auto &transform = transforms[index];
    
    // Calculate position and scale (game logic)
    Engine::Graphics::Vector2f position(transform->position.x, transform->position.y);
    float scale = transform->scale.x;
    
    // Apply opacity
    Engine::Graphics::Color color = drawable->color;
    color.a = static_cast<uint8_t>(drawable->opacity * 255.0f);
    
    // Optional texture rect
    Engine::Graphics::FloatRect* texture_rect_ptr = nullptr;
    Engine::Graphics::FloatRect texture_rect;
    if (drawable->texture_rect.width > 0 && drawable->texture_rect.height > 0) {
        texture_rect = Engine::Graphics::FloatRect(
            static_cast<float>(drawable->texture_rect.left),
            static_cast<float>(drawable->texture_rect.top),
            static_cast<float>(drawable->texture_rect.width),
            static_cast<float>(drawable->texture_rect.height)
        );
        texture_rect_ptr = &texture_rect;
    }
    
    // High-level rendering API
    game_world.rendering_engine_->RenderSprite(
        drawable->texture_id,                    // String ID
        position,
        scale,
        transform->rotationDegrees,
        texture_rect_ptr,
        color,
        Engine::Graphics::Vector2f(0.0f, 0.0f),  // Origin
        nullptr                                   // No shader
    );
}

void DrawableSystem(
    Eng::registry &reg,
    GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables
) {
    for (auto &&[i, transform, drawable] : make_indexed_zipper(transforms, drawables)) {
        RenderOneEntity(transforms, drawables, game_world, i);
    }
}
```

**Key Changes:**
- ❌ Removed: `InitializeDrawable()` function
- ❌ Removed: `sf::Sprite`, `sf::Texture` members
- ❌ Removed: Manual `setPosition()`, `setScale()`, `setRotation()`
- ✅ Added: `texture_id` string reference
- ✅ Added: Single `RenderSprite()` call
- ✅ Changed: Types to `Engine::Graphics` namespace

---

## 📝 Example 2: Text Rendering

### Before (Old Engine - SFML Direct)

```cpp
// old_engine/engine/systems/systems_functions/render/DrawTextSystem.cpp

#include <SFML/Graphics.hpp>

void DrawTextSystem(
    GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::DrawText> &texts
) {
    for (auto &&[i, transform, text] : make_indexed_zipper(transforms, texts)) {
        // Manual font loading (if not loaded)
        if (!text->isLoaded) {
            if (!text->font.loadFromFile(text->fontPath)) {
                std::cerr << "Failed to load font\n";
                continue;
            }
            text->text.setFont(text->font);
            text->isLoaded = true;
        }
        
        // Manual text setup
        text->text.setString(text->content);
        text->text.setCharacterSize(text->character_size);
        text->text.setFillColor(text->color);
        text->text.setPosition(transform->position.x, transform->position.y);
        text->text.setRotation(transform->rotationDegrees);
        
        // Direct draw
        game_world.window_.draw(text->text);
    }
}
```

### After (New Engine - RenderingEngine + Plugin)

```cpp
// client/engine/systems/systems_functions/render/DrawTextSystem.cpp

#include "rendering/RenderingEngine.hpp"

void DrawTextSystem(
    GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::DrawText> &texts
) {
    for (auto &&[i, transform, text] : make_indexed_zipper(transforms, texts)) {
        // Position (game logic)
        Engine::Graphics::Vector2f position(
            transform->position.x + text->offset.x,
            transform->position.y + text->offset.y
        );
        
        // Apply opacity
        Engine::Graphics::Color color = text->color;
        color.a = static_cast<uint8_t>(text->opacity * 255);
        
        // High-level text rendering
        game_world.rendering_engine_->RenderText(
            text->content,                          // String
            text->font_id,                          // Font ID (loaded in main)
            position,
            1.0f,                                   // Scale
            transform->rotationDegrees,
            text->character_size,
            color,
            Engine::Graphics::Vector2f(0.0f, 0.0f)  // Origin
        );
    }
}
```

**Key Changes:**
- ❌ Removed: `sf::Font`, `sf::Text` members
- ❌ Removed: Manual font loading
- ❌ Removed: `setFont()`, `setString()`, `setCharacterSize()`, etc.
- ✅ Added: `font_id` string reference
- ✅ Added: Single `RenderText()` call
- ✅ Simplified: No initialization logic in system

---

## 🎨 Example 3: Shader Application

### Before (Old Engine - SFML Direct)

```cpp
// old_engine - Shader setup and usage

#include <SFML/Graphics.hpp>

// In initialization
sf::Shader shader;
if (!shader.loadFromFile("shaders/wave.vert", "shaders/wave.frag")) {
    std::cerr << "Failed to load shader\n";
}

// In render loop
float time = clock.getElapsedTime().asSeconds();
shader.setUniform("time", time);
shader.setUniform("amplitude", 0.05f);

sf::RenderStates states;
states.shader = &shader;
game_world.window_.draw(sprite, states);
```

### After (New Engine - RenderingEngine + Plugin)

```cpp
// client/main.cpp - Shader loading in initialization

// Load shader once
rendering_engine->LoadShader(
    "wave_shader",
    "shaders/wave.vert",
    "shaders/wave.frag"
);

// In render loop (system)
float time = game_world.total_time_clock_.GetElapsedTime().AsSeconds();
game_world.rendering_engine_->SetShaderParameter("wave_shader", "time", time);
game_world.rendering_engine_->SetShaderParameter("wave_shader", "amplitude", 0.05f);

const std::string shader_id = "wave_shader";
game_world.rendering_engine_->RenderSprite(
    drawable->texture_id,
    position,
    scale,
    rotation,
    nullptr,
    color,
    origin,
    &shader_id  // Pass shader ID
);
```

**Key Changes:**
- ❌ Removed: `sf::Shader` object
- ❌ Removed: Manual shader loading in system
- ❌ Removed: `sf::RenderStates` struct
- ✅ Added: Centralized `LoadShader()` at startup
- ✅ Added: `SetShaderParameter()` for uniforms
- ✅ Added: Pass shader ID to render calls

---

## 🏗️ Example 4: GameWorld Initialization

### Before (Old Engine - SFML Direct)

```cpp
// old_engine/engine/GameWorld.hpp

#include <SFML/Graphics.hpp>

class GameWorld {
private:
    sf::RenderWindow window_;
    sf::Clock clock_;
    float last_delta_;
    
public:
    GameWorld(std::string const &title, int width, int height)
        : window_(sf::VideoMode(width, height), title),
          last_delta_(0.0f) {
        window_.setFramerateLimit(60);
    }
    
    sf::RenderWindow& GetWindow() { return window_; }
    
    void ClearWindow() {
        window_.clear(sf::Color(30, 30, 80, 255));
    }
    
    void DisplayWindow() {
        window_.display();
    }
};

// old_engine/main.cpp

int main() {
    GameWorld game_world("R-Type", 1920, 1080);
    
    while (game_world.GetWindow().isOpen()) {
        game_world.ClearWindow();
        
        // ... game logic and rendering ...
        
        game_world.DisplayWindow();
    }
    
    return 0;
}
```

### After (New Engine - RenderingEngine + Plugin)

```cpp
// client/engine/GameWorld.hpp

#include "rendering/RenderingEngine.hpp"

class GameWorld {
private:
    Engine::Rendering::RenderingEngine* rendering_engine_;
    float last_delta_;
    
public:
    GameWorld() : rendering_engine_(nullptr), last_delta_(0.0f) {}
    
    void SetRenderingEngine(Engine::Rendering::RenderingEngine* engine) {
        rendering_engine_ = engine;
    }
    
    Engine::Rendering::RenderingEngine* GetRenderingEngine() {
        return rendering_engine_;
    }
    
    // Expose rendering_engine_ as public or via getter
    Engine::Rendering::RenderingEngine* rendering_engine_;
};

// client/main.cpp

int main() {
    // Load plugin
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open("lib/sfml_video_module.so");
    auto video_module = loader.getInstance("entryPoint");
    
    // Create RenderingEngine
    auto rendering_engine = 
        std::make_unique<Engine::Rendering::RenderingEngine>(video_module);
    rendering_engine->Initialize(1920, 1080, "R-Type");
    
    // Create GameWorld
    GameWorld game_world;
    game_world.SetRenderingEngine(rendering_engine.get());
    
    // Load resources
    rendering_engine->LoadTexture("player", "assets/player.png");
    rendering_engine->LoadFont("main_font", "assets/font.ttf");
    
    while (rendering_engine->IsWindowOpen()) {
        rendering_engine->BeginFrame(Engine::Graphics::Color(30, 30, 80, 255));
        
        // ... game logic and rendering ...
        
        rendering_engine->EndFrame();
    }
    
    return 0;
}
```

**Key Changes:**
- ❌ Removed: `sf::RenderWindow window_` member
- ❌ Removed: `GetWindow()` method
- ❌ Removed: `ClearWindow()`, `DisplayWindow()` methods
- ✅ Added: `rendering_engine_` pointer
- ✅ Added: Plugin loading and initialization
- ✅ Added: Centralized resource loading
- ✅ Changed: Frame management to `BeginFrame()`/`EndFrame()`

---

## 🧩 Example 5: Component Structure

### Before (Old Engine - SFML Types)

```cpp
// old_engine/components/Drawable.hpp

#include <SFML/Graphics.hpp>

namespace Com {

struct Drawable {
    sf::Texture texture;        // SFML type
    sf::Sprite sprite;          // SFML type
    sf::IntRect textureRect;    // SFML type
    sf::Color color;            // SFML type
    std::string spritePath;
    bool isLoaded = false;
    int z_index = 0;
    float opacity = 1.0f;
};

}  // namespace Com
```

### After (New Engine - Abstract Types)

```cpp
// client/components/Drawable.hpp

#include "engine/types/Graphics.hpp"

namespace Com {

struct Drawable {
    std::string texture_id;                   // String ID instead of texture
    Engine::Graphics::IntRect texture_rect;   // Abstract type
    Engine::Graphics::Color color;            // Abstract type
    std::string spritePath;                   // Keep for reference
    int z_index = 0;
    float opacity = 1.0f;
    // No sprite member needed
    // No isLoaded needed (plugin handles loading)
};

}  // namespace Com
```

**Key Changes:**
- ❌ Removed: `sf::Texture texture` → `std::string texture_id`
- ❌ Removed: `sf::Sprite sprite` (no sprite objects)
- ❌ Removed: `bool isLoaded` (plugin handles state)
- ✅ Changed: `sf::IntRect` → `Engine::Graphics::IntRect`
- ✅ Changed: `sf::Color` → `Engine::Graphics::Color`

---

## 🎯 Example 6: Event Handling

### Before (Old Engine - SFML Events)

```cpp
// old_engine/main.cpp

#include <SFML/Window.hpp>

sf::Event event;
while (game_world.window_.pollEvent(event)) {
    if (event.type == sf::Event::Closed) {
        game_world.window_.close();
    }
    
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            game_world.window_.close();
        }
    }
    
    if (event.type == sf::Event::MouseButtonPressed) {
        if (event.mouseButton.button == sf::Mouse::Left) {
            // Handle click
        }
    }
}

// Keyboard state
if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
    // Handle space key
}
```

### After (New Engine - Abstract Events)

```cpp
// client/main.cpp

#include "engine/types/Graphics.hpp"

Engine::Graphics::Event event;
while (rendering_engine->PollEvent(event)) {
    if (event.type == Engine::Graphics::Event::Closed) {
        rendering_engine->CloseWindow();
    }
    
    if (event.type == Engine::Graphics::Event::KeyPressed) {
        if (event.key.code == Engine::Graphics::Keyboard::Escape) {
            rendering_engine->CloseWindow();
        }
    }
    
    if (event.type == Engine::Graphics::Event::MouseButtonPressed) {
        if (event.mouseButton.button == Engine::Graphics::Mouse::Left) {
            // Handle click
        }
    }
}

// Keyboard state
if (rendering_engine->IsKeyPressed(Engine::Graphics::Keyboard::Space)) {
    // Handle space key
}
```

**Key Changes:**
- ❌ Changed: `sf::Event` → `Engine::Graphics::Event`
- ❌ Changed: `window_.pollEvent()` → `rendering_engine->PollEvent()`
- ❌ Changed: `sf::Keyboard::isKeyPressed()` → `rendering_engine->IsKeyPressed()`
- ❌ Changed: All enum types to `Engine::Graphics` namespace

---

## 🚀 Example 7: Complete System Transformation

### Before (Old Engine - 348 lines)

```cpp
// old_engine/engine/systems/systems_functions/render/DrawableSystem.cpp
// (Simplified excerpt showing key patterns)

void InitializeDrawable(Com::Drawable &drawable, Com::Transform const &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath)) {
        std::cerr << "Failed to load: " << drawable.spritePath << std::endl;
        drawable.isLoaded = false;
        return;
    }
    drawable.sprite.setTexture(drawable.texture);
    drawable.sprite.setTextureRect(drawable.textureRect);
    drawable.sprite.setOrigin(
        drawable.sprite.getLocalBounds().width / 2,
        drawable.sprite.getLocalBounds().height / 2
    );
    drawable.isLoaded = true;
}

void DrawSprite(GameWorld &game_world, sf::Sprite &sprite, 
                Com::Drawable *drawable, std::optional<Com::Shader> &shader_opt) {
    if (shader_opt) {
        sf::RenderStates states;
        states.shader = &shader_opt->shader;
        game_world.window_.draw(sprite, states);
    } else {
        game_world.window_.draw(sprite);
    }
}

void RenderOneEntity(/* many parameters */, int index) {
    auto &drawable = drawables[index];
    auto &transform = transforms[index];
    
    sf::Vector2f world_position = CalculateCumulativePosition(transform.value(), transforms);
    sf::Vector2f world_scale = CalculateCumulativeScale(transform.value(), transforms);
    
    drawable->sprite.setPosition(world_position);
    drawable->sprite.setScale(world_scale);
    drawable->sprite.setRotation(transform->rotationDegrees);
    
    sf::Color color = drawable->color;
    color.a = static_cast<sf::Uint8>(drawable->opacity * 255);
    drawable->sprite.setColor(color);
    
    std::optional<Com::Shader> shader_opt = shaders.has(i) ? shaders[i] : std::nullopt;
    DrawSprite(game_world, drawable->sprite, &drawable.value(), shader_opt);
}

void DrawableSystem(Eng::registry &reg, GameWorld &game_world, /* ... */) {
    std::vector<RenderItem> render_order;
    
    // Collect and sort entities
    for (auto &&[i, transform, drawable] : make_indexed_zipper(transforms, drawables)) {
        if (!drawable.isLoaded)
            InitializeDrawable(drawable, transform);
        render_order.push_back({i, drawable.z_index, false});
    }
    
    std::sort(render_order.begin(), render_order.end(), /* ... */);
    
    // Render in sorted order
    for (const auto &item : render_order) {
        RenderOneEntity(transforms, drawables, shaders, game_world, item.index);
    }
}
```

### After (New Engine - 140 lines)

```cpp
// client/engine/systems/systems_functions/render/DrawableSystem.cpp
// (Complete simplified version)

void RenderOneEntity(
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders,
    GameWorld &game_world,
    int index
) {
    auto &drawable = drawables[index];
    auto &transform = transforms[index];
    
    // Calculate world transforms (game logic)
    Engine::Graphics::Vector2f world_position =
        CalculateWorldPositionWithHierarchy(transform.value(), transforms);
    float world_scale = CalculateCumulativeScale(transform.value(), transforms);
    
    // Calculate sprite size for origin
    Engine::Graphics::Vector2f sprite_size;
    if (drawable->texture_rect.width > 0 && drawable->texture_rect.height > 0) {
        sprite_size = Engine::Graphics::Vector2f(
            static_cast<float>(drawable->texture_rect.width),
            static_cast<float>(drawable->texture_rect.height));
    } else {
        sprite_size = game_world.rendering_engine_->GetTextureSize(drawable->texture_id);
    }
    Engine::Graphics::Vector2f origin_offset =
        GetOffsetFromTransform(transform.value(), sprite_size);
    
    // Apply opacity
    Engine::Graphics::Color final_color = drawable->color;
    final_color.a = static_cast<uint8_t>(drawable->opacity * 255.0f);
    
    // Texture rect
    Engine::Graphics::FloatRect *texture_rect_ptr = nullptr;
    Engine::Graphics::FloatRect texture_rect;
    if (drawable->texture_rect.width > 0 && drawable->texture_rect.height > 0) {
        texture_rect = Engine::Graphics::FloatRect(
            static_cast<float>(drawable->texture_rect.left),
            static_cast<float>(drawable->texture_rect.top),
            static_cast<float>(drawable->texture_rect.width),
            static_cast<float>(drawable->texture_rect.height));
        texture_rect_ptr = &texture_rect;
    }
    
    // Shader handling
    const std::string *shader_id_ptr = nullptr;
    std::string shader_id_str;
    if (shaders.has(index) && shaders[index]->is_loaded) {
        shader_id_str = shaders[index]->shader_id;
        shader_id_ptr = &shader_id_str;
        
        float time = game_world.total_time_clock_.GetElapsedTime().AsSeconds();
        game_world.rendering_engine_->SetShaderParameter(shader_id_str, "time", time);
    }
    
    // Single rendering call
    game_world.rendering_engine_->RenderSprite(
        drawable->texture_id,
        world_position,
        world_scale,
        transform->rotationDegrees,
        texture_rect_ptr,
        final_color,
        origin_offset,
        shader_id_ptr
    );
}

void DrawableSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Transform> const &transforms,
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::Shader> &shaders) {
    
    std::vector<RenderItem> render_order;
    
    // Collect entities
    for (auto &&[i, transform, drawable] : make_indexed_zipper(transforms, drawables)) {
        render_order.push_back({i, drawable.z_index, false});
    }
    
    // Sort by z-index
    std::sort(render_order.begin(), render_order.end(),
        [](const RenderItem &a, const RenderItem &b) {
            return a.z_index < b.z_index;
        });
    
    // Render in sorted order
    for (const auto &item : render_order) {
        RenderOneEntity(transforms, drawables, shaders, game_world, item.index);
    }
}
```

**Key Changes:**
- ❌ Removed: `InitializeDrawable()` (60 lines)
- ❌ Removed: `DrawSprite()` helper (20 lines)
- ❌ Removed: Manual sprite manipulation (30 lines)
- ✅ Simplified: One `RenderSprite()` call handles everything
- ✅ Reduced: 348 lines → 140 lines (-60%)

---

## 📚 Additional Resources

- **API Translation Table**: `docs/API_TRANSLATION_TABLE.md` - Quick lookup for all patterns
- **Breaking Changes**: `docs/BREAKING_CHANGES.md` - Priority checklist
- **Full Migration Guide**: `docs/RENDERING_ENGINE_MIGRATION.md` - Complete architectural overview
- **Pattern Detection**: `./scripts/detect_old_patterns.sh` - Automatically find old code
- **Merge Strategy**: `docs/MERGE_STRATEGY.md` - Step-by-step merge process
