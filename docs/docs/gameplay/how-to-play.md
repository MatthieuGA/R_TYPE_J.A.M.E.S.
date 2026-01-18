---
sidebar_position: 1
---

# How to Play R-Type J.A.M.E.S.

## 🎮 Game Overview

R-Type J.A.M.E.S. is a classic side-scrolling shoot 'em up where you pilot a spacecraft to battle through waves of alien enemies. Master precise movement, strategic shooting, and power-up management to survive increasingly challenging levels.

## 🚀 Starting the Game

### Single Player

1. Launch the client:
   ```bash
   ./r-type_client
   ```
   Or on Windows:
   ```cmd
   r-type_client.exe
   ```

2. The client will automatically attempt to connect to a local server (`localhost:4242`)
3. Once connected, you'll spawn as Player 1 in the game world

### Multiplayer (Local Network)

1. **Start the Server:**
   ```bash
   ./r-type_server 4242
   ```

2. **Connect Clients:**
   Each player runs the client and specifies the server IP:
   ```bash
   ./r-type_client <server-ip> 4242
   ```
   Example:
   ```bash
   ./r-type_client 192.168.1.100 4242
   ```

3. The server supports up to **4 players** simultaneously

### Level Selection

When starting the server, you'll be prompted to select a level:
- Enter the number corresponding to your chosen level
- Levels can be finite (with an ending) or infinite (endless waves)
- The server will display available levels with their properties

## 🎯 Game Objective

- **Survive** waves of enemies by avoiding obstacles and enemy fire
- **Destroy** all enemies to progress through the level
- **Score** points by eliminating enemies and collecting power-ups
- **Reach** the end of the level (finite levels) or survive as long as possible (infinite levels)

## 🕹️ Controls

### Keyboard (Default)

| Action | Key |
|--------|-----|
| **Move Up** | `↑` (Up Arrow) or `W` |
| **Move Down** | `↓` (Down Arrow) or `S` |
| **Move Left** | `←` (Left Arrow) or `A` |
| **Move Right** | `→` (Right Arrow) or `D` |
| **Fire** | `Space` |
| **Charge Shot** | Hold `Space` |
| **Pause** | `Esc` |

### Gamepad Support

- **D-Pad / Left Stick**: Movement
- **Button A**: Fire/Charge
- **Start**: Pause

:::tip Movement Tips
- Your ship moves continuously - you don't need to hold "Right to maintain speed
- Diagonal movement is allowed for precise dodging
- Stay within the playfield boundaries (movement is automatically limited)
:::

## 👾 Enemy Types

### Basic Enemies
- **Daemons**: Small, fast-moving enemies that fly in formation
- **Kamikaze Fish**: Enemies that dive directly toward your position
- **Golems**: Slower, tankier enemies that take multiple hits

### Advanced Enemies
- **Mermaids**: Mid-tier enemies with unpredictable movement patterns
- **Wave Formations**: Groups of enemies that attack in coordinated waves

### Bosses *(Coming Soon)*
- Large enemies with complex attack patterns and multiple phases

:::info Enemy Behavior
- Enemies spawn from JSON definitions (located in `data/` folder)
- Each enemy type has unique health, speed, and AI behavior
- Some enemies drop power-ups when destroyed
:::

## 💎 Power-Ups *(Planned Feature)*

Power-ups will enhance your ship's capabilities:
- **Speed Boost**: Increase movement speed temporarily
- **Weapon Upgrade**: Enhanced firepower
- **Shield**: Absorbs damage for a limited time
- **Multi-Shot**: Fire multiple projectiles simultaneously

## ❤️ Health System

- You start with **100 HP**
- Contact with enemies or enemy projectiles deals damage
- Health is displayed on the HUD
- When health reaches 0, you are respawned (if lives remain)

## 📊 Scoring System

- **Enemy Kill**: Points vary by enemy type (check JSON definitions)
- **Wave Completion**: Bonus points for clearing all enemies
- **Survival Time**: Points accumulate over time in infinite levels
- **Multiplayer Bonus**: Team score increases with cooperation

