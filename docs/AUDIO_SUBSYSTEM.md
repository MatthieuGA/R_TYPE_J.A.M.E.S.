# Audio Subsystem Usage Guide

## Overview

The audio subsystem provides a clean, event-driven approach to playing sounds and music in the R-Type client. It follows the ECS architecture and provides a clean abstraction over SFML Audio.

## Architecture

### Components

1. **IAudioBackend** (Interface): Abstract interface for audio backends
2. **SFMLAudioBackend**: Concrete SFML implementation (the ONLY class that uses SFML Audio)
3. **AudioManager**: High-level audio manager that wraps the backend
4. **SoundRequest** (Component): Transient ECS component for requesting sounds
5. **AudioSystem**: ECS system that processes SoundRequest components

### Design Principles

- **Separation of Concerns**: Gameplay systems don't know about audio implementation
- **Non-blocking**: Audio never blocks the main thread
- **Swappable Backend**: Changing audio library requires only implementing IAudioBackend
- **Event-driven**: Sounds are requested via components, not direct calls

## Usage

### 1. Initialize the Audio Subsystem (main.cpp)

```cpp
#include "Engine/audio/AudioManager.hpp"
#include "Engine/audio/SFMLAudioBackend.hpp"

// Create backend and manager
auto audio_backend = std::make_unique<Audio::SFMLAudioBackend>();
Audio::AudioManager audio_manager(std::move(audio_backend));

// Register audio assets
audio_manager.RegisterAsset("shot", "Assets/shot.wav", false);
audio_manager.RegisterAsset("explosion", "Assets/explosion.wav", false);
audio_manager.RegisterAsset("bgm", "Assets/music.ogg", true);

// Pass to system initialization
init_registry(reg, window, audio_manager);
```

### 2. Play Sounds from Gameplay Systems

To play a sound effect, add a `SoundRequest` component to an entity:

```cpp
// In a shooting system
void ShootingSystem(Engine::registry &reg, /* ... */) {
    for (auto &&[i, player, input] : /* ... */) {
        if (input.shoot) {
            // Spawn projectile entity
            auto projectile = reg.spawn_entity();
            reg.emplace_component<Component::Projectile>(projectile, /* ... */);
            
            // Request shot sound
            reg.emplace_component<Component::SoundRequest>(projectile,
                Component::SoundRequest{"shot", 1.0f, false});
        }
    }
}
```

### 3. Play Sounds on Collision

```cpp
// In a collision handling system
void CollisionSystem(Engine::registry &reg, /* ... */) {
    for (auto &&[i, entity_a, entity_b] : detected_collisions) {
        // Handle collision logic...
        
        // Request explosion sound
        auto explosion_entity = reg.spawn_entity();
        reg.emplace_component<Component::SoundRequest>(explosion_entity,
            Component::SoundRequest{"explosion", 0.8f, false});
    }
}
```

### 4. Play Background Music

```cpp
// At game start or level transition
audio_manager.PlayMusic("bgm", true);  // true = loop

// To stop music
audio_manager.StopMusic();
```

### 5. Volume and Mute Controls

```cpp
// Set volumes (0.0 to 1.0)
audio_manager.SetSfxVolume(0.7f);
audio_manager.SetMusicVolume(0.5f);

// Mute/unmute
audio_manager.MuteSfx(true);
audio_manager.MuteMusic(false);
```

## Component Reference

### SoundRequest (POD Component)

```cpp
struct SoundRequest {
    std::string sound_id;  // Asset ID registered with AudioManager
    float volume = 1.0f;   // Volume multiplier (0.0 to 1.0)
    bool loop = false;     // Loop the sound (rarely used for SFX)
};
```

**Lifecycle**: This component is automatically removed after the sound is played by the AudioSystem.

## API Reference

### AudioManager

| Method | Description |
|--------|-------------|
| `RegisterAsset(id, path, is_music)` | Load a sound or music asset |
| `PlaySound(id, volume)` | Play a one-shot sound effect |
| `PlayMusic(id, loop)` | Play background music |
| `StopMusic()` | Stop currently playing music |
| `SetSfxVolume(volume)` | Set SFX category volume |
| `SetMusicVolume(volume)` | Set music category volume |
| `MuteSfx(mute)` | Mute/unmute SFX |
| `MuteMusic(mute)` | Mute/unmute music |
| `Update()` | Process queued audio (called by AudioSystem) |

### IAudioBackend (for implementing custom backends)

| Method | Description |
|--------|-------------|
| `LoadSound(id, path)` | Load a sound buffer |
| `LoadMusic(id, path)` | Load a music stream |
| `Play(request)` | Queue a playback request |
| `StopMusic()` | Stop music playback |
| `SetCategoryVolume(category, volume)` | Set category volume |
| `SetCategoryMute(category, mute)` | Mute category |
| `Update()` | Process queued commands |

## Best Practices

1. **Asset Registration**: Register all audio assets at startup, not during gameplay
2. **Transient Entities**: For one-shot sounds, create a temporary entity with only SoundRequest
3. **No Direct SFML**: Never use SFML Audio directly outside SFMLAudioBackend
4. **Volume Range**: Keep volumes between 0.0 and 1.0
5. **Non-blocking**: The audio backend handles threading; don't worry about it in gameplay code

## Example: Complete Shooting System with Audio

```cpp
void ShootingSystem(Engine::registry &reg,
                   Engine::sparse_array<Component::Player> &players,
                   Engine::sparse_array<Component::InputState> &inputs) {
    for (auto &&[i, player, input] : make_indexed_zipper(players, inputs)) {
        if (input.shoot && player.can_shoot) {
            // Spawn projectile
            auto projectile = reg.spawn_entity();
            reg.emplace_component<Component::Transform>(projectile,
                Component::Transform{player.x, player.y, 0.0f, 1.0f});
            reg.emplace_component<Component::Projectile>(projectile,
                Component::Projectile{10.0f, 1});
            
            // Add sound request (will be automatically removed after playing)
            reg.emplace_component<Component::SoundRequest>(projectile,
                Component::SoundRequest{"shot", 1.0f, false});
            
            player.can_shoot = false;
            player.shoot_cooldown = 0.5f;
        }
    }
}
```

## Non-Goals (Not Implemented)

- Positional/spatial 3D audio
- DSP effects or mixing buses
- Persistent AudioSource components
- Real-time audio synthesis
- Audio ducking or advanced mixing

These features can be added later if needed by extending the backend interface.

## Replacing the Audio Backend

To use a different audio library (e.g., miniaudio, OpenAL):

1. Create a new class implementing `IAudioBackend`
2. Replace `SFMLAudioBackend` instantiation in `main.cpp`
3. No other code changes required!

```cpp
// Example: Custom backend
auto audio_backend = std::make_unique<Audio::CustomAudioBackend>();
Audio::AudioManager audio_manager(std::move(audio_backend));
```
