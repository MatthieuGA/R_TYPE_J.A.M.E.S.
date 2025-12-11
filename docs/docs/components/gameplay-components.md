# Gameplay Components

**Source file:** `client/include/components/GameplayComponents.hpp`

**Purpose:** Describe gameplay-facing components: player state, enemy markers, projectiles, movement patterns, timed/frame-based events, health and UI bars.

## PlayerTag
- Core player stats: max speed `speed_max`, shoot cooldown `shoot_cooldown_max`, minimum charge time `charge_time_min`.
- Runtime state: `isInPlay` (spawned in arena), `shoot_cooldown`, `charge_time`, `playerNumber`.
- Consumed by input/movement/shoot systems to gate control and calculate speeds.

## AnimationEnterPlayer
- `isEntering` flag used by `AnimationEnterPlayerSystem` to slide the player into the playfield before enabling controls.

## EnemyTag
- Basic enemy metadata (current `speed`) so AI or movement systems can scale behaviors.

## TimedEvents
- Holds a list of `CooldownAction` (callback `action(entity_id)`, `cooldown_max`, current `cooldown`).
- `AddCooldownAction` registers periodic tasks; `TimedEventSystem` increments timers and fires callbacks when thresholds are met.

## FrameEvents
- List of `FrameEvent` (target animation name, trigger frame, callback, `triggered` latch).
- `AddFrameEvent` configures per-frame hooks; `FrameBaseEventSystem` triggers callbacks when the animation reaches the specified frame and resets on loop.

## EnemyShootTag
- Tuning for enemy shots: `speed_projectile`, `damage_projectile`, and local `offset_shoot_position` to position the muzzle.

## Projectile
- Damage payload `damage`, travel `direction`, `speed`, owner identifier `ownerId`, and `isEnemyProjectile` to filter friendly fire.
- Used by collision and lifetime systems to apply damage and clean up.

## Health
- Hit points `currentHealth` / `maxHealth` plus invulnerability controls (`invincible`, `invincibilityDuration`, `invincibilityTimer`).
- Health deduction and death handling systems rely on these fields to trigger hit and death animations.

## StatsGame
- Score tracking for an entity (`score`), typically updated on kills or pickups.

## ParrallaxLayer
- Parallax scroll speed `scroll_speed` consumed by `ParallaxSystem` to move background layers and loop them smoothly.

## AnimationDeath
- Marker `is_dead` indicating a death animation is playing; `DeathAnimationSystem` removes the entity when the animation finishes.

## PatternMovement
- Encodes movement patterns via `PatternType` (Straight, Sine*, ZigZag*, Wave, Waypoints, FollowPlayer, Circular).
- Parameters: `elapsed`, `spawnPos`, `baseDir`, `baseSpeed`, `amplitude`, `frequency`, `waypoints`, `currentWaypoint`, `waypointThreshold`, `angle`, `radius`, `targetEntityId`.
- `PaternMovementSystem` interprets these to set velocities or absolute positions each frame.

## HealthBar
- UI data for health display: `offset`, instantaneous `percent`, delayed `percent_delay`, damage timers and flags.
- Holds sprites/textures (green, yellow, foreground) with `is_loaded` to render the multi-layer health bar.
