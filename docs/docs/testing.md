# Testing Strategy and Best Practices

## Overview

This document outlines the testing approach for the R-Type J.A.M.E.S. project, covering unit tests, integration tests, and system-level validation. All tests are written using GoogleTest (v1.14.0) and follow the project's C++20 standards.

## Testing Pyramid

```
        ┌─────────────────────────────┐
        │   Integration & E2E Tests   │  10%
        ├─────────────────────────────┤
        │    System-Level Tests       │  30%
        ├─────────────────────────────┤
        │       Unit Tests            │  60%
        └─────────────────────────────┘
```

**Principle:** Most tests are unit tests (fast, isolated). Fewer integration tests (slower, more fragile). Minimal E2E tests (slow, high-level).

## Unit Testing

### Structure

Tests are located in the `tests/` directory, mirroring the source structure:

```
tests/
├── test_registry.cpp          // Tests for engine/src/registry.cpp
├── test_drawable_system.cpp   // Tests for DrawableSystem
├── test_animation_system.cpp  // Tests for AnimationSystem
├── test_particle_emitter.cpp  // Tests for ParticleEmitterSystem
├── components/
│   ├── test_transform.cpp
│   ├── test_drawable.cpp
│   └── test_animation_frame.cpp
└── systems/
    ├── test_render_systems.cpp
    └── test_game_logic_systems.cpp
```

### Naming Convention

- **Test File:** `test_<component_or_system>.cpp`
- **Test Suite:** `TEST(ComponentNameTest, TestName)` or `TEST(SystemNameTest, TestName)`
- **Test Case:** Descriptive action: `TEST(DrawableSystemTest, RendersVisibleEntities)`

### Example: Unit Test for DrawableSystem

```cpp
#include <gtest/gtest.h>
#include "engine/include/registry.hpp"
#include "engine/include/components.hpp"
#include "graphics/mock_render_context.hpp"  // Mock backend

class DrawableSystemTest : public ::testing::Test {
 protected:
  Registry registry;
  MockRenderContext mock_render;
  
  void SetUp() override {
    // Initialize registry with render-agnostic systems
  }
  
  void TearDown() override {
    registry.Clear();
  }
};

TEST_F(DrawableSystemTest, RendersVisibleEntities) {
  // Arrange
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Transform>(entity, {{100, 200}, 0, {1, 1}});
  registry.AddComponent<Drawable>(entity, {"sprite", {0, 0}, 0, 0, {1, 1}, true, ""});
  
  // Act
  DrawableSystem(registry, mock_render, 0.016f);
  
  // Assert
  EXPECT_EQ(mock_render.draw_calls.size(), 1);
  EXPECT_EQ(mock_render.draw_calls[0].texture_key, "sprite");
  EXPECT_EQ(mock_render.draw_calls[0].position.x, 100);
}

TEST_F(DrawableSystemTest, SkipsInvisibleEntities) {
  // Arrange
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Transform>(entity, {{100, 200}, 0, {1, 1}});
  registry.AddComponent<Drawable>(entity, {"sprite", {0, 0}, 0, 0, {1, 1}, false, ""});  // visible = false
  
  // Act
  DrawableSystem(registry, mock_render, 0.016f);
  
  // Assert
  EXPECT_EQ(mock_render.draw_calls.size(), 0);
}

TEST_F(DrawableSystemTest, AppliesShadersWhenRequested) {
  // Arrange
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Transform>(entity, {{100, 200}, 0, {1, 1}});
  registry.AddComponent<Drawable>(entity, {"sprite", {0, 0}, 0, 0, {1, 1}, true, "charged_glow"});
  
  // Act
  DrawableSystem(registry, mock_render, 0.016f);
  
  // Assert
  EXPECT_EQ(mock_render.shader_calls.size(), 1);
  EXPECT_EQ(mock_render.shader_calls[0].shader_key, "charged_glow");
}
```

## Rendering Layer Testing

### Mock Backend (IRenderContext)

Instead of initializing SFML (which requires a window), tests use a mock implementation:

