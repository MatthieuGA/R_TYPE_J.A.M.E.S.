# Factory Actors

**Source files:** [FactoryActors.hpp](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/client/game/factory/factory_ennemies/FactoryActors.hpp), [FactoryActors.cpp](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/client/game/factory/factory_ennemies/FactoryActors.cpp), [FactoryActorsInit.cpp](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/client/game/factory/factory_ennemies/FactoryActorsInit.cpp), [CreateMermaidActor.cpp](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/client/game/factory/factory_ennemies/CreateMermaidActor.cpp), [CreatePlayerActor.cpp](https://github.com/MatthieuGA/R_TYPE_J.A.M.E.S./blob/main/client/game/factory/factory_ennemies/CreatePlayerActor.cpp)

**Purpose:** Centralized builder that instantiates actors (player and enemies) from JSON-driven definitions. It attaches the shared ECS components (transform, health, visuals, collision), then layers specialized logic per actor type (animations, AI patterns, shooting, VFX).

## Data flow
- **InitializeEnemyInfoMap(json_folder):** scans a folder of JSON files and fills the `enemy_info_map_` cache with `EnnemyInfo` descriptors. Each file is named after the enemy tag (stem of filename).
- **CreateActor(entity, reg, tag):** looks up the `EnnemyInfo` by `tag`, applies shared components via `CreateBasicActor`, then dispatches to `CreatePlayerActor` for `tag == "player"` or to `CreateBasicEnnemy` plus any specialized constructors (e.g., mermaid).
- **Runtime spawning:** systems or scene scripts spawn an entity, then call `CreateActor` with the desired tag; all components are attached in one place.

## EnnemyInfo schema (JSON)
Keys read from each JSON file in the provided folder (defaults are applied if missing):

| Field | Type | Purpose |
|-------|------|---------|
| `tag` | string | Logical identifier ("player", "mermaid", ...). |
| `health` | int | Starting HP. |
| `speed` | float | Baseline speed used by movement/shoot systems. |
| `spritePath` | string | Sprite sheet used by the base `Drawable`. |
| `hitbox` | object `{x, y}` | Hitbox dimensions. |
| `offset_healthbar` | object `{x, y}` | Offset for positioning the health bar. |
| `transform.scale` | object `{x, y}` | Scale applied to the base `Transform`. |

## Shared setup (CreateBasicActor)
Attaches common components for any actor:
- `Transform`: positioned at origin with provided scale, origin set to `CENTER`.
- `Health` and `HealthBar`: HP pool and on-screen bar offset.
- `HitBox`: AABB sized from config.
- `Velocity`: zeroed, ready for movement systems.
- `Drawable`: sprite path + z-layer (actors). **Note:** currently added twice; if unintended, consider removing the duplicate.

## Player path (CreatePlayerActor)
Adds player-specific components:
- `AnimatedSprite`: idle sheet plus dedicated `Hit` and `Death` animations (custom sizes/durations).
- `Inputs`: mutable input state populated by `InputSystem`.
- `PlayerTag`: speed, shoot cooldown, charge time, and charge state from config constants.
- `AnimationEnterPlayer`: flag enabling the entry animation effect.
- `ParticleEmitter`: rear thruster VFX (yellow-to-red gradient, continuous emission, actor-layered).

## Enemy path (CreateBasicEnnemy)
Adds enemy-specific components:
- `EnemyTag`: speed baseline for AI systems.
- `AnimatedSprite`: default sheet with `Hit`, `Death`, and `Attack` animations; defaults to `currentAnimation = "Default"`.

## Mermaid specialization (CreateMermaidActor)
- `PatternMovement`: sine horizontal pattern with configured offsets and `BASIC_SPEED`.
- `EnemyShootTag`: projectile speed/damage and spawn offset.
- `FrameEvents`: triggers on frame 5 of the `Attack` animation to spawn a projectile via `CreateEnemyProjectile` (safe-guarded by `try/catch`).
- `TimedEvents`: periodic callback (every `BASIC_SHOOT_COOLDOWN`) that switches the animation to `Attack` if the enemy is still alive.
- Adds `EnemyShootTag` to the entity after configuring callbacks.

## Projectile creation helper (CreateEnemyProjectile)
Builds enemy projectiles with consistent components:
- `Transform`: positioned using the shoot offset scaled by the shooter’s transform; centered origin.
- `Drawable`: projectile sprite on projectile layer.
- `Projectile`: damage, direction, speed, owner ID, and an enemy flag.
- `HitBox`: small 8x8 AABB.
- `Velocity`: initialized from direction.
- `ParticleEmitter`: short red trail for visual feedback.

## JSON loading behavior
Helper getters enforce types and apply defaults. Missing or malformed keys fall back to sane defaults; `transform.scale` defaults to `(1,1)` when absent. Files that cannot be opened are logged to `std::cerr` and skipped.

## Usage notes
- Call `InitializeEnemyInfoMap` once at startup with the folder containing enemy JSON definitions, then use `CreateActor` whenever spawning an actor.
- Extend the factory by adding new specialized creators and branching on `info.tag` inside `CreateActor`.
- Keep `EnnemyInfo` values in sync with art assets (sprite sizes, hitboxes) to avoid visual/collision mismatch.
