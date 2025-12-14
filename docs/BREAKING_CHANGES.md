# Breaking Changes Checklist

## Critical Changes Required for Merge

This document lists **code patterns that will NOT work** in the new architecture and **MUST** be changed. Use this as a priority checklist during merge.

---

## 🔴 Priority 1: CRITICAL - Code Will Not Compile

### ⚠️ CRITICAL LESSON LEARNED

**If you merge code with these patterns, you WILL get:**
- Silent rendering failures (blue screen only)
- Misleading "OpenGL context" errors
- "Maximum texture size 0x0" errors
- Hours of debugging the wrong problem

**Why?** The old code bypasses the plugin entirely. The plugin works fine - it's just never being called!

**Real-world example from failed merge:**
```cpp
// This compiled but DID NOTHING (sprite rendered to non-existent window)
drawable->sprite.setPosition(x, y);
game_world.window_.draw(drawable->sprite);  // window_ doesn't exist

// Plugin was working perfectly, but never used
game_world.rendering_engine_->RenderSprite(...);  // This code was never reached
```

### 1.1 GameWorld Window Access

**❌ BROKEN:**
```cpp
game_world.window_             // Member removed
game_world.GetWindow()         // Method removed
game_world.window_.clear()     // No window_ member
game_world.window_.draw()      // No window_ member
game_world.window_.display()   // No window_ member
```

**✅ FIX:**
```cpp
game_world.rendering_engine_                          // Use rendering_engine_
game_world.rendering_engine_->BeginFrame(color)       // Replace clear()
game_world.rendering_engine_->RenderSprite(...)       // Replace draw()
game_world.rendering_engine_->EndFrame()              // Replace display()
```

**Priority**: 🔴 **CRITICAL** - Fix immediately, code won't compile  
**Estimated Occurrences**: High (every rendering system)
**Detection**: `grep -r "game_world\.window_" --include="*.cpp"`

---

### 1.2 SFML Sprite/Texture Objects

**❌ BROKEN:**
```cpp
sf::Sprite sprite;                    // No sprite objects in new code
sf::Texture texture;                  // No texture objects in new code
drawable.sprite.setPosition(x, y);    // No sprite member
drawable.texture.loadFromFile(path);  // No texture member
window_.draw(sprite);                 // No window access
```

**✅ FIX:**
```cpp
// Store texture ID instead
std::string texture_id;
// Load at initialization (main.cpp)
rendering_engine->LoadTexture("enemy_ship", "assets/enemy.png");
// Render using high-level API
rendering_engine->RenderSprite(texture_id, position, scale, rotation, ...);
```

**Priority**: 🔴 **CRITICAL** - Fix immediately  
**Estimated Occurrences**: Very High (every drawable entity)

---

### 1.3 Manual Resource Loading

**❌ BROKEN:**
```cpp
if (!texture.loadFromFile(path)) {        // No texture member
    std::cerr << "Failed to load" << std::endl;
}
sprite.setTexture(texture);                // No sprite member
```

**✅ FIX:**
```cpp
// Move to initialization phase (main.cpp or scene setup)
rendering_engine->LoadTexture("texture_id", path);
// Systems just reference by ID
drawable.texture_id = "texture_id";
```

**Priority**: 🔴 **CRITICAL** - Must centralize loading  
**Estimated Occurrences**: High (initialization code)

---

### 1.4 Direct SFML Includes

**❌ BROKEN:**
```cpp
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Texture.hpp>
```

**✅ FIX:**
```cpp
#include "rendering/RenderingEngine.hpp"
#include "engine/types/Graphics.hpp"  // For Engine::Graphics types
// Or use GameWorld.hpp which includes RenderingEngine
```

**Priority**: 🔴 **CRITICAL** - Replace all SFML includes  
**Estimated Occurrences**: Medium (header files)

---

## 🟠 Priority 2: HIGH - Architectural Changes Required

### 2.1 Drawable Component Structure

**❌ BROKEN:**
```cpp
struct Drawable {
    sf::Texture texture;      // SFML type
    sf::Sprite sprite;        // SFML type
    sf::IntRect textureRect;  // SFML type
    sf::Color color;          // SFML type
    std::string spritePath;
    bool isLoaded;
};
```

**✅ FIX:**
```cpp
struct Drawable {
    std::string texture_id;              // String ID instead of texture
    // No sprite member needed
    Engine::Graphics::IntRect texture_rect;   // Abstract type
    Engine::Graphics::Color color;            // Abstract type
    std::string spritePath;              // Keep for reference
    // isLoaded may not be needed (plugin handles loading)
    int z_index;
    float opacity;
};
```

