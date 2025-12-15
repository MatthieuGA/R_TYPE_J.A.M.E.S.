# Charging Show Asset System

## Description

The `ChargingShowAssetPlayerSystem` controls the visual charge asset for players. It dynamically adjusts the opacity of child entities (typically the charge effect) based on the player's accumulated charge time.

## Source Files

- **Path**: `client/engine/systems/SystemsFunctions/ChargingShowAssetPlayerSystem.cpp`
- **Header**: `client/engine/systems/InitRegistrySystems.hpp`

## Components Used

| Component | Access | Description |
|-----------|--------|-------------|
| `PlayerTag` | Read/Write | Holds the player's current charge time |
| `Drawable` | Write | Opacity of the charge asset is adjusted |
| `AnimatedSprite` | Write | Frame reset when charge starts |
| `Transform` | Read | Used to access the player's child list |

## Behavior

### Main Logic

1. **Iterate players**: for each entity with `PlayerTag` and `Transform`:
   - Check if it has children in `transform.children`
   - Grab the first child (charge asset)

2. **Opacity update**:
   - If `player_tag.charge_time > player_tag.charge_time_min` → set child opacity to **1.0** (visible)
   - Otherwise → set child opacity to **0.0** (hidden)

3. **Animation reset**:
   - When opacity goes from 0 to visible, reset the child's animation frame to 0

### Helper

#### `SetOpacityChildren`

```cpp
void SetOpacityChildren(
    Eng::sparse_array<Com::Drawable> &drawables,
    Eng::sparse_array<Com::AnimatedSprite> &animated_sprites,
    Com::PlayerTag &player_tag,
    Eng::sparse_array<Com::Transform> &transforms,
    size_t child_id
)
```

**Role:** Configure opacity and animation state of a specific child entity.

**Parameters:**

- `drawables`: sparse array of Drawable components
- `animated_sprites`: sparse array of AnimatedSprite components
- `player_tag`: parent player tag containing charge time
- `transforms`: sparse array of Transform components (not used currently)
- `child_id`: child entity ID to modify

**Logic:**

1. If current opacity is 0, reset `current_frame` to 0.
2. Update opacity based on `charge_time > charge_time_min`.

## Parent-child architecture

The system leverages entity hierarchy:

- **Parent entity:** Player (`PlayerTag`, `Transform`)
- **Child entity:** Charge asset (`Drawable`, `AnimatedSprite`, `Transform`)

`Transform.children` stores child IDs for direct access without scanning all transforms.

## Example usage

### Initial setup

When creating the player in `initGameLevel.cpp`:

```cpp
// Create the player
auto player_entity = reg.SpawnEntity();
reg.AddComponent<Component::Transform>(player_entity, {...});
reg.AddComponent<Component::PlayerTag>(player_entity, {...});

// Create the charge asset (child)
auto player_charging_entity = reg.SpawnEntity();
reg.AddComponent<Component::Transform>(player_charging_entity,
    Component::Transform(..., player_entity.GetId())); // Parent ID
reg.AddComponent<Component::Drawable>(player_charging_entity, {...});
reg.AddComponent<Component::AnimatedSprite>(player_charging_entity, {...});

// Add the child to the parent's list
reg.GetComponent<Component::Transform>(player_entity)
    .children.push_back(player_charging_entity.GetId());
```

### In-game flow

1. Player holds the shoot key (`Space`).
2. `ShootPlayerSystem` increments `player_tag.charge_time`.
3. When `charge_time` exceeds `charge_time_min` (0.5s default):
   - `ChargingShowAssetPlayerSystem` makes the asset visible (`opacity = 1.0`).
   - The charge animation becomes visible.
4. When the player releases shoot:
   - `charge_time` resets to 0.
   - The asset hides again (`opacity = 0.0`).

## Performance

**Complexity:** O(n × m) where:

- n = number of entities with `PlayerTag` and `Transform`
- m = average number of children per player (usually 1)

**Optimization:** Uses `transform.children` for direct access instead of scanning all transforms.

## Relations to other systems

| System | Relation |
|--------|----------|
| `ShootPlayerSystem` | Updates `charge_time` read by this system |
| `AnimationSystem` | Animates the charge asset when visible |
| `DrawableSystem` | Renders the asset with the opacity set here |

## Technical notes

- Assumes the first child (`children[0]`) is the charge asset.
- If an entity has no children, it is skipped without error.
- Opacity is binary (0.0 or 1.0) but could be made gradual.
- Child `Transform` is currently unused in `SetOpacityChildren`.

## Possible improvements

1. **Progressive opacity:** interpolate opacity based on charge time.
2. **Multi-child support:** handle multiple charge assets with different thresholds.
3. **Visual effects:** add particles or glow during charging.
4. **Audio:** add sound feedback when charging activates.
