# Scenes Components

**Source file:** `client/include/components/ScenesComponents.hpp`

**Purpose:** Track the active scene, the pending scene transition, and the registered scenes on the client.

## SceneManagement
- `current`: name of the currently active scene.
- `next`: name of the scene to transition to; consumed by `GameStateSystem`.
- `scenes`: map of scene names to `std::shared_ptr<Scene_A>` instances used to call `InitScene` / `DestroyScene` hooks.