**Priority**: 🟠 **HIGH** - Affects component definition  
**Estimated Occurrences**: Low (component definition files)

---

### 2.2 InitializeDrawable Functions

**❌ BROKEN:**
```cpp
void InitializeDrawable(Drawable &drawable, Transform const &transform) {
    if (!drawable.texture.loadFromFile(drawable.spritePath)) {
        // ...
    }
    drawable.sprite.setTexture(drawable.texture);
    drawable.sprite.setTextureRect(drawable.textureRect);
    drawable.sprite.setOrigin(...);
    drawable.isLoaded = true;
}
```

**✅ FIX:**
```cpp
// Remove InitializeDrawable function entirely
// Load textures centrally in main.cpp or scene initialization:
rendering_engine->LoadTexture(drawable.texture_id, drawable.spritePath);
// Systems directly render without initialization
```

**Priority**: 🟠 **HIGH** - Remove initialization functions  
**Estimated Occurrences**: Medium (system files)

---

### 2.3 Manual Sprite Manipulation

**❌ BROKEN:**
```cpp
drawable->sprite.setPosition(world_position);
drawable->sprite.setScale(world_scale);
drawable->sprite.setRotation(transform->rotationDegrees);
drawable->sprite.setColor(color);
drawable->sprite.setOrigin(origin);
DrawSprite(game_world, drawable->sprite, ...);
```

**✅ FIX:**
```cpp
// Pass all parameters to RenderSprite
game_world.rendering_engine_->RenderSprite(
    drawable->texture_id,
    world_position,
    world_scale,
    transform->rotationDegrees,
    &texture_rect,
    color,
    origin,
    shader_id_ptr
);
```

**Priority**: 🟠 **HIGH** - Refactor rendering logic  
**Estimated Occurrences**: Very High (every render function)

---

## 🟡 Priority 3: MEDIUM - Type Changes Required

### 3.1 SFML Type Usage

**❌ BROKEN:**
```cpp
sf::Color color(255, 255, 255, 255);
sf::Vector2f position(100.0f, 200.0f);
sf::IntRect rect(0, 0, 32, 32);
sf::FloatRect bounds = sprite.getLocalBounds();
```

**✅ FIX:**
```cpp
Engine::Graphics::Color color(255, 255, 255, 255);
Engine::Graphics::Vector2f position(100.0f, 200.0f);
Engine::Graphics::IntRect rect(0, 0, 32, 32);
Engine::Graphics::FloatRect bounds = 
    rendering_engine->GetTextBounds(...);  // or GetTextureSize(...)
```

**Priority**: 🟡 **MEDIUM** - Update type references  
**Estimated Occurrences**: High (throughout codebase)

---

### 3.2 Text Rendering

**❌ BROKEN:**
```cpp
sf::Font font;
font.loadFromFile(path);
sf::Text text;
text.setFont(font);
text.setString("Hello");
text.setCharacterSize(24);
text.setFillColor(sf::Color::White);
window_.draw(text);
```

**✅ FIX:**
```cpp
// Load font centrally
rendering_engine->LoadFont("main_font", path);
// Render text with high-level API
rendering_engine->RenderText(
    "Hello",
    "main_font",
    position,
    1.0f,
    0.0f,
    24,
    Engine::Graphics::Color(255, 255, 255, 255),
    origin
);
```

**Priority**: 🟡 **MEDIUM** - Refactor text systems  
**Estimated Occurrences**: Medium (text-related code)

---

### 3.3 Shader Handling

**❌ BROKEN:**
```cpp
sf::Shader shader;
shader.loadFromFile(vertex_path, fragment_path);
shader.setUniform("time", time_value);
sf::RenderStates states;
states.shader = &shader;
window_.draw(sprite, states);
```

**✅ FIX:**
```cpp
// Load shader centrally
rendering_engine->LoadShader("wave_shader", vertex_path, fragment_path);
// Set parameters
rendering_engine->SetShaderParameter("wave_shader", "time", time_value);
// Pass shader ID to render call
const std::string* shader_id = &shader_id_string;
rendering_engine->RenderSprite(..., shader_id);
```

**Priority**: 🟡 **MEDIUM** - Update shader workflow  
**Estimated Occurrences**: Low-Medium (shader systems)

---

## 🟢 Priority 4: LOW - Code Quality Improvements

### 4.1 Frame Management

**⚠️ SUBOPTIMAL:**
```cpp
window_.clear(sf::Color(30, 30, 80));
// ... rendering ...
window_.display();
```

