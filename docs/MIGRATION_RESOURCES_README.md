# Rendering Engine Migration Resources

## 📚 Complete Documentation Suite for Merging Old Engine Code

This directory contains **5 comprehensive resources** designed to help merge ~4000 lines of old SFML-based code into the new RenderingEngine + Plugin architecture.

---

## 🎯 Quick Start

**If you're about to merge old engine code, start here:**

1. **Read**: [RENDERING_ENGINE_MIGRATION.md](RENDERING_ENGINE_MIGRATION.md) - Understand the full architectural transformation
2. **Bookmark**: [API_TRANSLATION_TABLE.md](API_TRANSLATION_TABLE.md) - Keep open during merge for quick lookups
3. **Follow**: [MERGE_STRATEGY.md](MERGE_STRATEGY.md) - Step-by-step process for the merge
4. **Run**: `../scripts/detect_old_patterns.sh` - Automatically find old code patterns
5. **Reference**: [MIGRATION_EXAMPLES.md](MIGRATION_EXAMPLES.md) - Copy-paste ready code transformations

---

## 📖 Documentation Overview

### 1. [RENDERING_ENGINE_MIGRATION.md](RENDERING_ENGINE_MIGRATION.md)
**Purpose**: Complete architectural overview  
**Length**: ~700 lines  
**Use When**: Need to understand the full transformation from SFML-direct to plugin-based

**Contents:**
- Architecture evolution (old vs new)
- File structure changes
- API changes with code examples
- GameWorld initialization
- Frame management
- Sprite/text/shader rendering
- Benefits summary
- Migration checklist

**Best For**: Initial reading, architectural understanding, convincing stakeholders

---

### 2. [API_TRANSLATION_TABLE.md](API_TRANSLATION_TABLE.md)
**Purpose**: Fast lookup table for pattern replacements  
**Length**: ~400 lines  
**Use When**: Resolving merge conflicts, converting specific patterns

**Contents:**
- GameWorld & initialization mappings
- Frame management translations
- Texture & resource loading
- Sprite rendering API changes
- Text rendering API changes
- Shader handling
- Types & enums conversions
- Component structure changes
- Common pitfalls
- Quick migration checklist

**Best For**: During active merge, quick reference, IDE side panel

---

### 3. [BREAKING_CHANGES.md](BREAKING_CHANGES.md)
**Purpose**: Priority checklist of must-fix issues  
**Length**: ~550 lines  
**Use When**: Planning merge, prioritizing work, daily standup

**Contents:**
- **Priority 1 (CRITICAL)**: Won't compile - fix immediately
  - GameWorld window access
  - SFML sprite/texture objects
  - Manual resource loading
  - Direct SFML includes
- **Priority 2 (HIGH)**: Architectural changes required
  - Drawable component structure
  - InitializeDrawable functions
  - Manual sprite manipulation
- **Priority 3 (MEDIUM)**: Type changes
  - SFML type usage
  - Text rendering
  - Shader handling
- **Priority 4 (LOW)**: Code quality improvements
- 4-day merge strategy by priority
- Verification checklist

**Best For**: Work planning, task breakdown, progress tracking

---

### 4. [MIGRATION_EXAMPLES.md](MIGRATION_EXAMPLES.md)
**Purpose**: Complete, copy-paste ready code transformations  
**Length**: ~650 lines  
**Use When**: Migrating specific systems, need working code examples

**Contents:**
- **Example 1**: Basic sprite rendering (complete system)
- **Example 2**: Text rendering (before/after)
- **Example 3**: Shader application (initialization + usage)
- **Example 4**: GameWorld initialization (old vs new)
- **Example 5**: Component structure (type conversions)
- **Example 6**: Event handling (SFML → abstract events)
- **Example 7**: Complete DrawableSystem (348 → 140 lines)

**Best For**: Active coding, system migration, learning by example

---

### 5. [MERGE_STRATEGY.md](MERGE_STRATEGY.md)
**Purpose**: Systematic step-by-step merge process  
**Length**: ~700 lines  
**Use When**: Executing the merge, managing multi-day work