```cpp
// graphics/mock_render_context.hpp
class MockRenderContext : public IRenderContext {
 public:
  struct DrawCall {
    std::string texture_key;
    sf::Vector2f position;
    sf::Vector2f origin;
    sf::IntRect source_rect;
  };
  
  std::vector<DrawCall> draw_calls;
  std::vector<std::string> textures_queried;
  
  void DrawSprite(const DrawSpriteParams& params) override {
    draw_calls.push_back({params.texture_key, params.position, params.origin, params.source_rect});
  }
  
  sf::Vector2f GetTextureSize(const std::string& texture_key) override {
    textures_queried.push_back(texture_key);
    return {64, 64};  // Return a fixed size for all textures
  }
  
  sf::FloatRect GetTextBounds(const std::string& font_key,
                               const std::string& text,
                               unsigned int char_size) override {
    return {0, 0, static_cast<float>(text.length() * char_size / 2), static_cast<float>(char_size)};
  }
  
  sf::Vector2i GetGridFrameSize(const std::string& texture_key,
                                 int grid_cols,
                                 int frame_width) override {
    return {frame_width, frame_width};
  }
  
  void Display() override {}
  void Clear(const sf::Color& color) override {}
  
  // ... other method stubs
};
```

### Testing Animation Systems

```cpp
class AnimationSystemTest : public ::testing::Test {
 protected:
  Registry registry;
  MockRenderContext mock_render;
};

TEST_F(AnimationSystemTest, AdvancesFrameOnTick) {
  // Arrange
  auto entity = registry.SpawnEntity();
  registry.AddComponent<AnimationFrame>(entity, {
    .current_frame_ = 0,
    .elapsed_time_ = 0,
    .frame_duration_ = 0.1f,
    .animation_mode_ = "strip",
    .frame_count_ = 4,
    .frame_width_ = 64,
    .grid_cols_ = 1
  });
  
  // Act
  AnimationSystem(registry, 0.05f);  // Advance 0.05s (not enough to next frame)
  auto& anim1 = registry.GetComponent<AnimationFrame>(entity);
  EXPECT_EQ(anim1.current_frame_, 0);
  EXPECT_FLOAT_EQ(anim1.elapsed_time_, 0.05f);
  
  AnimationSystem(registry, 0.06f);  // Advance 0.06s (total 0.11s, should advance frame)
  auto& anim2 = registry.GetComponent<AnimationFrame>(entity);
  EXPECT_EQ(anim2.current_frame_, 1);
}

TEST_F(AnimationSystemTest, LoopsAtEndOfAnimation) {
  // Arrange
  auto entity = registry.SpawnEntity();
  registry.AddComponent<AnimationFrame>(entity, {
    .current_frame_ = 3,  // Last frame
    .elapsed_time_ = 0.09f,
    .frame_duration_ = 0.1f,
    .animation_mode_ = "strip",
    .frame_count_ = 4,
    .frame_width_ = 64,
    .grid_cols_ = 1
  });
  
  // Act
  AnimationSystem(registry, 0.02f);  // Advance past duration
  
  // Assert
  auto& anim = registry.GetComponent<AnimationFrame>(entity);
  EXPECT_EQ(anim.current_frame_, 0);  // Loops back to 0
}
```

### Testing with MockRenderContext

Benefits:
1. **No SFML dependency:** Tests run without initializing a window.
2. **Deterministic:** No graphics state or timing issues.
3. **Fast:** In-memory operations; no disk I/O.
4. **Introspectable:** Can assert on draw parameters, texture lookups, etc.

### Limitations

- MockRenderContext does not validate parameter ranges (e.g., source_rect bounds).
- Behavior differences vs. real SFML backend may slip through.
- **Mitigation:** Use integration tests with real SFMLRenderContext for critical paths.

## Integration Testing

### Full Rendering Pipeline

Integration tests exercise the complete rendering path:

