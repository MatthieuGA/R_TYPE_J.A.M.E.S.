# Playfield Limit System

**Source:** `client/engine/systems/SystemsFunctions/PlayfieldLimitSystem.cpp`

**Purpose:** Prevent entities (typically the player) from leaving the game window.

**Components used:**

- `Transform`
- `PlayerTag`

## Behavior

- Clamp `x` and `y` between `0` and the window size (`window.getSize()`).
- Directly adjust `Transform` to correct out-of-bounds positions.

## Main signature

```cpp
void PlayfieldLimitSystem(Eng::registry &reg, const sf::RenderWindow &window,
    Eng::sparse_array<Com::Transform> &transforms,
    Eng::sparse_array<Com::PlayerTag> const &player_tags);
```

## Notes

- Uses the provided SFML window size.
- Simple bounds logic; can be extended (margins, wrap, etc.).
