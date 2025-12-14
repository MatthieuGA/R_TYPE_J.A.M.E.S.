# Merge Strategy Guide

## Step-by-Step Process for Merging 4000+ Line Changes

This guide provides a **systematic approach** to merging significant changes from the old SFML-based engine to the new RenderingEngine + Plugin architecture.

---

## 🎯 Overview

**Challenge**: Your colleague has ~4000 insertions based on the old SFML-direct architecture
**Goal**: Merge these changes into the new plugin-based architecture with minimal conflicts
**Timeline**: 3-5 days (depends on complexity)

## ⚠️ CRITICAL LESSONS LEARNED

### What NOT To Do (From Real Failed Merge)

**❌ DON'T merge blindly and try to fix rendering issues afterward**
- **Problem**: Merged game logic branch into plugin branch, sprites didn't render
- **Symptom**: "Only blue background visible", OpenGL context errors
- **Wrong Fix Attempted**: Added dummy texture initialization, hacked OpenGL context activation
- **Why It Failed**: We were treating the symptom (OpenGL errors), not the cause (incomplete API migration)
- **Time Wasted**: 6+ hours debugging fake OpenGL issues

**✅ DO migrate systematically BEFORE merging**
- **Correct Approach**: Use `detect_old_patterns.sh` FIRST, fix all CRITICAL issues, THEN merge
- **Why It Works**: Old code uses `window_.draw()`, new code uses `RenderSprite()` - these are fundamentally incompatible
- **Key Insight**: The plugin abstraction was already working perfectly - we broke it by merging unmigrated code

### The Real Problem

```cpp
// Old code (what was merged) - f0fdf40
void DrawableSystem() {
    drawable->sprite.setPosition(x, y);
    game_world.window_.draw(drawable->sprite);  // ← This doesn't exist anymore!
}

// New code (what we should have) - plugin branch
void DrawableSystem() {
    game_world.rendering_engine_->RenderSprite(
        drawable->texture_id, position, scale, rotation, ...
    );  // ← Completely different API!
}
```

**The merge brought in direct SFML calls that bypassed the plugin entirely.**

### What You'll Actually See

When you merge unmigrated code, you'll get:
1. **Compilation errors** (best case) - missing `window_` member
2. **Silent failures** (worst case) - code compiles but nothing renders
3. **Misleading errors** - "OpenGL context inactive", "max texture size 0x0"
   - These are symptoms, not root causes!
   - Real cause: Textures loaded via old API, sprites drawn via old API, plugin never used

### The Right Order

```
1. Detect old patterns in colleague's branch
2. Migrate old patterns to new API
3. Verify migration (no CRITICAL patterns remain)
4. THEN merge
5. Fix only NEW merge conflicts (not architecture issues)
```

---

## 📋 Pre-Merge Preparation

### Step 0: Setup (30 minutes)

1. **Create a dedicated merge branch:**
   ```bash
   git checkout -b merge/colleague-changes
   ```

2. **Run pattern detection on current codebase:**
   ```bash
   ./scripts/detect_old_patterns.sh > current_patterns.txt
   ```
   Should show minimal issues in new architecture.

3. **Document current architecture:**
   ```bash
   tree -I 'build|vcpkg|node_modules' -L 3 > architecture_new.txt
   ```

4. **Review migration resources:**
   - Read: `docs/RENDERING_ENGINE_MIGRATION.md`
   - Bookmark: `docs/API_TRANSLATION_TABLE.md`
   - Review: `docs/BREAKING_CHANGES.md`
   - Study: `docs/MIGRATION_EXAMPLES.md`

5. **Identify merge scope:**
   ```bash
   # Get diff statistics from colleague's branch
   git fetch origin colleague-branch-name
   git diff --stat origin/colleague-branch-name
   ```

---

## 📊 Phase 1: Analysis (1-2 hours)

### Step 1.1: Identify Changed Files

