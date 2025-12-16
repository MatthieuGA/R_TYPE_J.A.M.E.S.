# Initialize Shader System

**Source:** `client/engine/systems/SystemsFunctions/InitializeShaderSystem.cpp`

**Purpose:** Load and initialize fragment shaders for `Shader` components.

**Components used:**

- `Shader` (contains `shaderPath`, `uniforms_float`, `isLoaded`, `shader` shared_ptr)

## Behavior

- For each `Shader` not yet loaded and with a valid `shaderPath`, load an `sf::Shader` into memory.
- Initialize the `texture` uniform to `sf::Shader::CurrentTexture` and apply provided float uniforms.
- On failure, log an error and reset the `shader` pointer to `nullptr`.

## Main signature

```cpp
void InitializeShaderSystem(Eng::registry &reg,
    Eng::sparse_array<Com::Shader> &shaders);
```

## Notes

- Shaders are stored as `std::shared_ptr<sf::Shader>`.
- Idempotent: the system will not reload a shader already marked `isLoaded`.