```cpp
#include <gtest/gtest.h>
#include <SFML/Graphics.hpp>
#include "client/graphics/SFMLRenderContext.hpp"

class RenderingIntegrationTest : public ::testing::Test {
 protected:
  sf::RenderWindow window{sf::VideoMode(1920, 1080), "Test Window"};
  SFMLRenderContext render_context{window};
  Registry registry;
  
  void SetUp() override {
    render_context.LoadTexture("test_sprite", "assets/test_sprite.png");
    render_context.LoadFont("default", "assets/fonts/arial.ttf");
  }
};

TEST_F(RenderingIntegrationTest, LoadsTextureAndQueries SizeCorrectly) {
  // Act
  auto size = render_context.GetTextureSize("test_sprite");
  
  // Assert
  EXPECT_GT(size.x, 0);
  EXPECT_GT(size.y, 0);
}

TEST_F(RenderingIntegrationTest, RendersWithoutCrashing) {
  // Arrange
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Transform>(entity, {{960, 540}, 0, {1, 1}});
  registry.AddComponent<Drawable>(entity, {"test_sprite", {32, 32}, 0, 0, {1, 1}, true, ""});
  
  // Act
  render_context.Clear(sf::Color::Black);
  DrawableSystem(registry, render_context, 0.016f);
  render_context.Display();
  
  // Assert (no exception thrown)
  SUCCEED();
}
```

### System Integration Tests

```cpp
class FullRenderingStackTest : public ::testing::Test {
 protected:
  Registry registry;
  MockRenderContext mock_render;
  
  void InitializeDefaultResources() {
    // Mock would return fixed sizes; real backend would load
  }
};

TEST_F(FullRenderingStackTest, AnimationAndRenderingIntegration) {
  // Arrange: Spawn an animated entity
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Transform>(entity, {{100, 200}, 0, {1, 1}});
  registry.AddComponent<Drawable>(entity, {"sprite", {0, 0}, 0, 0, {1, 1}, true, ""});
  registry.AddComponent<AnimationFrame>(entity, {
    .current_frame_ = 0,
    .elapsed_time_ = 0,
    .frame_duration_ = 0.1f,
    .animation_mode_ = "strip",
    .frame_count_ = 4,
    .frame_width_ = 64,
    .grid_cols_ = 1
  });
  
  InitializeAnimationSystem(registry, mock_render);
  
  // Act: Run one frame
  AnimationSystem(registry, 0.05f);
  DrawableSystem(registry, mock_render, 0.05f);
  
  // Assert: Drawable should have been rendered
  EXPECT_EQ(mock_render.draw_calls.size(), 1);
  // Drawable should still be on frame 0 (0.05s < 0.1s duration)
  EXPECT_EQ(registry.GetComponent<AnimationFrame>(entity).current_frame_, 0);
}
```

## Build and Run Tests

### Compile

```bash
cmake -S . -B build
cmake --build build -j$(nproc)
```

### Run All Tests

```bash
cd build
ctest --output-on-failure
```

### Run Specific Test

```bash
cd build
ctest --output-on-failure -R "DrawableSystemTest"
```

### Run with Verbose Output

```bash
cd build
ctest --output-on-failure -V
```

## Test Coverage

### Code Coverage (Linux/GCC/Clang)

Requires `gcovr`:

```bash
sudo apt-get install gcovr  # Linux

cmake -S . -B build -DENABLE_COVERAGE=ON
cmake --build build -j$(nproc)
cd build && make run-coverage
# Report: build/coverage.html
```

### Coverage Goals

- **Engine Core (ECS, Components, Systems):** ≥ 80%
- **Graphics Abstraction (IRenderContext, MockRenderContext):** ≥ 85%
- **Rendering Systems (Drawable, Animation, Text, Particles):** ≥ 75%
- **Overall:** ≥ 70%

## Best Practices

### 1. Use Arrange-Act-Assert (AAA)

```cpp
TEST(ExampleTest, DescribesWhatItDoes) {
  // Arrange: Set up preconditions
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Drawable>(entity, {...});
  
  // Act: Perform the action
  DrawableSystem(registry, mock_render, 0.016f);
  
  // Assert: Verify the result
  EXPECT_EQ(mock_render.draw_calls.size(), 1);
}
```

