# R-Type Plugin Testing Guide

This directory contains comprehensive tests for the graphics backend plugin system (Phase E).

## Test Structure

```
tests/
├── unit/
│   └── test_dl_loader.cpp          # Plugin loader unit tests
├── integration/
│   ├── HeadlessTestRenderer.hpp    # Test helper for headless rendering
│   ├── smoke_plugin_load.cpp       # Plugin load and basic rendering tests
│   └── pixel_compare_test.cpp      # Pixel-perfect regression tests
├── baseline/
│   └── smoke_baseline.png          # Known-good rendering baseline
└── artifacts/
    └── *.png                        # Generated test outputs (gitignored)
```

## Test Suites

### 1. Plugin Loader Unit Tests

**Location:** `tests/unit/test_dl_loader.cpp`  
**Executable:** `build/tests/plugin_loader_tests`

Tests the core plugin loading functionality:
- **OpenMissingLibrary**: Verifies graceful failure when loading non-existent plugins
- **MissingSymbols**: Verifies detection of plugins missing required ABI symbols
- **OpenSuccess**: Verifies successful loading of valid plugins
- **MultipleLoadsSamePath**: Verifies same plugin can be loaded with different names
- **EmptyPath/EmptyName**: Verifies validation of input parameters

**Run locally:**
```bash
cd /path/to/R_TYPE_J.A.M.E.S.
cmake --build build
./build/tests/plugin_loader_tests
```

**Expected output:** 6/6 tests passing

### 2. Integration Smoke Tests

**Location:** `tests/integration/smoke_plugin_load.cpp`  
**Executable:** `build/tests/integration_smoke_test`

End-to-end tests that verify:
- Plugins load successfully in realistic scenarios
- Loaded backends can render without crashing
- Multiple plugins can coexist

**Run locally:**
```bash
./build/tests/integration_smoke_test
```

**Expected output:** 2-3 tests passing (1 may skip if windowing not available)

### 3. Pixel Comparison Tests

**Location:** `tests/integration/pixel_compare_test.cpp`  
**Executable:** `build/tests/pixel_compare_test`

Regression tests that catch rendering changes:
- Compares rendered output to known-good baseline image
- Tolerance: max 4 per-channel difference, max 0.2% differing pixels
- Generates diff image for debugging on failure

**Run locally:**
```bash
./build/tests/pixel_compare_test
```

**First run:** Generates baseline image at `tests/baseline/smoke_baseline.png`  
**Subsequent runs:** Compares against baseline

**Artifacts generated:**
- `tests/artifacts/smoke_output.png` - Current rendered output
- `tests/artifacts/diff.png` - Visualization of pixel differences (red = diff)

## Running All Plugin Tests

### Quick Run (All Tests)
```bash
cd /path/to/R_TYPE_J.A.M.E.S.
cmake --build build
ctest --test-dir build --output-on-failure -R "plugin|smoke|pixel"
```

### Individual Test Suites
```bash
# Unit tests only
./build/tests/plugin_loader_tests

# Integration tests only
./build/tests/integration_smoke_test

# Pixel comparison only
./build/tests/pixel_compare_test
```

### With Verbose Output
```bash
./build/tests/plugin_loader_tests --gtest_filter="*" --gtest_verbose=1
```

## Headless Testing (CI / No Display)

Tests are designed to run headlessly using Xvfb (X Virtual Frame Buffer):

```bash
# Install Xvfb (if not already installed)
sudo apt-get install xvfb

# Run with Xvfb
xvfb-run --auto-servernum --server-args='-screen 0 1024x768x24' \
  ./build/tests/pixel_compare_test
```

## Dependencies

### Required
- **C++20 compiler** (GCC 12+, Clang 15+, MSVC 2022+)
- **CMake 3.23+**
- **vcpkg** (for SFML and other deps)
- **GoogleTest** (fetched automatically by CMake)
- **SFML 2.6+** (via vcpkg)

