# Game State System

**Source:** `client/engine/systems/SystemsFunctions/GameStateSystem.cpp`

**Purpose:** Handle scene transitions by calling `DestroyScene` and `InitScene` hooks when the next state changes.

**Components used:**

- `SceneManagement` (current, next, scenes: `std::unordered_map<std::string, std::shared_ptr<Scene_A>>`)

## Behavior

- Skip if `next` is empty or identical to `current`.
- If the current scene exists in `scenes`, call `DestroyScene(reg)` to clean up the outgoing state.
- If the target scene exists in `scenes`, call `InitScene(reg)` to initialize the new state.
- Update `current` with `next`, then reset `next` to an empty string to avoid unintended reloads.

## Main signature

```cpp
void GameStateSystem(Eng::registry &reg,
    Eng::sparse_array<Component::SceneManagement> &sceneManagements);
```

## Notes

- `Scene_A` defines the `InitScene` and `DestroyScene` hooks invoked by this system.
- The `scenes` map must be prefilled with available scenes and their keys.
- `next` must be set by other logic (UI, network, scripts) before this system runs so the transition occurs during the pass.