### 2. Test One Behavior Per Test

Avoid testing multiple behaviors in one test; this makes failures ambiguous.

```cpp
// Good
TEST(AnimationSystemTest, AdvancesFrame) { ... }
TEST(AnimationSystemTest, LoopsAtEnd) { ... }

// Bad
TEST(AnimationSystemTest, HandlesAnimation) {
  // Tests both advancement and looping; unclear which fails
}
```

### 3. Use Fixtures for Common Setup

```cpp
class MySystemTest : public ::testing::Test {
 protected:
  Registry registry;
  MockRenderContext mock_render;
  
  void SetUp() override { /* called before each test */ }
  void TearDown() override { /* called after each test */ }
};
```

### 4. Mock External Dependencies

Use `MockRenderContext` to isolate system logic from graphics details.

```cpp
// Good: Isolated, fast, deterministic
TEST(DrawableSystemTest, RendersEntities) {
  DrawableSystem(registry, mock_render, 0.016f);
  EXPECT_EQ(mock_render.draw_calls.size(), 1);
}

// Bad: Depends on SFML window, slow, fragile
TEST(DrawableSystemTest, RendersEntities) {
  sf::RenderWindow window(...);  // Heavy setup
  SFMLRenderContext render_context(window);
  DrawableSystem(registry, render_context, 0.016f);
  // Hard to assert on actual pixels
}
```

### 5. Test Edge Cases

```cpp
TEST(DrawableSystemTest, HandlesZeroScale) {
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Drawable>(entity, {..., {0, 0}, ...});  // Zero scale
  
  DrawableSystem(registry, mock_render, 0.016f);
  
  EXPECT_EQ(mock_render.draw_calls.size(), 1);  // Should still render
}

TEST(DrawableSystemTest, HandlesNegativeRotation) {
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Drawable>(entity, {..., -45, ...});  // -45 degrees
  
  DrawableSystem(registry, mock_render, 0.016f);
  
  EXPECT_EQ(mock_render.draw_calls.size(), 1);
}
```

### 6. Test Error Conditions

```cpp
TEST(DrawableSystemTest, LogsWarningForMissingTexture) {
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Drawable>(entity, {"nonexistent_texture", {...}});
  
  DrawableSystem(registry, mock_render, 0.016f);
  
  EXPECT_GT(mock_render.warnings.size(), 0);  // Should warn
}
```

## Continuous Integration (CI)

CI automatically runs tests on every commit:

```yaml
# .github/workflows/test.yml (example)
name: Tests
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v3
      - name: Build
        run: cmake -S . -B build && cmake --build build -j4
      - name: Test
        run: cd build && ctest --output-on-failure
```

## Debugging Tests

### Run a Single Test with Breakpoint

```bash
cd build
gdb ./tests/engine_tests
(gdb) run --gtest_filter="DrawableSystemTest.RendersVisibleEntities"
(gdb) # Use breakpoint commands as usual
```

### Print Debug Info

```cpp
TEST(ExampleTest, WithDebug) {
  auto entity = registry.SpawnEntity();
  registry.AddComponent<Drawable>(entity, {...});
  
  std::cout << "Entity: " << entity << std::endl;
  
  DrawableSystem(registry, mock_render, 0.016f);
  
  std::cout << "Draw calls: " << mock_render.draw_calls.size() << std::endl;
}
```

### Check Mock State

```cpp
TEST(RenderingTest, CheckMockState) {
  MockRenderContext mock;
  
  // ... run system ...
  
  EXPECT_EQ(mock.draw_calls.size(), 1);
  EXPECT_EQ(mock.textures_queried.size(), 1);
  EXPECT_EQ(mock.textures_queried[0], "sprite");
}
```

## References

- [GoogleTest Documentation](https://google.github.io/googletest/)
- [Testing Best Practices](https://google.github.io/googletest/primer.html)
- [Rendering Systems](./rendering/systems.md)
- [Architecture](./architecture.md)
