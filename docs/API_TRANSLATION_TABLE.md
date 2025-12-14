# API Translation Table: Old Engine → New RenderingEngine

## Quick Reference Guide for Code Migration

This document provides a **fast lookup table** for translating old SFML-direct code patterns to the new RenderingEngine + Plugin architecture. Use this during merge conflict resolution.

## ⚠️ CRITICAL: Common Merge Failure Pattern

**If you see "blue screen only" after merging:**

```bash
# Check for unmigrated patterns
grep -r "window_\.draw\|window_\.clear\|loadFromFile" client/ --include="*.cpp"

# If you see ANY matches → You merged unmigrated code
# Fix: Use this table to convert every match
```

**Why this happens:** Old SFML calls coexist with plugin code. Neither works completely.  
**Solution:** Migrate ALL patterns before merging, or use this table to fix each occurrence.

---

## 📋 GameWorld & Initialization

| Old Pattern (SFML Direct) | New Pattern (RenderingEngine) | Notes |
|----------------------------|-------------------------------|-------|
| `sf::RenderWindow window_;` | `Engine::Rendering::RenderingEngine* rendering_engine_;` | Member variable in GameWorld |
| `window_(sf::VideoMode(w, h), title)` | `rendering_engine_` initialized via `SetRenderingEngine()` | Constructor pattern changed |
| `window_.setFramerateLimit(60);` | Handled by plugin initialization | FPS limit in engine config |
| `window_.GetWindow()` | `rendering_engine_` (no direct window access) | Window abstraction removed |
| `window_.isOpen()` | `rendering_engine_->IsWindowOpen()` | Window state check |
| `window_.pollEvent(event)` | `rendering_engine_->PollEvent(event)` | Event polling |
| `window_.close()` | `rendering_engine_->CloseWindow()` | Window closing |

---

## 🎨 Frame Management

| Old Pattern (SFML Direct) | New Pattern (RenderingEngine) | Notes |
|----------------------------|-------------------------------|-------|
| `window_.clear(sf::Color(r, g, b, a));` | `rendering_engine_->BeginFrame(Engine::Graphics::Color(r, g, b, a));` | Frame start |
| `window_.display();` | `rendering_engine_->EndFrame();` | Frame end |
| Manual clear + draw loop + display | `BeginFrame()` + render calls + `EndFrame()` | Explicit frame lifecycle |

---

## 🖼️ Texture & Resource Loading

| Old Pattern (SFML Direct) | New Pattern (RenderingEngine) | Notes |
|----------------------------|-------------------------------|-------|
| `sf::Texture texture;` | `std::string texture_id;` | Store ID, not texture object |
| `texture.loadFromFile(path)` | `rendering_engine_->LoadTexture(id, path)` | Centralized loading |
| `drawable.texture` (member) | `drawable.texture_id` (string) | Reference by ID |
| `sprite.setTexture(texture)` | Handled internally by plugin | No manual sprite setup |
| `texture.getSize()` | `rendering_engine_->GetTextureSize(texture_id)` | Query size by ID |

---

## 🎭 Sprite Rendering

| Old Pattern (SFML Direct) | New Pattern (RenderingEngine) | Notes |
|----------------------------|-------------------------------|-------|
| `sf::Sprite sprite;` | No sprite object needed | RenderingEngine handles it |
| `sprite.setTexture(texture)` | Pass `texture_id` to `RenderSprite()` | No manual setup |
| `sprite.setPosition(x, y)` | Pass position to `RenderSprite()` | No manual setup |
| `sprite.setScale(sx, sy)` | Pass scale to `RenderSprite()` | No manual setup |
| `sprite.setRotation(angle)` | Pass rotation to `RenderSprite()` | No manual setup |
| `sprite.setOrigin(ox, oy)` | Pass origin to `RenderSprite()` | No manual setup |
| `sprite.setColor(color)` | Pass color to `RenderSprite()` | No manual setup |
| `sprite.setTextureRect(rect)` | Pass `FloatRect*` to `RenderSprite()` | No manual setup |
| `window_.draw(sprite)` | `rendering_engine_->RenderSprite(...)` | Single API call |
| `window_.draw(sprite, states)` | Pass shader ID to `RenderSprite()` | Shader handled as parameter |

**Complete Translation:**
```cpp
// OLD (SFML Direct)
drawable->sprite.setPosition(world_position);
drawable->sprite.setScale(world_scale);
drawable->sprite.setRotation(transform->rotationDegrees);
drawable->sprite.setColor(color);
game_world.window_.draw(drawable->sprite);

// NEW (RenderingEngine)
game_world.rendering_engine_->RenderSprite(
    drawable->texture_id,
    world_position,
    world_scale,
    transform->rotationDegrees,
    texture_rect_ptr,
    color,
    origin_offset,
    shader_id_ptr
);
```

