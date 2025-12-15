# Initialize Shader System

# Add to your ~/.bashrc or ~/.bash_profile:
export VCPKG_ROOT="/home/matt/tek3/RTYPE/R_TYPE_J.A.M.E.S./vcpkg"**Source:** `client/engine/systems/systems_functions/render/InitializeShaderSystem.cpp`

**Purpose:** Load and initialize fragment shaders for `Shader` components via the `RenderingEngine` plugin API.

**Components used:**

- `Shader` (contains `shader_path`, `shader_id`, `uniforms_float`, `is_loaded`)

## Behavior

- For each `Shader` not yet loaded and with a valid `shader_path`, load the shader via `game_world.rendering_engine_->LoadShader()`.
- The shader is identified by `shader_id` and loaded from the file path.
- On success, set `is_loaded = true`; on failure, log an error.
- Shader uniforms are applied later by `DrawableSystem` when rendering.

## Main signature

```cpp
void InitializeShaderSystem(Eng::registry &reg, GameWorld &game_world,
    Eng::sparse_array<Com::Shader> &shaders);
```

## Notes

- Shaders are loaded via the plugin abstraction (no direct SFML usage).
- Idempotent: the system will not reload a shader already marked `is_loaded`.
- Compatible with any `IVideoModule` plugin that supports shaders.