```bash
# List all files changed by colleague
git diff --name-only 133-implement-api-like-plugin-for-game-engine..feature/game_logic > changed_files.txt

# Categorize by type
grep '\.hpp' changed_files.txt > changed_headers.txt
grep '\.cpp' changed_files.txt > changed_sources.txt
grep 'components/' changed_files.txt > changed_components.txt
grep 'systems/' changed_files.txt > changed_systems.txt
```

### Step 1.2: Analyze Old Patterns (CRITICAL STEP)

```bash
# Check colleague's branch for old patterns
git checkout feature/game_logic
./scripts/detect_old_patterns.sh > colleague_patterns.txt

# Count severity
echo "CRITICAL patterns (must fix):"
grep "CRITICAL" colleague_patterns.txt | wc -l

echo "WARNING patterns (should fix):"
grep "WARNING" colleague_patterns.txt | wc -l

# Identify specific files with CRITICAL issues
grep -B2 "CRITICAL" colleague_patterns.txt | grep "File:" | sort -u > critical_files.txt

git checkout merge/game-logic-to-plugin
```

**⚠️ DECISION POINT**: 
- **If CRITICAL patterns > 50**: Migrate colleague's branch FIRST, then merge
- **If CRITICAL patterns < 50**: Can merge then fix (but harder)
- **If CRITICAL patterns = 0**: Safe to merge directly

Review `colleague_patterns.txt` to understand scope:
- Count CRITICAL issues (sf::RenderWindow, window_.draw, etc.)
- Count WARNING issues (sprite manipulation, etc.)
- Estimate migration effort per file (5-15 min per CRITICAL pattern)

### Step 1.3: Create Migration Plan

Create a spreadsheet or document with columns:
- **File Path**
- **Category** (System/Component/GameWorld/Main)
- **Priority** (P1-Critical, P2-High, P3-Medium, P4-Low)
- **Old Patterns Found** (count from detect script)
- **Estimated Time** (minutes per file)
- **Status** (Not Started/In Progress/Complete)

**Priority Assignment:**
- **P1**: GameWorld, main.cpp, core systems (DrawableSystem, DrawTextSystem)
- **P2**: Supporting systems, components
- **P3**: Helper functions, utilities
- **P4**: Comments, documentation

---

## 🔧 Phase 2: Structural Changes (2-4 hours)

### Step 2.1: Merge Component Definitions First

**Why**: Components are data structures - easiest to merge, required by systems.

```bash
# For each component file in changed_components.txt:
git show origin/colleague-branch-name:path/to/Component.hpp > /tmp/old_component.hpp
```

**Migration Pattern:**
1. Open new component file and old component file side-by-side
2. Compare structures
3. Update types:
   - `sf::Texture` → `std::string texture_id`
   - `sf::Sprite` → Remove (no sprite objects)
   - `sf::Color` → `Engine::Graphics::Color`
   - `sf::Vector2f` → `Engine::Graphics::Vector2f`
   - `sf::IntRect` → `Engine::Graphics::IntRect`
4. Add new fields from colleague's version
5. Remove SFML-specific members

**Example:**
```cpp
// Colleague's version (old)
struct Drawable {
    sf::Texture texture;
    sf::Sprite sprite;
    sf::Color color;
    std::string new_field;  // ← New field to preserve!
};

// Your merged version (new)
struct Drawable {
    std::string texture_id;           // ← Converted
    Engine::Graphics::Color color;    // ← Converted
    std::string new_field;            // ← Preserved!
};
```

### Step 2.2: Merge GameWorld Changes

**Critical File**: `client/engine/GameWorld.hpp`

```bash
git show origin/colleague-branch-name:client/engine/GameWorld.hpp > /tmp/old_gameworld.hpp
```

**Migration Checklist:**
- [ ] Replace `sf::RenderWindow window_` with `rendering_engine_*`
- [ ] Remove `GetWindow()` method
- [ ] Add any new game state fields from colleague
- [ ] Add any new methods (but update window access)
- [ ] Preserve game logic additions

