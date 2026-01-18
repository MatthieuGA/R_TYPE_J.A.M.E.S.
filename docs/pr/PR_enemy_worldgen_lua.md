## 🚀 Pull Request Summary

Closes: #ISSUE_NUMBER
Milestone: <!-- ex: Milestone 1 -->

This PR adds design and implementation documentation for:
- Enemy generation integrated with WorldGen Frames (WGF)
- Lua-scripted enemy patterns (bosses / special enemies)
- A summary of Issue #125 implementation and next runtime steps

These files are pure documentation (no code changes):
- `docs/docs/design/ENEMY_GENERATION.md`
- `docs/docs/design/LUA_ENEMY_SCRIPTS.md`
- `docs/docs/design/WORLDGEN_ISSUE_125_SUMMARY.md`

---

## 🧩 Type of Change

Select all relevant options:

- [x] 📝 Documentation
- [ ] ✨ Feature
- [ ] 🐛 Bug Fix
- [ ] ♻️ Refactor
- [ ] 🧪 Tests
- [ ] 🔧 Tooling / CI
- [ ] 🚀 Performance
- [ ] Other (please describe)

---

## 📦 Changes Included

List the main changes made in this PR:

- Add `ENEMY_GENERATION.md`: describes data-driven enemy spawns in WGFs, schema extensions, formations, integration flow, and tests
- Add `LUA_ENEMY_SCRIPTS.md`: describes Lua scripting architecture, Sol2 dependency, example scripts, C++ integration, and performance guidance
- Add `WORLDGEN_ISSUE_125_SUMMARY.md`: maps `issue_detail.md` to implemented files, documents scope of this PR and lists next runtime-focused PRs

---

## 🧪 How to Test

Documentation-only PR — review steps for accuracy and integration suggestions:

1. Open the three files in `docs/docs/design/` and verify content matches expectations:
   - `ENEMY_GENERATION.md`
   - `LUA_ENEMY_SCRIPTS.md`
   - `WORLDGEN_ISSUE_125_SUMMARY.md`

2. Optional: run local search to confirm references exist and paths are correct:

```bash
# from repository root
rg "ENEMY_GENERATION.md|LUA_ENEMY_SCRIPTS.md|WORLDGEN_ISSUE_125_SUMMARY.md" docs -n
```

3. If you want the codebase to exercise related parts (loader/tests):

```bash
# build worldgen tests
cmake --build build --target worldgen_tests
./build/tests/worldgen_tests --gtest_brief=1
```

Expected results:
- Documentation renders correctly in the repo viewer
- `worldgen_tests` (if built) should still pass (this PR did not change code)

---

## 🗂️ Related Areas

Which part of the project does this PR affect?

- [x] area/docs
- [ ] area/server
- [ ] area/client
- [ ] area/engine
- [ ] area/tests

---

## 🔍 Checklist Before Requesting Review

Please confirm the following items are done (or note if you want me to do them):

- [ ] My commits follow the **Gitmoji + English commit message** convention
- [ ] My branch name follows the required format (`feature/XX-name`)
- [x] I have linked the PR to an Issue (`Closes #ISSUE_NUMBER`)  <!-- update ISSUE_NUMBER -->
- [ ] I ran and passed **all Git hooks**
- [x] I added or updated documentation if needed
- [ ] I added or updated tests if needed
- [ ] The code builds without errors
- [ ] I performed manual testing
- [ ] CI passes all checks (build + tests + formatting)

---

## 📸 Screenshots / Logs (if applicable)

None — documentation only. Attach diagrams or sample outputs here if helpful.

---

## 👀 Reviewer Notes

Please pay attention to:

- Schema consistency between `ENEMY_GENERATION.md` and `WorldGenTypes.hpp` (field names, types)
- Integration approach for runtime spawning (server-managed frame timing → client factory)
- Lua API surface in `LUA_ENEMY_SCRIPTS.md` and whether the proposed C++ bindings are acceptable
- Whether the `WORLDGEN_ISSUE_125_SUMMARY.md` correctly reflects which work is in this PR vs deferred to follow-up PRs

---

## Next Steps (I can do on request)

- Commit these PR files and push to a target branch (e.g., `epic-2` or `125-new-task-implement-worldgen-config-files`)
- Open a Pull Request on GitHub and assign reviewers
- Add `vcpkg.json` changes and minimal example code if you want a runnable Lua integration prototype

If you want me to commit & push, tell me target branch name and whether to push directly to `origin/epic-2` (I will stash/restore local changes if necessary).