**Contents:**
- **Phase 0**: Pre-merge preparation (30 min)
- **Phase 1**: Analysis (1-2 hours)
- **Phase 2**: Structural changes (2-4 hours)
  - Components → GameWorld → main.cpp
- **Phase 3**: System migration (4-8 hours)
  - Priority order, templates, asset loading
- **Phase 4**: Validation & testing (2-3 hours)
  - Compilation, pattern detection, visual testing
- **Phase 5**: Conflict resolution (ongoing)
  - Common conflicts + resolutions
- **Phase 6**: Documentation & cleanup (1 hour)
- Daily workflow breakdown (4-day plan)
- Troubleshooting guide
- Success metrics

**Best For**: Project management, daily planning, tracking progress

---

## 🛠️ Tool: Pattern Detection Script

### [../scripts/detect_old_patterns.sh](../scripts/detect_old_patterns.sh)
**Purpose**: Automatically scan codebase for old SFML patterns  
**Language**: Bash  
**Usage**:
```bash
# Scan entire project
./scripts/detect_old_patterns.sh

# Scan specific directory
./scripts/detect_old_patterns.sh client/engine/systems

# Save output
./scripts/detect_old_patterns.sh > patterns_report.txt
```

**What It Detects:**
- **CRITICAL**: `sf::RenderWindow`, `window_.draw()`, `window_.clear()`, `loadFromFile()`, SFML sprite/texture/font objects
- **WARNING**: Manual sprite manipulation (`setPosition`, `setScale`, etc.)
- **INFO**: SFML type usage, include statements, old backend references

**Output**: Color-coded report with file locations and line numbers

---

## 🚀 Recommended Workflow

### For a 4000+ Line Merge

#### Week 1: Preparation
**Monday** (1-2 hours):
- Read `RENDERING_ENGINE_MIGRATION.md` fully
- Review `API_TRANSLATION_TABLE.md`
- Run pattern detection on colleague's branch
- Create migration plan spreadsheet

**Tuesday-Thursday** (6-8 hours/day):
- Follow `MERGE_STRATEGY.md` phases
- Use `API_TRANSLATION_TABLE.md` for lookups
- Reference `MIGRATION_EXAMPLES.md` for code patterns
- Check off items in `BREAKING_CHANGES.md`
- Run `detect_old_patterns.sh` after each system

**Friday** (2-4 hours):
- Final testing and validation
- Code review preparation
- Documentation updates
- Create merge summary

---

## 📊 Document Comparison

| Resource | Best For | Read Time | During Merge |
|----------|----------|-----------|--------------|
| **RENDERING_ENGINE_MIGRATION.md** | Understanding | 30 min | Reference |
| **API_TRANSLATION_TABLE.md** | Quick lookup | 10 min | Always open |
| **BREAKING_CHANGES.md** | Planning | 15 min | Daily review |
| **MIGRATION_EXAMPLES.md** | Coding | 20 min | Copy-paste |
| **MERGE_STRATEGY.md** | Execution | 25 min | Daily guide |
| **detect_old_patterns.sh** | Validation | 2 min | After each file |

---

## 🎯 Success Criteria

Your merge is successful when:

✅ **Pattern Detection**: `./scripts/detect_old_patterns.sh` shows 0 CRITICAL issues  
✅ **Compilation**: `cmake --build build -j$(nproc)` completes without errors  
✅ **Tests**: `cd build && ctest --output-on-failure` all pass  
✅ **Visual**: `./build/client/r-type_client` displays correctly  
✅ **Code Review**: Team approves changes  
✅ **Documentation**: Comments and guides updated  

---

## 💡 Key Principles

### During Merge, Remember:

1. **Preserve Intent**: Keep colleague's game logic and features
2. **Transform Structure**: Convert SFML → RenderingEngine patterns
3. **Test Incrementally**: Compile after each file
4. **Document Decisions**: Note complex resolutions
5. **Ask Questions**: Contact colleague about unclear changes
6. **Be Systematic**: Follow priority order (P1 → P4)

### What Changes vs What Stays