**Example:**
```cpp
// Colleague's version (old) - added health system
class GameWorld {
private:
    sf::RenderWindow window_;
    int player_health_;  // ← New field!

public:
    void SetPlayerHealth(int health) { player_health_ = health; }  // ← New method!
    sf::RenderWindow& GetWindow() { return window_; }
};

// Your merged version (new)
class GameWorld {
private:
    Engine::Rendering::RenderingEngine* rendering_engine_;
    int player_health_;  // ← Preserved!

public:
    void SetPlayerHealth(int health) { player_health_ = health; }  // ← Preserved!
    Engine::Rendering::RenderingEngine* GetRenderingEngine() { return rendering_engine_; }
};
```

### Step 2.3: Merge main.cpp / Application Entry Point

**Critical File**: `client/main.cpp`

```bash
git show origin/colleague-branch-name:client/main.cpp > /tmp/old_main.cpp
```

**Migration Strategy:**
1. Keep new plugin loading code (do NOT merge old window creation)
2. Preserve colleague's game initialization logic
3. Update frame management calls
4. Add resource loading for new assets

**Example:**
```cpp
// Colleague's version (old)
int main() {
    GameWorld game_world("R-Type", 1920, 1080);

    // New initialization logic to preserve!
    game_world.LoadLevel("level_1.json");
    game_world.SetPlayerHealth(100);

    while (game_world.GetWindow().isOpen()) {
        game_world.GetWindow().clear(sf::Color::Black);
        // ... game loop ...
        game_world.GetWindow().display();
    }
}

// Your merged version (new)
int main() {
    // Keep new plugin code
    Engine::DLLoader<Engine::Video::IVideoModule> loader;
    loader.open("lib/sfml_video_module.so");
    auto video_module = loader.getInstance("entryPoint");

    auto rendering_engine =
        std::make_unique<Engine::Rendering::RenderingEngine>(video_module);
    rendering_engine->Initialize(1920, 1080, "R-Type");

    GameWorld game_world;
    game_world.SetRenderingEngine(rendering_engine.get());

    // Preserve colleague's initialization logic!
    game_world.LoadLevel("level_1.json");
    game_world.SetPlayerHealth(100);

    // Load any new assets mentioned in colleague's code
    rendering_engine->LoadTexture("new_enemy", "assets/new_enemy.png");

    while (rendering_engine->IsWindowOpen()) {
        rendering_engine->BeginFrame(Engine::Graphics::Color(0, 0, 0, 255));
        // ... game loop ...
        rendering_engine->EndFrame();
    }
}
```

---

## 🎨 Phase 3: System Migration (4-8 hours)

### Step 3.1: Process Each System File

For each system in `changed_systems.txt`:

**Workflow:**
1. Run detection script on old version
2. Identify patterns to convert
3. Apply transformations
4. Test incrementally

**Script Usage:**
```bash
# Extract colleague's version
git show origin/colleague-branch-name:path/to/System.cpp > /tmp/old_system.cpp

# Check patterns in old version
grep -n "window_\|sf::\|loadFromFile" /tmp/old_system.cpp
```

### Step 3.2: Priority Order for Systems

**P1 - Critical Systems (Start Here):**
1. `DrawableSystem.cpp` - Main rendering
2. `DrawTextSystem.cpp` - Text rendering
3. Any system with window access

**P2 - Supporting Systems:**
4. `AnimationSystem.cpp`
5. `ParallaxSystem.cpp`
6. `InitializeShaderSystem.cpp`

**P3 - Game Logic Systems:**
7. `MovementSystem.cpp` (usually no rendering changes)
8. `CollisionSystem.cpp` (usually no rendering changes)
9. Other gameplay systems

### Step 3.3: System Migration Template

For each system, follow this pattern:

**A. Remove Initialization Functions:**
```cpp
// OLD - DELETE THIS
void InitializeDrawable(Com::Drawable &drawable, Com::Transform const &transform) {
    drawable.texture.loadFromFile(drawable.spritePath);
    drawable.sprite.setTexture(drawable.texture);
    // ... manual setup ...
}
```

**B. Update RenderOneEntity / Render Functions:**

