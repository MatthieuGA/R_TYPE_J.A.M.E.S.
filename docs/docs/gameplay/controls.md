---
sidebar_position: 2
---

# Controls Reference

Complete guide to controlling your spacecraft in R-Type J.A.M.E.S.

## 🎮 Input Devices

R-Type J.A.M.E.S. supports multiple input devices:
- ✅ **Keyboard** (Primary, fully supported)
- ✅ **Gamepad** (Xbox, PlayStation, Generic USB controllers)
- 🚧 **Mouse** (Planned for menu navigation)

## ⌨️ Keyboard Controls

### Default Keybindings

| Action | Primary Key | Alternative Key | Description |
|--------|-------------|-----------------|-------------|
| **Move Up** | `↑` | `W` | Move ship upward |
| **Move Down** | `↓` | `S` | Move ship downward |
| **Move Left** | `←` | `A` | Move ship left |
| **Move Right** | `→` | `D` | Move ship right |
| **Fire** | `Space` | - | Fire primary weapon |
| **Charge Shot** | Hold `Space` | - | Charge and release for powerful shot |
| **Pause/Menu** | `Esc` | `P` | Pause game or open menu |

### Movement Characteristics

- **Speed**: Constant movement speed (no acceleration)
- **Precision**: Pixel-perfect control for dodging
- **Diagonal Movement**: Combine directional keys for diagonal movement
- **Boundary Limits**: Automatic screen edge prevention

:::tip Pro Tip
Use WASD for movement if you're more comfortable with FPS-style controls. Both arrow keys and WASD work simultaneously!
:::

## Settings

You can modify a lot of settings, like :
- bigger text
- input remapping
- gameplay changes (difficulty settings, rule changes...)

### Planned Configuration File

Controls will be configurable via `config/controls.json`:

```json
{
  "keyboard": {
    "move_up": ["Up", "W"],
    "move_down": ["Down", "S"],
    "move_left": ["Left", "A"],
    "move_right": ["Right", "D"],
    "fire": ["Space"],
    "pause": ["Escape", "P"]
  },
  "gamepad": {
    "move": "LeftStick",
    "fire": "ButtonA",
    "pause": "Start"
  }
}
```

## 🎯 Movement Guide

### Basic Movement

```
       ↑ (Up)
        |
  ←─────┼─────→
   (Left) (Right)
        |
       ↓ (Down)
```

### Diagonal Movement

Combine two directional keys for diagonal movement:

```
  ↖ (Up+Left)    ↗ (Up+Right)




  ↙ (Down+Left)  ↘ (Down+Right)
```

**Speed**: Diagonal movement maintains the same speed as cardinal directions (normalized vectors).

### Screen Boundaries

```
┌──────────────────────────────┐
│ ← Playfield Boundary →       │
│                              │
│     🚀 Your Ship             │
│                              │
│                              │
└──────────────────────────────┘
```

- You cannot move outside the visible playfield
- Movement is automatically clamped to screen edges
- No "wrapping" around screen edges

## 🔫 Firing Mechanics

### Rapid Fire

Press and release `Space` repeatedly for rapid fire:
- **Fire Rate**: Limited by weapon cooldown
- **Projectile Speed**: Fast, straight trajectory
- **Damage**: Standard damage per shot

```
Player: ░░▶ Space Space Space
Output: 🚀 ─→ ─→ ─→
```

### Charge Shot *(Planned Feature)*

Hold `Space` to charge, release to fire:

1. **Press and Hold** `Space`
2. **Visual Indicator**: Ship glows/charges
3. **Release** when fully charged
4. **Powerful Blast**: Deals 3x damage

```
Player: ░░▶ Hold Space (1s)
Charge: 🚀 ✧✦✦✦ (charging)
Release: 🚀 ━━━▶ (powerful shot)
```

:::warning Charge Time
While charging, you cannot fire regular shots. Plan your charge timing carefully!
:::

## ⏸️ Pause Menu

Press `Esc` to pause the game:

### Pause Menu Options

- **Resume**: Return to game
- **Settings** *(Coming Soon)*: Adjust volume, graphics
- **Quit to Menu**: Return to main menu
- **Quit Game**: Exit application

:::note Multiplayer Behavior
In multiplayer, pausing only affects your local view. The server and other players continue running. Use this for brief breaks only.
:::

## 🎛️ Advanced Control Techniques

### Dodging

- **Predictive Movement**: Watch enemy bullet trajectories
- **Perpendicular Dodging**: Move perpendicular to incoming projectiles
- **Safe Zones**: Learn enemy spawn patterns to position optimally

### Kiting

- **Backward Fire**: Fire while moving backward to maintain distance
- **Circle Strafing**: Move in circles around slow enemies
- **Hit and Run**: Attack, then retreat to safety

### Precision Movement

For tight corridors and bullet hell sections:
- Use **small, controlled taps** on directional keys
- Avoid holding keys continuously
- Practice **feathering** inputs for micro-adjustments

## 🔧 Troubleshooting Controls

### Keyboard Not Responding

1. **Check Focus**: Click on the game window to ensure it has focus
2. **Test Keys**: Try alternate keys (WASD if arrows don't work)
3. **Restart**: Close and relaunch the client

### Gamepad Not Detected

1. **Connect First**: Plug in controller before launching game
2. **Check USB**: Try different USB port
3. **Driver Update**: Ensure controller drivers are installed
4. **Test Tool**: Use jstest-gtk (Linux) or joy.cpl (Windows) to verify controller works

### Input Lag

If you experience input lag:
- **V-Sync**: Disable V-Sync if enabled
- **Polling Rate**: Ensure controller has standard polling rate
- **CPU Load**: Close background applications
- **Network**: In multiplayer, check network latency

### Multiple Inputs

If multiple inputs register from one press:
- **Sticky Keys**: Disable OS sticky keys feature
- **Controller Calibration**: Recalibrate controller in OS settings
- **Clean Keyboard**: Physical debris may cause key bounce

## 📱 Platform-Specific Notes

### Linux

- Controllers may require **xboxdrv** or **SDL** for proper detection
- Some keyboards may have **key repeat** enabled in OS - this is normal
- Use `evtest` to verify controller input events

### Windows

- Xbox controllers work natively
- PlayStation controllers require **DS4Windows** or **similar drivers**
- Generic controllers require **DirectInput** drivers

### macOS *(Coming Soon)*

- Support planned for future releases
- Will use native macOS input APIs

## 📚 Related Documentation

- [How to Play](./how-to-play.md) - General gameplay guide
- [Input Abstraction Layer](../input-abstraction.md) - Technical input system architecture

---

## 🎮 Control Layout Visual Reference

### Keyboard Layout

```
┌─────┬─────┬─────┐          ┌─────┐
│  W  │     │     │          │ ↑   │
│  ↑  │     │     │     OR   │     │
├─────┼─────┼─────┤          ├─────┤
│  A  │  S  │  D  │          │← ↓ →│
│  ←  │  ↓  │  →  │          │     │
└─────┴─────┴─────┘          └─────┘

         ┌───────────┐
         │   SPACE   │ = Fire
         └───────────┘
```

### Gamepad Layout

```
      Start
       [⏸]

    [↑]           ╭─────╮
[←] [↓] [→]      │  ʘ  │ = Left Stick
                 ╰─────╯

              [Y]
          [X]     [B]
              [A] ← Fire
```

---

*Last Updated: January 2026*