**✅ BETTER:**
```cpp
rendering_engine->BeginFrame(Engine::Graphics::Color(30, 30, 80, 255));
// ... rendering ...
rendering_engine->EndFrame();
```

**Priority**: 🟢 **LOW** - Improves clarity  
**Estimated Occurrences**: Low (game loop)

---

### 4.2 Event Polling

**⚠️ SUBOPTIMAL:**
```cpp
sf::Event event;
while (window_.pollEvent(event)) {
    // ...
}
```

**✅ BETTER:**
```cpp
Engine::Graphics::Event event;
while (rendering_engine->PollEvent(event)) {
    // ...
}
```

**Priority**: 🟢 **LOW** - Type update  
**Estimated Occurrences**: Low (input systems)

---

## 🚨 Common Mistakes & How to Avoid Them

### Mistake #1: "Just merge and fix later"
**Symptom**: Blue screen, no sprites, misleading errors  
**Why It Happens**: Old SFML calls coexist with plugin, neither works  
**Solution**: Migrate BEFORE merging (use `detect_old_patterns.sh`)

### Mistake #2: "OpenGL context is broken"
**Symptom**: "Maximum texture size 0x0", context activation failures  
**Why It Happens**: Textures loaded via old API, not through plugin  
**Solution**: Don't hack OpenGL - fix the API calls  
**Real Case**: Spent 6 hours fixing "OpenGL issues" that were actually unmigrated `loadFromFile()` calls

### Mistake #3: "Plugin initialization must be wrong"
**Symptom**: Window creates but nothing renders  
**Why It Happens**: Plugin initializes fine, but old code uses `window_.draw()` which doesn't exist  
**Solution**: Search for ALL `window_.` patterns and replace

### Mistake #4: "Add setActive(true) everywhere"
**Symptom**: Still not rendering despite context "fixes"  
**Why It Happens**: Context was never the problem - wrong API was  
**Solution**: Don't add hacks - migrate the API properly

## 📊 Quick Priority Summary

| Priority | Severity | Must Fix? | Typical Occurrences |
|----------|----------|-----------|---------------------|
| 🔴 P1: CRITICAL | Won't compile | YES | Very High |
| 🟠 P2: HIGH | Won't work correctly | YES | High |
| 🟡 P3: MEDIUM | Type mismatches | YES | Medium |
| 🟢 P4: LOW | Code quality | Recommended | Low |

---

## 🔧 Merge Strategy by Priority

### Day 1: Fix Critical Issues (P1)
1. Search for `game_world.window_` → Replace with `rendering_engine_`
2. Search for `sf::Sprite`, `sf::Texture` → Refactor to ID-based system
3. Search for `loadFromFile()` → Move to centralized initialization
4. Search for `#include <SFML/` → Replace with Engine includes

**Goal**: Code compiles without errors

### Day 2: Fix Architectural Issues (P2)
1. Update Drawable component structure
2. Remove InitializeDrawable functions
3. Refactor sprite manipulation to RenderSprite() calls
4. Centralize resource loading in main.cpp

**Goal**: Code runs without crashes

### Day 3: Fix Type Issues (P3)
1. Replace `sf::Color` → `Engine::Graphics::Color`
2. Replace `sf::Vector2f` → `Engine::Graphics::Vector2f`
3. Update text rendering to new API
4. Update shader workflow

**Goal**: Code is fully migrated

### Day 4: Code Quality (P4)
1. Update frame management calls
2. Update event polling
3. Run linters and tests
4. Code review

**Goal**: Production-ready code

---

## ✅ Verification Checklist

After merge, verify:

- [ ] No `#include <SFML/` in game code (except plugin implementation)
- [ ] No `sf::` types in game code
- [ ] No `game_world.window_` access
- [ ] No `sf::Sprite` or `sf::Texture` member variables
- [ ] All textures loaded via `LoadTexture()` at initialization
- [ ] All rendering uses `RenderSprite()` / `RenderText()` API
- [ ] Frame lifecycle uses `BeginFrame()` / `EndFrame()`
- [ ] All tests pass
- [ ] Code compiles without warnings
- [ ] Pattern detection script shows no issues: `./scripts/detect_old_patterns.sh`

---

## 📚 Additional Resources

- **API Translation**: `docs/API_TRANSLATION_TABLE.md`
- **Full Migration Guide**: `docs/RENDERING_ENGINE_MIGRATION.md`
- **Code Examples**: `docs/MIGRATION_EXAMPLES.md`
- **Merge Strategy**: `docs/MERGE_STRATEGY.md`
- **Pattern Detection**: Run `./scripts/detect_old_patterns.sh`