Use `docs/MIGRATION_EXAMPLES.md` as template. Pattern:
1. Keep game logic (position calculation, hierarchy)
2. Replace sprite manipulation with RenderSprite() call
3. Update types to Engine::Graphics namespace

**C. Preserve Colleague's Game Logic:**
```cpp
// OLD - Colleague added new feature
if (drawable.is_flashing) {  // ← New feature!
    color.a = static_cast<uint8_t>((std::sin(time * 5.0f) * 0.5f + 0.5f) * 255);
}

// NEW - Preserve the feature!
if (drawable.is_flashing) {  // ← Preserved!
    color.a = static_cast<uint8_t>((std::sin(time * 5.0f) * 0.5f + 0.5f) * 255);
}
```

**D. Update Rendering Calls:**
```cpp
// OLD
drawable->sprite.setPosition(pos);
drawable->sprite.setScale(scale);
drawable->sprite.setRotation(rot);
game_world.window_.draw(drawable->sprite);

// NEW
game_world.rendering_engine_->RenderSprite(
    drawable->texture_id, pos, scale, rot, rect_ptr, color, origin, shader_ptr
);
```

### Step 3.4: Asset Loading Migration

Track new assets mentioned in colleague's code:
```bash
# Find all loadFromFile calls in colleague's branch
git grep "loadFromFile" origin/colleague-branch-name | grep -v "^Binary"
```

Create a mapping file `asset_loading.txt`:
```
OLD: drawable.texture.loadFromFile("assets/enemy.png")
NEW: rendering_engine->LoadTexture("enemy", "assets/enemy.png") [in main.cpp]

OLD: font.loadFromFile("assets/font.ttf")
NEW: rendering_engine->LoadFont("main_font", "assets/font.ttf") [in main.cpp]
```

Add all to main.cpp initialization:
```cpp
// In main.cpp after rendering_engine creation
rendering_engine->LoadTexture("enemy", "assets/enemy.png");
rendering_engine->LoadTexture("power_up", "assets/power_up.png");
rendering_engine->LoadFont("main_font", "assets/font.ttf");
rendering_engine->LoadShader("wave", "shaders/wave.vert", "shaders/wave.frag");
```

---

## ✅ Phase 4: Validation & Testing (2-3 hours)

### Step 4.1: Incremental Compilation

After each system migration:
```bash
cmake --build build -j$(nproc)
```

**Fix compilation errors immediately**. Use API Translation Table for quick lookup.

### Step 4.2: Pattern Detection Verification

```bash
./scripts/detect_old_patterns.sh
```

**Goal**: Zero CRITICAL issues, minimal WARNING issues.

If issues found:
- Review each occurrence
- Apply transformations from `docs/API_TRANSLATION_TABLE.md`
- Re-run script

### Step 4.3: Visual Testing

```bash
./build/client/r-type_client
```

**Test Checklist:**
- [ ] Window opens
- [ ] Graphics render correctly
- [ ] Text displays properly
- [ ] Shaders work (if used)
- [ ] Animations play
- [ ] No crashes
- [ ] Performance is acceptable

### Step 4.4: Unit Tests

```bash
cd build
ctest --output-on-failure
```

Fix any failing tests. Update test code if it uses old patterns.

### Step 4.5: Code Review

**Self-Review Checklist:**
- [ ] No `#include <SFML/` in game code
- [ ] No `sf::` types in game code (except plugin implementation)
- [ ] No `game_world.window_` access
- [ ] All textures loaded via `LoadTexture()` at startup
- [ ] All rendering uses high-level API
- [ ] Colleague's game logic preserved
- [ ] New features functional
- [ ] No dead code (old initialization functions removed)

---

## 🚨 Phase 5: Conflict Resolution (Ongoing)

### Common Merge Conflicts

#### Conflict 1: GameWorld Member Variables

```
<<<<<<< HEAD (your version)
    Engine::Rendering::RenderingEngine* rendering_engine_;
=======
    sf::RenderWindow window_;
    int player_health_;  // New field
>>>>>>> origin/colleague-branch-name
```

