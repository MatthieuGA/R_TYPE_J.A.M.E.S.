# Render Components

**Source file:** `client/include/components/RenderComponent.hpp`

**Purpose:** Enumerate the SFML render components: sprites, shaders, animated sprites, text, and particles.

## Drawable
- Sprite path `spritePath` (prefixed with `assets/images/`), layering `z_index`, `opacity`, `rotation`, and tint `color`.
- Owns SFML resources (`texture`, `sprite`) with `isLoaded` to avoid redundant loads.
- Non-copyable; move operations rebind the sprite to the moved texture to keep visuals intact.

## Shader
- Fragment shader description: `shaderPath` (prefixed `assets/shaders/`), shared pointer `shader`, `uniforms_float`, and `isLoaded` flag.
- `InitializeShaderSystem` binds `texture` to `sf::Shader::CurrentTexture` and uploads provided uniforms.

## AnimatedSprite
- Animation map (`animations` name → `Animation`), current animation key `currentAnimation`, optional `animationQueue` for temporary overrides.
- Playback state: `animated`, `elapsedTime`.
- `Animation` holds spritesheet path (prefixed), frame dimensions, `totalFrames`, `current_frame`, `frameDuration`, `loop`, `first_frame_position`, `offset`, plus SFML `texture` / `sprite` and `isLoaded`.
- Main API: `AddAnimation`, `SetCurrentAnimation`, `GetCurrentAnimation` (const and non-const) used by animation/render systems.

## Text
- Text content `content`, font path `fontPath` (prefixed `assets/fonts/`), `characterSize`, `color`, `opacity`, `z_index`, local `offset`.
- SFML resources: `sf::Text text`, `sf::Font font`, `is_loaded` guard. Non-copyable; move rebinds the text to the moved font to avoid dangling pointers.

## ParticleEmitter / Particle
- `Particle`: position, velocity, remaining `lifetime` and `maxLifetime` snapshot.
- `ParticleEmitter`: emission toggles (`active`, `emitting`), optional duration, container `particles` (capped by `maxParticles`), `emissionRate` and accumulator, color gradient (`startColor`, `endColor`), spawn `offset`, physical params (lifetime, speed, base `direction`, `spreadAngle`, `gravity`, `emissionRadius`), size interpolation (`start_size`, `end_size`), render `z_index`, and `vertices` (point list) for drawing.
- Intended to be consumed by a particle system to spawn, update, and render GPU-friendly points.
