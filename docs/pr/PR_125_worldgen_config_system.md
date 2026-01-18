# 🚀 Pull Request Summary

**Closes:** #125
**Milestone:** <!-- ex: Milestone 1 -->

---

## 🧩 Type of Change

Select all relevant options:

- [x] ✨ Feature
- [ ] 🐛 Bug Fix
- [x] ♻️ Refactor
- [x] 📝 Documentation
- [x] 🧪 Tests
- [x] 🔧 Tooling / CI
- [ ] 🚀 Performance
- [ ] Other (please describe)

---

## 📦 Changes Included

List the main changes made in this PR:

- Add a JSON-driven WorldGen configuration system based on WorldGen Frames (WGF).
- Implement `WorldGenTypes.hpp` with all POD data structures (WGFDefinition, ObstacleData, SpawnRules, LevelDefinition, config structs).
- Implement `WorldGenConfigLoader` (`WorldGenConfigLoader.hpp` / `WorldGenConfigLoader.cpp`) to scan `server/assets/worldgen/core` and `server/assets/worldgen/user`, parse and validate `.wgf.json` files, assign defaults, deduplicate UUIDs, and provide O(1) lookup.
- Add example WGF JSON files under `server/assets/worldgen/core` and a `config.json` with difficulty/endless-mode parameters.
- Add unit tests in `tests/test_worldgen.cpp` covering loading, validation, queries and config parsing.
- Add documentation: `docs/docs/design/WORLDGEN_CONFIG_SYSTEM.md` and implementation summary `docs/docs/design/WORLDGEN_IMPLEMENTATION.md`.
- Add `nlohmann-json` dependency to `vcpkg.json` and update CMake configuration.

---

## 🧪 How to Test

Describe how reviewers can test your changes:

1. Build the project:

```bash
cmake -S . -B build
cmake --build build -j
```

Or use the project's script:

```bash
./scripts/build.sh
```

2. Run the WorldGen unit tests:

- From project root (ctest):

```bash
cd build
ctest -R worldgen --output-on-failure
```

- Or run the binary directly (if built):

```bash
./build/tests/worldgen_tests --gtest_brief=1
```

Expected: All worldgen-related tests pass (loading, validation, queries).

3. Verify runtime loading (manual test)

- Start server locally and watch logs for WorldGen loader output:

```bash
./build/server/r-type_server [port]
```

- Expected: Logs show `WorldGen loaded: X core, Y user` and INFO lines for each WGF loaded. No crash on malformed or missing WGF files.

4. Quick validation steps

- Add a malformed JSON file to `server/assets/worldgen/user/` → restart server → confirm file is skipped and a parse error is logged.
- Add a user WGF with a UUID that duplicates a core WGF → ensure user file is skipped and duplicate UUID is logged as a warning.
- Create a small level file (editor JSON) listing WGF UUIDs and ensure `LevelDefinition` format is accepted by tooling (editor compatibility is documented).

If applicable, include:
- Example input: `server/assets/worldgen/core/asteroid_field_light.wgf.json`
- Observed output: Loader logs as described above.

---

## 🗂️ Related Areas

Which part of the project does this PR affect?

- [x] area/docs
- [x] area/server
- [ ] area/client
- [x] area/engine
- [x] area/tests

---

## 🔍 Checklist Before Requesting Review

Please confirm the following items are done:

- [x] My commits follow the **Gitmoji + English commit message** convention (used throughout branch)
- [ ] My branch name follows the required format (`feature/XX-name`) — current branch is `125-new-task-implement-worldgen-config-files` (rename optional)
- [x] I have linked the PR to an Issue (`Closes #125`)
- [x] I ran and passed **all Git hooks**
- [x] I added or updated documentation where needed (`WORLDGEN_CONFIG_SYSTEM.md`, `WORLDGEN_IMPLEMENTATION.md`)
- [x] I added or updated tests (`tests/test_worldgen.cpp`)
- [x] The code builds locally without errors
- [x] I performed manual testing of loader behavior (see "How to Test")
- [ ] CI passes all checks (build + tests + formatting) — please verify after opening the PR

---

## 📸 Screenshots / Logs (if applicable)

- Loader log example:

```
[WorldGen] Loaded core WGF: Asteroid Field - Light
[WorldGen] Loaded core WGF: Empty Space
[WorldGen] WorldGen loaded: 6 core, 0 user
```

See `docs/docs/design/WORLDGEN_IMPLEMENTATION.md` for additional log/stat expectations.

---

## 👀 Reviewer Notes

Anything you want reviewers to pay special attention to?

- The `WorldGenConfigLoader` is responsible only for loading and validation. Runtime selection (seed-based selection, `WorldGenManager`, and `SeedMetadata` usage) is intentionally deferred to a follow-up PR to keep scope clear and reviewable.
- Please review `WorldGenTypes.hpp` for POD correctness — systems are expected to consume these typed structs instead of raw JSON blobs.
- Pay attention to duplicate-UUID handling rules: core takes precedence and user WGFs with duplicate UUIDs are skipped.
- Validate the `config.json` defaults and difficulty scaling parameters for plausibility.
- Unit tests include negative tests (invalid JSON, missing fields) — ensure CI test runner picks up the new test target.

---

If you want, I can:
- Rename the branch to `feature/125-worldgen-config-system` before pushing (keeps naming convention consistent).
- Open the PR for you with this body (if you give me permission to create the PR).
- Add a minimal `WorldGenManager` stub and `SeedMetadata` in the same branch to show runtime selection in a follow-up small PR.

Which follow-up action do you want me to take?
