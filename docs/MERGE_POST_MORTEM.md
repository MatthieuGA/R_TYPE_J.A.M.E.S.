# Merge Post-Mortem: December 14-15, 2025

## What Went Wrong

### The Situation
- **Branches**: `133-implement-api-like-plugin-for-game-engine` (new plugin API) ← `feature/game_logic` (old SFML API)
- **Goal**: Merge game logic changes into plugin branch
- **What We Did**: Merged first, tried to fix rendering issues afterward
- **Time Wasted**: 6+ hours debugging the wrong problem

### The Symptoms
1. ✅ Window opened (blue background visible)
2. ❌ No sprites rendered (only clear color shown)
3. ❌ Error: "Failed to create texture, maximum is 0x0"
4. ❌ Error: "OpenGL context inactive"

### What We Tried (All Wrong)
1. **Added dummy texture initialization** - Created 1x1 texture to "force OpenGL init"
2. **Added setActive(true) everywhere** - In LoadTexture, Clear, Display, etc.
3. **Added OpenGL ContextSettings** - Explicit OpenGL 3.0 settings
4. **Suspected Wayland issues** - Tested X11, no difference
5. **Suspected SFML library issues** - Created standalone test (worked fine)
6. **Investigated PollEvent** - Thought it deactivated context

### The Real Problem (Obvious in Hindsight)

**Two rendering systems coexisting:**

```cpp
// Old code (merged from feature/game_logic)
void DrawableSystem() {
    drawable->sprite.setPosition(x, y);
    game_world.window_.draw(drawable->sprite);  // ← window_ doesn't exist
}

// New code (already in plugin branch)  
void DrawableSystem() {
    game_world.rendering_engine_->RenderSprite(...);  // ← Never called
}
```

**The old code tried to use removed members. It compiled but did nothing.**

The "OpenGL errors" were a red herring - textures were being loaded via old SFML API (`texture.loadFromFile()`), which bypassed the plugin entirely.

---

## Why This Happened

### 1. Merged Before Migration
- Did NOT run `detect_old_patterns.sh` on source branch first
- Did NOT identify that `feature/game_logic` had 200+ CRITICAL old patterns
- Assumed we could "fix it after merging"

### 2. Chased Symptoms Instead of Root Cause
- Saw "OpenGL context" errors → assumed OpenGL was broken
- Actually: Old API calls coexisting with new API
- Should have searched for `window_.draw` patterns immediately

### 3. Didn't Trust the Plugin
- Plugin abstraction was working perfectly
- We broke it by merging unmigrated code
- Then spent hours "fixing" the plugin that wasn't broken

---

## The Correct Approach

### Phase 1: Pre-Merge Analysis (15 minutes)
```bash
# Check source branch for old patterns
git checkout feature/game_logic
./scripts/detect_old_patterns.sh > patterns.txt

# Count CRITICAL patterns
grep "CRITICAL" patterns.txt | wc -l
# Output: 247 patterns

# DECISION: Don't merge yet - migrate first
```

### Phase 2: Migration Branch (2-3 hours)
```bash
# Create migration branch from source
git checkout -b migrate/game-logic-api feature/game_logic

# Fix all CRITICAL patterns using migration docs
# - Replace window_.draw() → rendering_engine_->RenderSprite()
# - Replace window_.clear() → BeginFrame()
# - Replace window_.display() → EndFrame()
# - Remove sf::Sprite, sf::Texture objects
# - Replace loadFromFile() with centralized LoadTexture()

# Verify migration
./scripts/detect_old_patterns.sh
# Output: 0 CRITICAL patterns

git commit -m "🔧 Migrate game logic to plugin API"
```

### Phase 3: Clean Merge (30 minutes)
```bash
# Merge migrated code into plugin branch
git checkout 133-implement-api-like-plugin-for-game-engine
git merge migrate/game-logic-api

# Resolve any NEW conflicts (not architecture issues)
# Test - should work immediately
```

**Total Time: 3 hours**  
**Actual Time Taken (wrong approach): 6+ hours**

---

## Key Lessons

### 1. "Merge conflict resolution" ≠ "Architecture migration"
- Merge conflicts: Both sides touched same lines
- Architecture migration: One side uses completely different API
- **Fix architecture BEFORE merging**

### 2. Use the detection script
```bash
./scripts/detect_old_patterns.sh
```
- Run on BOTH branches before merging
- If source branch has CRITICAL patterns → migrate first
- Don't assume you can "fix it later"

### 3. Trust the abstraction
- The plugin system was working perfectly
- We broke it by merging unmigrated code
- Don't hack the abstraction - fix the callers

### 4. Symptoms vs Root Cause
- "OpenGL context inactive" → Symptom
- "Texture loading via old API" → Root cause
- **Always check API usage before debugging low-level issues**

---

## Red Flags That Should Have Warned Us

1. ✋ **"Let's just merge and see what breaks"** - NO, detect patterns first
2. ✋ **"The plugin must be broken"** - NO, check if it's being called at all
3. ✋ **"Add more initialization"** - NO, fix the API calls
4. ✋ **"OpenGL context needs fixes"** - NO, that's a symptom
5. ✋ **"It worked before the merge"** - YES, because we broke it by merging

---

## Correct Merge Checklist

- [ ] Run `detect_old_patterns.sh` on source branch
- [ ] If CRITICAL patterns > 0 → create migration branch
- [ ] Use migration docs to fix patterns:
  - [ ] `RENDERING_ENGINE_MIGRATION.md` for understanding
  - [ ] `API_TRANSLATION_TABLE.md` for quick lookups
  - [ ] `BREAKING_CHANGES.md` for priority order
  - [ ] `MIGRATION_EXAMPLES.md` for code templates
- [ ] Verify migration: 0 CRITICAL patterns
- [ ] THEN merge migrated code
- [ ] Test immediately - should work
- [ ] If issues persist → NOW you can debug (not before)

---

## What We're Doing Now

### Correct Approach (Starting Fresh)
```bash
# Reset to before bad merge
git checkout 133-implement-api-like-plugin-for-game-engine
git reset --hard b2969b9  # Before merge commit

# Start over with correct process
git checkout -b migrate/game-logic-api feature/game_logic

# Migrate systematically using docs
# ... (follow Phase 2 above)

# Merge clean code
git checkout 133-implement-api-like-plugin-for-game-engine  
git merge migrate/game-logic-api
```

---

## For Future Reference

**When merging large changes:**
1. ✅ Analyze FIRST (detect patterns, count occurrences)
2. ✅ Migrate source branch if needed
3. ✅ Verify migration (0 CRITICAL patterns)
4. ✅ THEN merge
5. ✅ Test immediately

**Don't:**
- ❌ Merge unmigrated code
- ❌ Chase symptoms before checking root cause
- ❌ Hack the abstraction layer
- ❌ Assume "fix it later" will be faster

**Time saved: 3+ hours**  
**Frustration saved: Immeasurable**

---

## Documentation Updated

Added warnings to:
- ✅ `MERGE_STRATEGY.md` - Added "CRITICAL LESSONS LEARNED" section
- ✅ `BREAKING_CHANGES.md` - Added "Common Mistakes" section
- ✅ `API_TRANSLATION_TABLE.md` - Added "Read This First" warning
- ✅ `RENDERING_ENGINE_MIGRATION.md` - Added failed merge example
- ✅ `MIGRATION_EXAMPLES.md` - Added "WRONG vs RIGHT" example
- ✅ This document - Complete post-mortem

Future developers won't make the same mistake.