**Resolution:**
```cpp
// Take both: your rendering_engine_ + colleague's new fields
Engine::Rendering::RenderingEngine* rendering_engine_;
int player_health_;  // Keep new field
```

#### Conflict 2: Render Function Signatures

```
<<<<<<< HEAD (your version)
void RenderSprite(GameWorld &game_world, std::string texture_id, ...)
=======
void RenderSprite(GameWorld &game_world, sf::Sprite &sprite, ...)
>>>>>>> origin/colleague-branch-name
```

**Resolution:**
```cpp
// Use new signature (high-level API)
void RenderSprite(GameWorld &game_world, std::string texture_id, ...)
// But preserve any new parameters colleague added
```

#### Conflict 3: System Logic

```
<<<<<<< HEAD (your version)
    game_world.rendering_engine_->RenderSprite(...);
=======
    if (drawable.is_invincible) {  // New feature!
        sprite.setColor(sf::Color(255, 255, 255, 128));
    }
    game_world.window_.draw(sprite);
>>>>>>> origin/colleague-branch-name
```

**Resolution:**
```cpp
// Preserve colleague's logic + use new API
if (drawable.is_invincible) {  // Preserved!
    color.a = 128;  // Apply alpha instead
}
game_world.rendering_engine_->RenderSprite(..., color, ...);
```

### Conflict Resolution Strategy

1. **Identify conflict type:**
   - Data structure change
   - API change
   - Game logic addition
   - Resource management

2. **Apply transformation:**
   - Use API Translation Table
   - Preserve colleague's logic
   - Use new architecture patterns

3. **Test immediately:**
   ```bash
   cmake --build build && ./build/client/r-type_client
   ```

4. **Document complex resolutions:**
   ```bash
   echo "Merged invincibility feature: applied alpha to color instead of setColor()" >> merge_notes.txt
   ```

---

## 📝 Phase 6: Documentation & Cleanup (1 hour)

### Step 6.1: Update Comments

Replace old pattern references in comments:
```cpp
// OLD COMMENT: "Draw sprite to window"
// NEW COMMENT: "Render sprite via RenderingEngine"

// OLD COMMENT: "Load texture with loadFromFile()"
// NEW COMMENT: "Texture loaded centrally in main.cpp"
```

### Step 6.2: Remove Dead Code

Search for unused functions:
```bash
grep -r "InitializeDrawable" client/ engine/
grep -r "DrawSprite.*sf::Sprite" client/ engine/
```

If found and unused, delete them.

### Step 6.3: Update Internal Documentation

If colleague added documentation about rendering, update it:
- Replace SFML references with RenderingEngine
- Update code examples
- Add migration notes

### Step 6.4: Create Merge Summary

Document in `MERGE_SUMMARY.md`:
```markdown
# Merge Summary: Colleague's Changes

## Overview
- Source Branch: origin/colleague-branch-name
- Target Branch: main (via merge/colleague-changes)
- Lines Changed: ~4000 insertions
- Duration: 3 days

## Changes Migrated
- Added player health system (GameWorld)
- Added level loading (main.cpp)
- Added invincibility feature (DrawableSystem)
- Added 5 new enemy types (Components + assets)
- Added flashing effect (DrawableSystem)

## Transformations Applied
- GameWorld: sf::RenderWindow → rendering_engine_
- DrawableSystem: 348 → 140 lines (-60%)
- All components: SFML types → Engine::Graphics types
- All textures: Centralized loading in main.cpp
- All rendering: High-level RenderSprite() API

## Testing
- ✅ Compilation successful
- ✅ All unit tests pass
- ✅ Visual verification complete
- ✅ Pattern detection: 0 critical issues

## Known Issues
- None

## Next Steps
- Code review with team
- Merge to main
- Deploy to test environment
```

---

## 🎯 Quick Reference: Daily Workflow