---

## 📝 Text Rendering

| Old Pattern (SFML Direct) | New Pattern (RenderingEngine) | Notes |
|----------------------------|-------------------------------|-------|
| `sf::Font font;` | `std::string font_id;` | Store ID, not font object |
| `font.loadFromFile(path)` | `rendering_engine_->LoadFont(id, path)` | Centralized loading |
| `sf::Text text;` | No text object needed | RenderingEngine handles it |
| `text.setFont(font)` | Pass `font_id` to `RenderText()` | No manual setup |
| `text.setString(str)` | Pass string to `RenderText()` | No manual setup |
| `text.setCharacterSize(size)` | Pass size to `RenderText()` | No manual setup |
| `text.setFillColor(color)` | Pass color to `RenderText()` | No manual setup |
| `text.setPosition(x, y)` | Pass position to `RenderText()` | No manual setup |
| `text.setRotation(angle)` | Pass rotation to `RenderText()` | No manual setup |
| `text.setOrigin(ox, oy)` | Pass origin to `RenderText()` | No manual setup |
| `window_.draw(text)` | `rendering_engine_->RenderText(...)` | Single API call |
| `text.getLocalBounds()` | `rendering_engine_->GetTextBounds(...)` | Query via API |

**Complete Translation:**
```cpp
// OLD (SFML Direct)
sf::Text text;
text.setFont(font);
text.setString(content);
text.setCharacterSize(24);
text.setFillColor(sf::Color::White);
text.setPosition(x, y);
game_world.window_.draw(text);

// NEW (RenderingEngine)
game_world.rendering_engine_->RenderText(
    content,
    font_id,
    Engine::Graphics::Vector2f(x, y),
    1.0f,
    0.0f,
    24,
    Engine::Graphics::Color(255, 255, 255, 255),
    Engine::Graphics::Vector2f(0, 0)
);
```

---

## 🎨 Shaders

| Old Pattern (SFML Direct) | New Pattern (RenderingEngine) | Notes |
|----------------------------|-------------------------------|-------|
| `sf::Shader shader;` | `std::string shader_id;` | Store ID, not shader object |
| `shader.loadFromFile(path, type)` | `rendering_engine_->LoadShader(id, vert_path, frag_path)` | Centralized loading |
| `shader.setUniform(name, value)` | `rendering_engine_->SetShaderParameter(id, name, value)` | Type-safe API |
| `sf::RenderStates states; states.shader = &shader;` | Pass `shader_id` to render calls | No manual states |
| `window_.draw(sprite, states)` | `RenderSprite(..., shader_id_ptr)` | Shader as parameter |

---

## 🎮 Input Handling

| Old Pattern (SFML Direct) | New Pattern (RenderingEngine) | Notes |
|----------------------------|-------------------------------|-------|
| `sf::Event event;` | `Engine::Graphics::Event event;` | Abstract event type |
| `window_.pollEvent(event)` | `rendering_engine_->PollEvent(event)` | Event polling |
| `sf::Keyboard::isKeyPressed(key)` | `rendering_engine_->IsKeyPressed(key)` | Key state check |
| `sf::Mouse::getPosition(window_)` | `rendering_engine_->GetMousePosition()` | Mouse position |

---

## 📐 Types & Enums

| Old Type (SFML) | New Type (Abstract) | Notes |
|-----------------|---------------------|-------|
| `sf::RenderWindow` | `Engine::Rendering::RenderingEngine` | No direct window access |
| `sf::Sprite` | N/A (handled internally) | No sprite objects in game code |
| `sf::Texture` | `std::string texture_id` | Reference by ID |
| `sf::Font` | `std::string font_id` | Reference by ID |
| `sf::Shader` | `std::string shader_id` | Reference by ID |
| `sf::Text` | N/A (handled internally) | No text objects in game code |
| `sf::Color` | `Engine::Graphics::Color` | RGBA struct |
| `sf::Vector2f` | `Engine::Graphics::Vector2f` | 2D vector |
| `sf::IntRect` | `Engine::Graphics::IntRect` | Integer rectangle |
| `sf::FloatRect` | `Engine::Graphics::FloatRect` | Float rectangle |
| `sf::RenderStates` | N/A (handled internally) | Render states abstracted |
| `sf::Event` | `Engine::Graphics::Event` | Event union |

---

## 🔧 Common Drawable Component Changes