**Changes (Architecture):**
- ❌ `sf::RenderWindow` → ✅ `RenderingEngine*`
- ❌ `sf::Sprite` objects → ✅ `texture_id` strings
- ❌ `loadFromFile()` → ✅ `LoadTexture()` at startup
- ❌ `window_.draw()` → ✅ `RenderSprite()`
- ❌ Direct SFML → ✅ Plugin abstraction

**Stays (Game Logic):**
- ✅ Transform hierarchy calculations
- ✅ Origin/anchor point logic
- ✅ Opacity and color handling
- ✅ Z-index sorting
- ✅ Game features and mechanics
- ✅ Component data structures (with type updates)

---

## 🆘 Getting Help

### If You're Stuck:

1. **Search Documentation**: Use Ctrl+F in markdown files
2. **Run Detection Script**: See what patterns remain
3. **Check Examples**: Find similar code in `MIGRATION_EXAMPLES.md`
4. **Consult Table**: Look up specific API in `API_TRANSLATION_TABLE.md`
5. **Review Strategy**: Re-read relevant phase in `MERGE_STRATEGY.md`
6. **Contact Team**: Describe issue with file path and line number

### Common Questions:

**Q: Where do I start?**  
A: Read `RENDERING_ENGINE_MIGRATION.md`, then follow `MERGE_STRATEGY.md` Phase 0-1.

**Q: How do I convert `window_.draw(sprite)`?**  
A: See `API_TRANSLATION_TABLE.md` "Sprite Rendering" section, or `MIGRATION_EXAMPLES.md` Example 1.

**Q: What's the priority order?**  
A: See `BREAKING_CHANGES.md` priority levels (P1 = Critical first).

**Q: How long will this take?**  
A: `MERGE_STRATEGY.md` estimates 3-5 days for 4000 lines.

**Q: Can I merge incrementally?**  
A: Yes! See `MERGE_STRATEGY.md` "Issue: Too Many Conflicts" for cherry-pick strategy.

---

## 📈 Progress Tracking

Use this checklist to track your merge:

### Pre-Merge
- [ ] Read `RENDERING_ENGINE_MIGRATION.md`
- [ ] Review `API_TRANSLATION_TABLE.md`
- [ ] Study `BREAKING_CHANGES.md`
- [ ] Create merge branch
- [ ] Run pattern detection on colleague's branch
- [ ] Create migration plan spreadsheet

### Structural Changes
- [ ] Merge component definitions
- [ ] Merge GameWorld changes
- [ ] Merge main.cpp changes
- [ ] Verify compilation

### System Migration
- [ ] Migrate DrawableSystem (P1)
- [ ] Migrate DrawTextSystem (P1)
- [ ] Migrate shader systems (P2)
- [ ] Migrate animation systems (P2)
- [ ] Migrate remaining systems (P3)

### Validation
- [ ] Run `detect_old_patterns.sh` (0 critical)
- [ ] Compile without errors
- [ ] All tests pass
- [ ] Visual testing complete
- [ ] Performance acceptable

### Finalization
- [ ] Remove dead code
- [ ] Update comments
- [ ] Create merge summary
- [ ] Code review
- [ ] Merge to main

---

## 🎉 After Successful Merge

Congratulations! You've completed a major architectural migration. Consider:

1. **Share Knowledge**: Present findings to team
2. **Update Wiki**: Document lessons learned
3. **Improve Process**: Suggest improvements to these docs
4. **Celebrate**: Major achievement! 🍾

---

## 📝 Feedback

These resources are designed to help with real-world merges. If you:
- Find missing information
- Discover better strategies
- Need additional examples
- Have suggestions for improvement

Please update the documentation or share feedback with the team.

---

## 📚 Additional Resources

- **Plugin System Guide**: `docs/docs/plugins/video-plugin-guide.md`
- **Plugin Index**: `docs/docs/plugins/index.md`
- **RenderingEngine API**: `engine/include/rendering/RenderingEngine.hpp`
- **RenderingEngine Implementation**: `client/engine/rendering/RenderingEngine.cpp`
- **Old Engine Reference**: `old_engine/` (for comparison)

---

**Good luck with your merge!** 🚀

If you follow these resources systematically, your 4000+ line merge will be manageable, traceable, and successful.