## 🌟 Gameplay Tips

### Survival Strategies

1. **Stay Mobile**: Constant movement makes you harder to hit
2. **Learn Patterns**: Enemies follow predictable patterns - memorize them
3. **Use the Full Screen**: Don't stay in one area - use all available space
4. **Prioritize Threats**: Focus on enemies that shoot projectiles first
5. **Team Coordination**: In multiplayer, spread out to cover more area

### Advanced Techniques

- **Charge Shots**: Hold fire to charge a more powerful shot (when implemented)
- **Safe Zones**: Learn where enemies don't spawn and use them strategically
- **Edge Play**: Stay near screen edges to give yourself more reaction time
- **Projectile Dodging**: Watch projectile trajectories and move perpendicular to them

## 🎭 Multiplayer Strategies

### Cooperative Play

- **Divide and Conquer**: Split coverage between top and bottom screen areas
- **Focus Fire**: Concentrate fire on tough enemies
- **Revive System**: Stay alive to help fallen teammates respawn
- **Communication**: Call out threats and coordinate movements

### Roles

- **Point Player**: Stays forward, attracts enemy attention
- **Support**: Cleans up enemies from safe distance
- **Scout**: Explores ahead to warn of incoming waves

## 🔧 Game Configuration

### Graphics Settings

Currently, graphics settings are configured at compile-time. Future updates will include:
- Resolution options
- Fullscreen toggle
- V-Sync control
- Graphics quality presets

### Network Settings

- **Default Port**: `4242` (UDP)
- **Latency Tolerance**: The game uses client-side prediction and server reconciliation
- **Snapshot Rate**: Server sends 20 snapshots per second
- **Input Rate**: Client sends inputs 60 times per second

## 📱 HUD Elements

Your heads-up display shows:
- **Health Bar**: Current HP (top-left)
- **Score**: Current points (top-right)
- **Lives**: Remaining lives (if multiple lives enabled)
- **Player ID**: Your player number (P1-P4) in multiplayer

## ⚠️ Common Issues

### Connection Problems

**Problem**: "Cannot connect to server"
- **Solution**: Ensure server is running first
- Check that you're using the correct IP and port
- Verify firewall settings allow UDP traffic on port 4242

**Problem**: High latency/lag
- **Solution**: Server and clients should be on same local network
- Check for network congestion
- Verify server machine has sufficient resources

### Gameplay Issues

**Problem**: Controls not responding
- **Solution**: Click on game window to ensure it has focus
- Check that another application isn't capturing input
- Try keyboard if gamepad isn't working (or vice versa)

**Problem**: Low framerate
- **Solution**: Close other applications
- Ensure graphics drivers are up to date
- Check system meets minimum requirements

## 🎓 Tutorials

### First Time Playing?

1. **Tutorial Level**: Start with Level 1 (easiest)
2. **Practice Movement**: Spend time just moving around, avoiding obstacles
3. **Learn to Fire**: Practice shooting enemies without getting hit
4. **Try Multiplayer**: Team up with friends for cooperative play

### Improving Your Skills

1. **Replay Levels**: Practice makes perfect - replay levels to improve scores
2. **Watch Patterns**: Observe enemy spawn patterns and timing
3. **Speed Runs**: Try to complete levels as fast as possible
4. **Challenge Modes**: Try infinite levels to test endurance

## 🏆 Achievements *(Coming Soon)*

Future updates will include:
- Score milestones
- Level completion achievements
- Multiplayer cooperative achievements
- Speedrun leaderboards

## 📚 Additional Resources

- [Game Design Document](../game-design.md) - Complete gameplay mechanics
- [Controls Reference](./controls.md) - Detailed control mapping
- [Multiplayer Guide](../network/multiplayer-guide.md) - Advanced multiplayer setup

## 🎉 Have Fun!

R-Type J.A.M.E.S. is designed to be challenging but fair. Don't get discouraged by deaths - each playthrough teaches you enemy patterns and improves your reflexes. Good luck, pilot!

---

*Last Updated: January 2026*