### Optional (for headless CI)
- **Xvfb** - X virtual framebuffer (Linux only)
- **ImageMagick** - For advanced pixel comparison (currently using built-in)

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    git \
    pkg-config \
    xvfb \
    libgl1-mesa-dev \
    libx11-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libudev-dev \
    libopenal-dev \
    libflac-dev \
    libvorbis-dev
```

**macOS:**
```bash
brew install cmake pkg-config
```

**Windows:**
- Install Visual Studio 2022 with C++ tools
- Install CMake (https://cmake.org/download/)
- vcpkg will be bootstrapped automatically

## CI Integration

Plugin tests run automatically in GitHub Actions on every push/PR.

**Workflow:** `.github/workflows/plugins-ci.yml`

**What CI does:**
1. Builds project with plugins enabled
2. Verifies plugin artifacts (`libgraphics_sfml.so`) exist
3. Runs all three test suites:
   - Plugin loader unit tests
   - Integration smoke tests
   - Pixel comparison tests
4. Uploads test artifacts on failure for debugging
5. Uploads baseline image for reference

**View CI results:**
- Check the "Actions" tab in GitHub
- Look for "Plugin Tests CI" workflow
- Download artifacts from failed builds for inspection

## Troubleshooting

### Test fails with "Plugin not built"
**Cause:** Plugin shared library not built  
**Fix:** Ensure CMake configuration includes plugins:
```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build
```

### Pixel comparison test fails
**Cause:** Rendering output differs from baseline  
**Fix:** 
1. Check `tests/artifacts/diff.png` to see what changed
2. If change is intentional, update baseline:
   ```bash
   rm tests/baseline/smoke_baseline.png
   ./build/tests/pixel_compare_test  # Regenerates baseline
   git add tests/baseline/smoke_baseline.png
   git commit -m "Update rendering baseline"
   ```

### "X11 connection" or "Cannot open display" errors
**Cause:** No display available (headless environment)  
**Fix:** Use Xvfb:
```bash
xvfb-run --auto-servernum ./build/tests/pixel_compare_test
```

### Tests hang or crash
**Cause:** Graphics driver issues or threading problems  
**Fix:** 
- Update graphics drivers
- Run with software rendering: `LIBGL_ALWAYS_SOFTWARE=1 ./build/tests/...`
- Check logs for specific error messages

## Test Coverage

Current test coverage for plugin system:

- ✅ Plugin loading (success/failure paths)
- ✅ Missing symbol detection
- ✅ Missing file detection
- ✅ Multiple plugin loads
- ✅ Backend registration
- ✅ Headless rendering
- ✅ Pixel-perfect output verification
- ✅ Plugin vs static backend equivalence

## Adding New Tests

### Unit Test Example
```cpp
// tests/unit/test_dl_loader.cpp
TEST_F(GraphicsPluginLoaderTest, MyNewTest) {
    bool result = GraphicsPluginLoader::LoadPlugin("path", "name");
    EXPECT_TRUE(result);
}
```

### Integration Test Example
```cpp
// tests/integration/smoke_plugin_load.cpp
TEST_F(PluginSmokeTest, MyNewSmokeTest) {
    HeadlessTestRenderer renderer(320, 200);
    renderer.RenderTestScene();
    EXPECT_TRUE(renderer.SaveToPNG("tests/artifacts/my_test.png"));
}
```

### Pixel Test Example
```cpp
// tests/integration/pixel_compare_test.cpp
TEST_F(PixelCompareTest, MyNewPixelTest) {
    sf::Image baseline, actual;
    baseline.loadFromFile("tests/baseline/my_baseline.png");
    // ... render actual ...
    bool matches = CompareImages(baseline, actual, 4, 0.2f);
    EXPECT_TRUE(matches);
}
```

## Performance Notes

- **Unit tests:** < 1 second total
- **Integration tests:** ~100ms per test
- **Pixel tests:** ~150ms per test (includes rendering + comparison)
- **Total plugin test suite:** < 3 seconds

Tests are designed to be fast and deterministic for CI.

## License

See project LICENSE file.

## Contact

For issues with plugin tests, please open a GitHub issue with:
- Test output (from `--gtest_output=xml`)
- Platform/OS information
- Generated artifacts (`tests/artifacts/*.png`)
