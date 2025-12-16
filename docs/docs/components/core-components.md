# Core Components

**Source file:** `client/include/components/CoreComponents.hpp`

**Purpose:** Define the foundational ECS components used by most systems (spatial data, simple kinematics, input capture, click interactions, collision helpers).

## Transform
- 2D position (`x`, `y`), rotation in degrees (`rotationDegrees`), uniform or non-uniform scale (`scale`).
- Configurable origin (`origin` enum or `customOrigin`) so render/collision offsets align with the sprite or UI element.
- Hierarchy support through `parent_entity` (optional) and `children` (entity IDs) to express parent/child transforms without raw pointers.
- `GetWorldRotation()` lets render systems accumulate rotations across the hierarchy.

## Velocity
- Linear velocities `vx`, `vy` plus accelerations `accelerationX`, `accelerationY` that are integrated each frame by `MovementSystem`.
- Keeps physics lightweight and deterministic; no forces are stored here, only already-resolved accelerations.

## Controllable
- Boolean `isControllable` gate used by `ControllablePlayerSystem` to ignore player inputs when entities are disabled or during cutscenes.

## InputState / Inputs
- `InputState`: raw digital state (up, down, left, right, shoot) often used for networking or replay.
- `Inputs`: analog-style axes `horizontal`, `vertical` plus `shoot` and `last_shoot_state`; populated by `InputSystem` and consumed by movement/shoot systems.

## HitBox
- Axis-aligned box defined by `width`, `height`, optional offsets (`offsetX`, `offsetY`).
- `scaleWithTransform` controls whether the AABB scales with the owning `Transform`, allowing precise UI hit-tests or world collisions.

## Solid
- Collision resolution hints: `isSolid` enables penetration resolution; `isLocked` prevents the entity from being moved during resolution (e.g., static walls).
- Used by `CollisionDetectionSystem` to decide which body (or both) is displaced.

## Clickable
- Holds the `onClick` callback and UI state flags `isHovered` / `isClicked`.
- Visual feedback colors `idleColor`, `hoverColor`, `clickColor` are applied to the paired `Drawable` by `ButtonClickSystem`.