### Day 1: Preparation & Structural Changes
**Goal**: Components + GameWorld + main.cpp migrated
- [ ] Setup merge branch
- [ ] Run analysis (detect patterns, categorize files)
- [ ] Merge component definitions
- [ ] Merge GameWorld changes
- [ ] Merge main.cpp changes
- [ ] Verify compilation

### Day 2: Core Systems Migration
**Goal**: Primary rendering systems migrated
- [ ] Migrate DrawableSystem
- [ ] Migrate DrawTextSystem
- [ ] Migrate shader-related systems
- [ ] Test visual output
- [ ] Verify pattern detection

### Day 3: Supporting Systems & Testing
**Goal**: All systems migrated, tests passing
- [ ] Migrate remaining systems
- [ ] Run full test suite
- [ ] Visual testing
- [ ] Performance testing
- [ ] Code review checklist

### Day 4: Cleanup & Documentation
**Goal**: Production-ready merge
- [ ] Remove dead code
- [ ] Update comments
- [ ] Create merge summary
- [ ] Final testing
- [ ] Create PR

---

## 🆘 Troubleshooting

### Issue: Too Many Conflicts

**Solution**: Merge in smaller chunks
```bash
# Instead of merging entire branch at once:
git cherry-pick <commit-with-components>
# Migrate components
git cherry-pick <commit-with-gameworld>
# Migrate GameWorld
# ... continue incrementally
```

### Issue: Unclear Original Intent

**Solution**: Contact colleague
- Ask about specific features
- Request explanation of complex changes
- Discuss alternative implementations

### Issue: Performance Degradation

**Solution**: Profile and optimize
```bash
# Use profiling tools
valgrind --tool=callgrind ./build/client/r-type_client
# Check for:
# - Excessive LoadTexture calls (should be once at startup)
# - RenderSprite calls in tight loops (expected)
# - Memory leaks (SFML objects not cleaned up)
```

### Issue: Visual Artifacts

**Solution**: Check transformations
- Origin calculation: Compare old sprite.getOrigin() with new origin_offset
- Texture rects: Verify FloatRect conversion
- Colors: Check alpha channel (255 vs 1.0f confusion)
- Shaders: Verify uniform names and types

---

## 📊 Success Metrics

Merge is complete when:
- ✅ **0 compilation errors**
- ✅ **0 CRITICAL pattern detections** (`./scripts/detect_old_patterns.sh`)
- ✅ **All unit tests pass** (`ctest`)
- ✅ **Visual output matches expected** (manual testing)
- ✅ **Code review approved** (team)
- ✅ **Documentation updated** (comments, guides)
- ✅ **Performance acceptable** (FPS, load time)

---

## 📚 Resources Quick Links

| Resource | Purpose | Location |
|----------|---------|----------|
| **API Translation Table** | Quick lookup for pattern replacements | `docs/API_TRANSLATION_TABLE.md` |
| **Breaking Changes** | Priority checklist of must-fix issues | `docs/BREAKING_CHANGES.md` |
| **Migration Examples** | Copy-paste ready code transformations | `docs/MIGRATION_EXAMPLES.md` |
| **Full Migration Guide** | Complete architectural overview | `docs/RENDERING_ENGINE_MIGRATION.md` |
| **Pattern Detection** | Automatically find old patterns | `./scripts/detect_old_patterns.sh` |

---

## 🎉 Post-Merge

After successful merge:
1. **Create PR** with detailed description
2. **Request code review** from team
3. **Run CI/CD pipeline** (if available)
4. **Update project board** / issue tracker
5. **Celebrate!** 🎊 Major architectural migration complete!

---

## 💡 Tips for Success

1. **Communicate Early**: Tell colleague about architectural changes before starting merge
2. **Merge Often**: Don't let branches diverge for weeks
3. **Test Incrementally**: Compile and test after each file migration
4. **Preserve Intent**: Understand colleague's goals, not just code
5. **Document Decisions**: Keep merge notes for future reference
6. **Ask for Help**: Use team for complex conflicts
7. **Be Patient**: Large merges take time - don't rush!

---

**Good luck with your merge!** 🚀

For questions or issues, refer to the full documentation set or reach out to the team.