| Old Member (SFML) | New Member (Plugin) | Notes |
|-------------------|---------------------|-------|
| `sf::Texture texture;` | `std::string texture_id;` | Store ID |
| `sf::Sprite sprite;` | Removed | No sprite object |
| `std::string spritePath;` | `std::string spritePath;` | Unchanged |
| `sf::IntRect textureRect;` | `Engine::Graphics::IntRect texture_rect;` | Type changed |
| `sf::Color color;` | `Engine::Graphics::Color color;` | Type changed |
| `bool isLoaded;` | May be removed (plugin handles loading) | Check with system |

---

## 📦 Initialization Sequence

### Old Initialization (SFML Direct)
```cpp
// main.cpp (OLD)
GameWorld game_world("R-Type", 1920, 1080);  // Creates sf::RenderWindow internally
// ... game loop with window_.clear(), window_.draw(), window_.display()
```

### New Initialization (Plugin + RenderingEngine)
```cpp
// main.cpp (NEW)
// 1. Load plugin
Engine::DLLoader<Engine::Video::IVideoModule> loader;
loader.open("lib/sfml_video_module.so");
auto video_module = loader.getInstance("entryPoint");

// 2. Create RenderingEngine
auto rendering_engine = 
    std::make_unique<Engine::Rendering::RenderingEngine>(video_module);

// 3. Initialize window
rendering_engine->Initialize(1920, 1080, "R-Type");

// 4. Set in GameWorld
GameWorld game_world;
game_world.SetRenderingEngine(rendering_engine.get());

// ... game loop with BeginFrame(), render calls, EndFrame()
```

---

## 🔍 System Function Changes

### DrawableSystem Pattern

| Old Pattern | New Pattern | Location |
|-------------|-------------|----------|
| `InitializeDrawable(drawable, transform)` | Removed (plugin loads textures) | Initialization moved to main |
| `DrawSprite(game_world, sprite, ...)` | `RenderSprite(...)` | Single API call |
| `RenderOneEntity(..., index)` | `RenderOneEntity(..., index)` | Simplified implementation |
| Direct sprite manipulation | Pass parameters to RenderSprite | No sprite objects |

### Old System Structure (348 lines)
```cpp
void InitializeDrawable(...) { texture.loadFromFile(...); }
void DrawSprite(...) { window_.draw(sprite); }
void RenderOneEntity(...) { sprite.setPosition(...); DrawSprite(...); }
void DrawableSystem(...) { InitializeDrawable(...); RenderOneEntity(...); }
```

### New System Structure (140 lines)
```cpp
void RenderOneEntity(...) { 
    // Calculate game logic
    // Call rendering_engine_->RenderSprite(...)
}
void DrawableSystem(...) { RenderOneEntity(...); }
```

---

## ⚠️ Common Pitfalls

| Anti-Pattern | Correct Pattern | Why |
|--------------|-----------------|-----|
| Accessing `game_world.window_` | Use `game_world.rendering_engine_` | Window is abstracted |
| Creating `sf::Sprite` objects | Pass parameters to `RenderSprite()` | Sprites handled internally |
| Manual `loadFromFile()` | Use `LoadTexture()` at startup | Centralized resource management |
| Storing `sf::Texture` | Store `std::string texture_id` | Resource managed by plugin |
| `window_.draw(sprite)` | `rendering_engine_->RenderSprite(...)` | High-level API |
| Using SFML types directly | Use `Engine::Graphics` types | Backend-agnostic |
| Manual clear/display | Use `BeginFrame()`/`EndFrame()` | Frame lifecycle |

---

## 🚀 Quick Migration Checklist

For each file from old engine:

1. **Search for**: `sf::RenderWindow`, `sf::Sprite`, `sf::Texture`, `sf::Font`, `sf::Shader`
2. **Replace with**: `rendering_engine_` pointer, `texture_id`/`font_id`/`shader_id` strings
3. **Search for**: `loadFromFile()`, `setTexture()`, `setPosition()`, `setScale()`, `setRotation()`, `window_.draw()`
4. **Replace with**: `LoadTexture()` (in main), parameters to `RenderSprite()`/`RenderText()`
5. **Search for**: `window_.clear()`, `window_.display()`
6. **Replace with**: `BeginFrame()`, `EndFrame()`
7. **Update includes**: Remove `<SFML/*.hpp>`, add `"rendering/RenderingEngine.hpp"`

---

## 📞 Need Help?

- **Full Migration Guide**: `docs/RENDERING_ENGINE_MIGRATION.md`
- **Pattern Detection Script**: `scripts/detect_old_patterns.sh`
- **Breaking Changes List**: `docs/BREAKING_CHANGES.md`
- **Example Code**: `docs/MIGRATION_EXAMPLES.md`
- **Merge Strategy**: `docs/MERGE_STRATEGY.md`
