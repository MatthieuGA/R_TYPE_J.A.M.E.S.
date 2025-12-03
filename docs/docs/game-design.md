# Game Design Document (GDD) — R-Type J.A.M.E.S.

**Project:** R-Type J.A.M.E.S.
**Genre:** Horizontal Shoot'em'up (Shmup)
**Platforms:** Linux, Windows
**Network:** Multiplayer competitive (up to 255 players), client-server architecture
**Version:** 2.0.4
**Last Updated:** December 3, 2025

## Table of Contents

1. [Overview & Vision](#1-overview--vision)
   1. [Game Concept](#11-game-concept)
   2. [Core Vision](#12-core-vision)
   3. [Design Philosophy](#13-design-philosophy)
   4. [Reference & Inspiration](#14-reference--inspiration)
2. [Core Mechanics (Global Level)](#2-core-mechanics-global-level)
   1. [Movement](#21-movement)
   2. [Shooting](#22-shooting)
   3. [Collision & Health](#23-collision--health)
   4. [Spawning Logic](#24-spawning-logic)
   5. [Scoring](#25-scoring)
   6. [Progression & Difficulty](#26-progression--difficulty)
   7. [Powerups & The Force](#27-powerups--the-force)
   8. [Game Loop](#28-game-loop)
3. [Entity Roles (Global Level)](#3-entity-roles-global-level)
   1. [Player](#31-player)
   2. [Enemy (Bydos)](#32-enemy-bydos)
   3. [Bullet / Missile (Projectile)](#33-bullet--missile-projectile)
   4. [Powerup / Force](#34-powerup--force)
   5. [Obstacle](#35-obstacle)
   6. [Spawner](#36-spawner)
   7. [Boss](#37-boss)
4. [Gameplay Examples & Scenarios](#4-gameplay-examples--scenarios)
   1. [Scenario 1: Early Wave Encounter](#scenario-1-early-wave-encounter)
   2. [Scenario 2: Mid-Level Challenge with Force](#scenario-2-mid-level-challenge-with-force)
   3. [Scenario 3: Boss Encounter — Dobkeratops](#scenario-3-boss-encounter--dobkeratops)
5. [Controls & Input](#5-controls--input)
   1. [Default Keyboard Mapping](#51-default-keyboard-mapping)
   2. [Gamepad Support (Optional)](#52-gamepad-support-optional)
   3. [Input Responsiveness](#53-input-responsiveness)
   4. [Remapping Support (Accessibility Requirement)](#54-remapping-support-accessibility-requirement)
6. [Player Experience: Feel & Readability Goals](#6-player-experience-feel--readability-goals)
   1. [Responsiveness (Feel)](#61-responsiveness-feel)
   2. [Visual Clarity (Readability)](#62-visual-clarity-readability)
   3. [Audio Design (Feel)](#63-audio-design-feel)
7. [Accessibility Intentions & Requirements](#7-accessibility-intentions--requirements)
   1. [Visual Accessibility](#71-visual-accessibility)
   2. [Motor/Physical Accessibility](#72-motorphysical-accessibility)
   3. [Audio/Hearing Accessibility](#73-audiohearing-accessibility)
   4. [Cognitive Accessibility](#74-cognitive-accessibility)
8. [Multiplayer & Networking Considerations](#8-multiplayer--networking-considerations)
   1. [Player Count & Roles](#81-player-count--roles)
   2. [Server-Authoritative Model](#82-server-authoritative-model)
   3. [Client-Side Prediction & Reconciliation](#83-client-side-prediction--reconciliation)
   4. [Visual Identification in Multiplayer](#84-visual-identification-in-multiplayer)
   5. [Friendly Fire Rules](#85-friendly-fire-rules)
   6. [Disconnect Handling](#86-disconnect-handling)
9. [Playtest Checklist](#9-playtest-checklist-accessibility--feel-verification)
   1. [Responsiveness](#91-responsiveness)
   2. [Visual Clarity](#92-visual-clarity)
   3. [Accessibility](#93-accessibility)
   4. [Gameplay Feel](#94-gameplay-feel)
   5. [Multiplayer & Networking](#95-multiplayer--networking)

---

## 1. Overview & Vision

### 1.1 Game Concept

R-Type J.A.M.E.S. is a networked reimagining of the classic 1987 horizontal shoot'em'up R-Type. Players pilot spacecraft through waves of the alien Bydos forces, fighting for survival in a side-scrolling space battlefield. The game emphasizes competitive multiplayer gameplay where up to 4 players play against each other against increasingly challenging enemy waves and bosses.

### 1.2 Core Vision

- **Networked Competitive Play:** Multiple players play against each other across a network, with a central authoritative server managing all game state.
- **Classic Shmup Feel:** Faithful to the original R-Type's tight controls, challenging enemies, and iconic powerup system.
- **Server Authority:** All critical gameplay decisions (collision, damage, spawning, scoring) are computed server-side to ensure fairness and consistency.
- **Accessible & Readable:** Clear visual distinction between entities, customizable controls, and support for players with various accessibility needs.

### 1.3 Design Philosophy

- **Responsiveness First:** Minimize perceived input lag through client-side prediction while maintaining server authority.
- **Visual Clarity:** Players must always know what's friendly, what's hostile, and what's dangerous.
- **Progressive Difficulty:** Start accessible, scale challenge through enemy patterns and boss mechanics.
- **Replayability:** Random enemy spawns, score competition, and multiplayer coordination encourage multiple playthroughs.

### 1.4 Reference & Inspiration

The original R-Type (1987, Irem) and its sequels provide the foundation. Similar games include Gradius, Blazing Star, and other horizontal shmups. Key R-Type elements preserved:

- Horizontal scrolling starfield background
- The iconic Force pod (attachable/detachable powerup)
- Memorable boss encounters (e.g., Dobkeratops)
- Destroyable environmental tiles
- Charged shot mechanics

---

## 2. Core Mechanics (Global Level)

### 2.1 Movement

**Player Control:**

- Players control a spacecraft with **4-directional or 8-directional movement** using arrow keys (or remappable controls).
- Movement is **continuous and analog** within the play area boundaries.
- **Speed:** Base movement speed allows players to dodge incoming fire while positioning for attacks. Powerups may temporarily boost speed.
- **Boundaries:** Players cannot leave the visible screen area. Attempting to move beyond boundaries simply stops movement in that direction.
  - **Crushing Rule:** If the scrolling screen pushes a player against a solid obstacle or the left edge of the screen, the player is instantly destroyed (Instant Death), regardless of remaining Health Points.

**Frame-Independent Timing:**

- All movement, spawning, and animations use **delta-time** or fixed timestep to ensure consistent behavior regardless of CPU speed or frame rate.

### 2.2 Shooting

**Basic Fire:**

- Players hold a **shoot button** (e.g., Spacebar or Ctrl) to continuously fire projectiles forward.
- **Fire rate:** Configurable interval between shots (e.g., 10 shots/second at base level).
- Projectiles travel horizontally (or at slight angles for upgraded weapons) until hitting an enemy, obstacle, or screen boundary.

**Charged Shot:**

- Players can **hold the fire button** to charge a more powerful shot.
- After a charge duration (e.g., 2 seconds), the next shot is a **charged missile** dealing significantly more damage and possibly penetrating multiple enemies.
- Visual/audio feedback indicates charge level.

**Weapon Upgrades:**

- Pickups modify the player's weapon:
  - **Spread shot:** Multiple bullets in a fan pattern.
  - **Laser:** Continuous beam dealing damage to anything in path.
  - **Homing missiles:** Projectiles track nearest enemy.

### 2.3 Collision & Health

**Player Health:**

- **Structure:** Players start with **1 Life** and **100 HP (Health Points)**.
- **Damage:** Taking damage from enemies or projectiles reduces HP (amount depends on enemy type).
- **Death:** When HP reaches 0, or upon "Crushing" (scrolling collision), the player loses their Life.
- **Elimination:** Since players start with 1 Life, death typically means elimination from the round (switching to Spectator Mode), unless an "Extra Life" item was collected previously.
- **Respawn Mechanics:** *If* a player has an Extra Life, they respawn with temporary invincibility (2 seconds), visual blinking, and ghosting is active.

**Enemy Health:**

- Basic enemies: destroyed in 1-3 hits.
- Tougher enemies: require more hits, may have distinct vulnerable points.
- Bosses: large health pools, multi-phase battles with changing attack patterns.

**Collision Rules:**

- **Player vs. Player:** **Ghosting is active.** (players pass through each other physically; no blocking), preventing griefing.
- **Player vs. Enemy/Projectile:** Reduces Player HP.
- **Player vs. Obstacle:** Wall collision blocks movement. Being crushed by scrolling = Instant Death.

### 2.4 Spawning Logic

**Enemy Waves:**

- Enemies **spawn on the right side of the screen** at intervals determined by the level script.
- Spawn timing and composition are **strictly deterministic** (fully scripted waves).
- **No Randomness:** To ensure competitive fairness, there is no RNG in enemy types or spawn patterns. Every match plays identically if players perform identical actions.

**Spawn Triggers:**

- Time-based: every X seconds, spawn Y enemies.
- Event-based: after previous wave is cleared, spawn next wave.
- Boss triggers: after certain number of waves or time, boss spawns.

**Monster Behavior:**

- Each enemy type has a predefined **movement pattern** (straight line, sine wave, snake pattern, stationary turret).
- Enemies follow pattern until destroyed or leaving the screen (left side despawns them).

**Data Driven Design:**
All spawn logic (wave composition, timing, enemy types) MUST be loaded from external **JSON configuration files**. No hardcoded values in the engine.

### 2.5 Scoring

**Point System:**

- **Pro-rata Scoring:** Points for destroying an enemy are distributed among players based on the **% of damage dealt**.
- **Last Hit Bonus:** The player who deals the killing blow receives a small flat bonus (e.g., +5%).
- **Competitive Multiplier:** To balance the fixed difficulty (see 2.6), a score multiplier is applied to living players based on how many players have died.
  - Formula: `Multiplier = 1 + 0.5 * (Initial_Players - Living_Players)`
  - *Example:* In a 4-player game, if 3 die, the survivor earns **x2.5 points** per action.

**Leaderboard:**

- Scores are tracked per session and globally in the server's database (or persistent storage)
- Server can broadcast high scores to all clients.

### 2.6 Progression & Difficulty

**Level Stages:**

- The game is divided into **stages/levels** with distinct visual themes and enemy compositions.
- Each stage ends with a **boss encounter**.
- Scrolling continues automatically; players cannot backtrack.

**Difficulty Scaling:**

- **Fixed Challenge:** Difficulty (enemy density, health, damage) is determined at the **start of the match** based on the initial player count (e.g., 4 Players = Difficulty x2.5).
- **No Downscaling:** The difficulty **does not decrease** if players die during the match. The last survivor must face the challenge intended for the full team, but receives a massive Score Multiplier (see 2.5) as a reward for the risk.
- **Boss HP:** Boss Health is fixed at the start and does not scale down dynamically.

### 2.7 Powerups & The Force

**Powerup Drops & Scripting:**

- **Scripted Fairness:** Critical powerups (especially The Force) are **scripted via JSON** to appear in waves (e.g., 4 transporters carrying 1 Force each) to guarantee every player has an opportunity to collect one.
- **No RNG:** Powerups are not random drops. They are assigned to specific enemies in the level data.
- **Competition:** Since drops are fixed, players must compete to destroy the carrier enemy to claim the item.

**Weapon Upgrades (Generic):**

- **Rapid Fire:** Increases shooting frequency.
- **Spread Shot:** Fires 3 projectiles in a cone.
- **Laser:** Penetrating beam, slower fire rate.
- **Speed Up:** Increases movement speed (caps at 3 stacks).
- **Repair:** Restores a portion of HP (New).

**The Force (Core Mechanic):**
The Force is a small pod that acts as the player's primary defensive and offensive tool.

- **Attaches** to the front or back of the player ship.
- **Shielding:** When attached, it blocks all enemy projectiles and damages enemies on contact.
- **Shooting:** It fires additional projectiles (dependent on current weapon).
- **Detachable:** Can be launched forward to act as an autonomous turret.
- **Recall:** Can be recalled to the ship, damaging enemies in its return path.

**Force Behavior:**

- **Invulnerable:** The Force cannot be destroyed.
- **Contact Damage:** The Force deals damage to any enemy it touches (attached or detached).

### 2.8 Game Loop

**Session Flow:**

1. Launch Executable with arguments (Server IP/Port).
2. Input Username.
3. Join Lobby.
4. **Ready Check:** All players must toggle "Ready". Game can start with min. 1 player.
5. **Game Start:** Loading JSON configuration for the match.
6. Gameplay (Waves -\> Boss -\> Next Stage).
7. End of Game (All players dead or Game Completed).

**Win Condition:**

- **Highest Score:** The player with the highest score at the end of the session wins.
- *Note:* It is possible to die at the boss but still win if the accumulated score (via damage/multiplier) is higher than the survivor's.

**Loss Condition:**

- Being eliminated with a lower score than opponents.

---

## 3. Entity Roles (Global Level)

### 3.1 Player

**Role:**  
The player-controlled spacecraft. The primary avatar through which the human player interacts with the game.

**Responsibilities:**

- Receive and execute player input (movement, shooting, Force control).
- Maintain health/lives state.
- Render at correct position with distinct visual identifier (color or sprite variation for multiplayer).

**Key Attributes:**

- Position (X, Y coordinates)
- Velocity (movement speed)
- Lives remaining
- Current weapon type
- Force attachment state (attached front/back, detached, absent)

**Interactions:**

- Collision with enemies/enemy projectiles → take damage.
- Collision with powerups → apply powerup effect.
- Shooting → spawn player projectiles.

### 3.2 Enemy (Bydos)

**Role:**
Hostile entities controlled by the game logic (server). Provide challenge and targets for players.

**Responsibilities:**

- Follow predefined movement patterns.
- Shoot projectiles at intervals (if applicable).
- Take damage from player projectiles.
- Drop powerups on destruction (if designated).

**Types:**

- **Basic Flyer:** Moves in a straight line or simple pattern, low health, shoots occasionally.
- **Snake Pattern:** Moves in sinusoidal wave, moderate health.
- **Ground Turret:** Stationary or slow-moving, shoots rapidly at player.
- **Heavy Enemy:** Moves slowly, high health, fires spread shots.
- **Boss:** See section 3.7.

**Key Attributes:**

- Position & velocity
- Health points
- Movement pattern script
- Fire rate & projectile type
- Powerup drop flag

**Interactions:**

- Collision with player → damage player.
- Hit by player projectile → take damage, possibly destroyed.
- Destroyed → award points, possibly drop powerup, remove from game.

### 3.3 Bullet / Missile (Projectile)

**Role:**
Ranged attack entities fired by players or enemies.

**Responsibilities:**

- Travel in a straight line (or curved path for homing missiles).
- Detect collision with targets (enemies for player bullets, players for enemy bullets).
- Apply damage on hit.
- Self-destruct after collision or leaving screen bounds.

**Types:**

- **Player Bullet:** Standard shot, low damage, fast travel.
- **Player Charged Shot:** High damage, may penetrate multiple enemies.
- **Player Missile:** Homing or spread pattern.
- **Enemy Bullet:** Standard hostile projectile.
- **Enemy Heavy Missile:** Slow, high damage projectile.

**Key Attributes:**

- Position & velocity
- Damage value
- Owner (player ID or enemy ID)
- Lifespan / max travel distance

**Interactions:**

- Collision with target → apply damage, destroy self.
- Collision with Force → absorbed (if player projectile hits enemy Force or vice versa).
- Leave screen → destroy self.

### 3.4 Powerup / Force

**Role:**
Collectible items that enhance player capabilities.

**Responsibilities:**

- Spawn at enemy death location (for drops) or fixed level positions.
- Float/drift slowly downward or leftward.
- Detect collision with player.
- Apply effect on collection (weapon change, Force attachment, life gain).

**Types:**

- **Force Pod:** Primary powerup; attaches to player, enables special abilities.
- **Weapon Upgrade Icon:** Changes player's shot type.
- **Speed Boost Icon:** Temporary speed increase.
- **Extra Life Icon:** Grants +1 life.

**Key Attributes:**

- Position
- Powerup type
- Lifespan (despawn after X seconds if not collected)

**Interactions:**

- Collision with player → apply effect, remove from game.
- Timeout → despawn.

### 3.5 Obstacle

**Role:**
Environmental hazards or level geometry that players and enemies must navigate around.

**Responsibilities:**

- Occupy space in the level.
- Block or damage entities on collision.
- Some obstacles are destroyable; others are indestructible.

**Types:**

- **Destroyable Tile:** Can be shot and destroyed by player, may block enemy fire.
- **Indestructible Wall:** Damages player on contact, blocks all projectiles.
- **Stage Hazard:** Moving obstacles (e.g., closing doors, rotating debris).

**Key Attributes:**

- Position & size (hitbox)
- Destructible flag
- Health (if destructible)

**Interactions:**

- Collision with player → damage player (if hazardous).
- Hit by projectile → take damage (if destructible), block projectile.

### 3.6 Spawner

**Role:**
Logic entity (not visually represented) that generates enemies at specified intervals.

**Responsibilities:**

- Trigger enemy spawns based on time, wave progression, or event conditions.
- Select enemy type and spawn position.
- Manage wave composition (e.g., 5 basic enemies, then 2 snake enemies, then 1 heavy).

**Key Attributes:**

- Spawn schedule (list of wave definitions)
- Current wave index
- Spawn timer

**Interactions:**

- Time elapsed → spawn enemy.
- Wave complete → advance to next wave.
- Boss trigger → spawn boss entity.

### 3.7 Boss

**Role:**  
Large, powerful enemy encountered at the end of stages. Provides climactic challenge.

**Responsibilities:**

- Execute complex attack patterns (multi-phase behavior).
- High health pool; requires sustained damage to defeat.
- Change behavior at health thresholds (phase transitions).

**Example: Dobkeratops (Stage 1 Boss):**

- **Visuals:** Large bio-mechanical creature fused with the environment. Static body, only head/tentacles move.
- **Weak Point:** The alien head protruding from the chest.
- **Attacks:** Projectiles from mouth, seeking eyes, or extending tentacles. (No flying detached tail).

**Key Attributes:**

- Position & large hitbox
- Health (significantly higher than normal enemies)
- Phase state
- Multiple attack scripts per phase

**Interactions:**

- Hit by player projectiles → accumulate damage, phase transition on thresholds.
- Defeated → award large score bonus, drop rare powerup, trigger stage clear.

---

## 4. Gameplay Examples & Scenarios

### Scenario 1: Early Wave Encounter

**Context:** Player(s) have just started Stage 1. The starfield scrolls slowly to the right.

**Flow:**

1. Three **basic flyers** spawn from the right side, moving left in a straight line.
2. Players shoot continuously; bullets travel right and hit enemies.
3. First enemy destroyed → awards 100 points.
4. Second enemy drops a **weapon upgrade icon** (spread shot).
5. Player moves up to collect powerup → weapon switches to spread shot (3 bullets per shot).
6. Third enemy destroyed → wave complete.
7. Short pause, then next wave spawns (5 enemies in loose formation).

**Outcome:** Players learn basic shooting, movement, and powerup collection. No deaths expected in early waves if players stay alert.

---

### Scenario 2: Mid-Level Challenge with Force

**Context:** Stage 2, players have collected the Force pod.

**Flow:**

1. **Snake-pattern enemies** spawn in pairs, weaving up and down.
2. Player with Force attached to **front** moves into position.
3. Enemy bullets hit the Force → absorbed, player takes no damage.
4. Player shoots; Force also shoots, dealing double damage.
5. Snake enemy destroyed → drops **Force upgrade** (larger Force).
6. Simultaneously, **ground turrets** appear at the bottom of screen, shooting upward rapidly.
7. Player **detaches Force**, launching it toward turrets.
8. Force autonomously shoots turrets while player focuses on snakes.
9. Player **recalls Force** after turrets are destroyed.
10. Wave complete → score multiplier for no damage taken.

**Outcome:** Players experience the tactical depth of Force positioning and detachment. Demonstrates importance of managing threats from multiple directions.

---

### Scenario 3: Boss Encounter — Dobkeratops

**Context:** Stage 1 boss, the iconic Dobkeratops (large biomechanical creature).

**Flow:**

1. Boss enters from right side, filling a large portion of the screen.
2. **Phase 1:** Boss remains static (fused to environment), fires bullet spread from mouth every 2 seconds.
3. Players focus fire on head; Force shields frontal bullets.
4. After 30% health lost, **Phase 2** triggers: Boss extends tentacles and increases fire rate.
5. Players must dodge bullets and tentacle strikes.
6. Head destroyed → chest opens, enters **Phase 3**.
7. **Phase 3:** Core exposed in center, fires dense bullet curtain.
8. Players use charged shots for high damage bursts.
9. Boss destroyed → points are distributed to each player in proportion to the amount of damage they dealt to the boss (e.g., if a player dealt 30% of the total damage, they receive 30% of the points).

**Outcome:** Boss fight showcases multi-phase mechanics, coordination in multiplayer, and high-stakes dodge/shoot gameplay. Victory is rewarding both in score and progression.

---

## 5. Controls & Input

### 5.1 Default Keyboard Mapping

| Action              | Key                |
|---------------------|--------------------|
| Move Up             | Arrow Up           |
| Move Down           | Arrow Down         |
| Move Left           | Arrow Left         |
| Move Right          | Arrow Right        |
| Shoot               | Spacebar (hold)    |
| Charge Shot         | Spacebar (hold 2s) |
| Detach/Recall Force | Ctrl               |
| Pause (local)       | Esc                |

### 5.2 Gamepad Support (Optional)

- **Left Stick / D-Pad:** Movement
- **A / Button 1:** Shoot
- **B / Button 2:** Detach/Recall Force
- **Start:** Pause

### 5.3 Input Responsiveness

**Implementation Notes:**

- Client immediately renders local player movement on input (prediction).
- Server validates and corrects position if discrepancy detected (reconciliation).
- Shooting and collision are server-authoritative; client shows immediate muzzle flash but server determines hit registration.

### 5.4 Remapping Support (Accessibility Requirement)

- Players MUST be able to remap all controls via an in-game settings menu or configuration file.
- Support for keyboard-only, gamepad-only, or hybrid setups.
- Key bindings saved per player profile.

---

## 6. Player Experience: Feel & Readability Goals

### 6.1 Responsiveness (Feel)

**Design Intent:**
Players should feel **immediate control** over their spacecraft. Input lag is the enemy of enjoyment in fast-paced shmups.

**Measurable Targets:**
-- **Input-to-action latency (local):** &lt;50ms from key press to sprite position update.
-- **Networked latency (acceptable):** &lt;150ms round-trip for server validation to feel responsive.

- **Frame rate:** Maintain stable 60 FPS minimum; frame drops break player immersion and timing.

**Implementation Strategies:**

- Use **client-side prediction** for local player movement (client renders position immediately, server corrects if needed).
- **Interpolation** for remote players' positions to smooth network jitter.
- **Fixed timestep** for game logic to decouple rendering from simulation.

**Feedback Indicators:**

- **Visual:** Immediate muzzle flash on shoot button press, instant sprite movement on arrow key.
- **Audio:** Shoot sound effect plays on button press (client-side), hit sounds on confirmed server collision.
- **Haptic (optional):** Controller rumble on taking damage or destroying boss.

---

### 6.2 Visual Clarity (Readability)

**Design Intent:**
Players must instantly distinguish between friend, foe, danger, and collectibles. In a screen full of entities, clarity prevents frustration and cheap deaths.

**Measurable Targets:**

- **Entity Color Coding:**
  - Player bullets: **Blue** or **Green**
  - Enemy bullets: **Red** or **Orange**
  - Player ships: **Distinct colors per player** (P1=Blue, P2=Green, P3=Yellow, P4=Red)
  - Enemies: **Gray/Brown tones** with hostile red accents
  - Powerups: **Bright Yellow** or **Cyan** with sparkle effect
- **Size Differentiation:**
  - Player bullets: Small (4x4 pixels)
  - Enemy bullets: Medium (6x6 pixels), distinct shape (diamond or cross)
  - Player ships: Medium (16x16 to 32x32 pixels)
  - Enemies: Varied (16x16 for small, 64x64+ for bosses)
- **On-Screen Clutter Limit:**
  - **Entity Limit:** No arbitrary software limit (e.g., "50 entities").
  - The engine supports `u32` IDs, allowing for virtually unlimited active entities, constrained only by server hardware performance.

**Background vs. Foreground:**

- **Starfield:** Low contrast, muted colors (dark blue/black background, dim white/gray stars).
- **Foreground entities:** High contrast, bright colors, distinct outlines or glows.
- **Scrolling speed:** Slow enough that background doesn't distract but fast enough to convey movement (e.g., 10 pixels/second).

**HUD Readability:**

- **Score, Lives, Weapon Status** displayed in corners (top-left: score, top-right: P1 lives, etc.).
- **Font:** Large, bold, high contrast (white text on dark semi-transparent background).
- **HUD elements do NOT overlap gameplay area** (confined to screen edges).

---

### 6.3 Audio Design (Feel)

**Design Intent:**
Audio reinforces player actions, signals threats, and sets the game's tone (tension, excitement, triumph).

**Key Audio Categories:**

**Music:**

- **Stage themes:** Energetic, retro-inspired synth tracks reminiscent of 90s arcade games.
- **Boss themes:** Intensified tempo and instrumentation to signal heightened challenge.
- **Game over / Victory:** Short musical stings to punctuate outcomes.

**Sound Effects (SFX):**

- **Player actions:**
  - Shoot: Sharp, satisfying "pew" sound.
  - Charge shot: Building hum, then powerful "boom" on release.
  - Force detach: Mechanical click.
- **Enemy events:**
  - Enemy destroyed: Small explosion sound.
  - Boss destroyed: Large, layered explosion with rumble.
  - Enemy shoot: Distinct "hostile" sound (e.g., lower pitch than player).
- **Collectibles:**
  - Powerup pickup: Pleasant chime or "power-up" jingle.
- **Damage/Warning:**
  - Player hit: Sharp alarm sound.
  - Low health: Persistent beeping warning tone.
  - Boss warning: Deep siren before boss spawn.

**Positional Audio (Optional):**

- Enemies off-screen right: Sound panned right to indicate incoming threat.
- Requires stereo or surround sound support; improves spatial awareness.

**Volume Balance:**

- **Music:** Background, not overpowering SFX (60% volume).
- **SFX:** Clear, audible above music (80% volume).
- **Player-configurable:** Separate sliders for Music, SFX, and Master volume.

---

## 7. Accessibility Intentions & Requirements

### 7.1 Visual Accessibility

**Goal:** Ensure players with visual impairments (colorblindness, low vision) can distinguish entities and understand game state.

**Concrete Features:**

1. **High Contrast Mode:**
   - **Toggle in settings:** Increases color contrast for all entities.
   - **Effect:** Player bullets = Bright Cyan, Enemy bullets = Bright Magenta, Enemies = White outlines on black fill.
   - **Result:** Easier distinction for players with low vision or colorblindness.

2. **Colorblind-Friendly Palette:**
   - Avoid red-green as primary distinction (common colorblindness).
   - Use **blue-orange** or **blue-yellow** contrasts.
   - **Example:** Player = Blue ship with yellow accent, Enemies = Orange with white accent.

3. **Scalable UI/Text:**
   - **Settings option:** Small / Medium / Large text sizes for HUD.
   - **Effect:** HUD font size scales 100% / 150% / 200%.
   - **Result:** Players with low vision can read score and lives clearly.

4. **Clear Visual Cues for Critical Events:**
   - **Player hit:** Screen flash (red tint) + large "!" icon near player.
   - **Low health:** Screen border pulses red.
   - **Boss warning:** Large "WARNING" text + screen shake.
   - **Result:** Players don't rely solely on subtle indicators.

5. **Subtitles/Text Prompts:**
   - Any audio dialogue or narration has **text overlay**.
   - Boss warnings: Text displays "BOSS INCOMING" in addition to audio siren.
   - **Result:** Deaf/hard-of-hearing players receive same information.

---

### 7.2 Motor/Physical Accessibility

**Goal:** Accommodate players with limited mobility, motor control, or single-hand play.

**Concrete Features:**

1. **Full Control Remapping:**
   - Players can assign **any action to any key/button**.
   - **Result:** Players can configure controls to suit their physical capabilities (e.g., all controls on one hand, foot pedals, accessibility peripherals).

2. **Alternate Input Methods:**
   - **Keyboard-only play:** All actions mappable to keyboard.
   - **Gamepad-only play:** All actions mappable to gamepad.
   - **Mouse support (optional):** Movement via mouse cursor, shoot on click.
   - **Result:** Players choose input method that suits them.

3. **Adjustable Difficulty:**
   - **Settings:** Easy / Normal / Hard modes.
   - **Easy mode effects:**
     - Reduced enemy spawn rate.
     - Lower enemy bullet speed.
     - Increased player damage output.
     - Extra starting lives.
   - **Result:** Players with slower reaction times can enjoy the game.

4. **Toggle vs. Hold Options:**
   - **Shoot button:** Option to toggle auto-fire (press once to start shooting, press again to stop) instead of holding.
   - **Result:** Reduces strain from holding buttons.

5. **Pause Capability:**
   - **Single-player or lobby leader:** Can pause the game.
   - **Multiplayer compromise:** Safe zones where no enemies spawn, allowing players to rest between waves.
   - **Result:** Players can take breaks without penalty.

---

### 7.3 Audio/Hearing Accessibility

**Goal:** Ensure deaf/hard-of-hearing players receive critical information.

**Concrete Features:**

1. **Visual Indicators for Audio Cues:**
   - **Enemy spawn (off-screen):** Small arrow at screen edge pointing to spawn location + icon of enemy type.
   - **Boss roar/warning sound:** "WARNING" text + screen flash.
   - **Powerup pickup sound:** Brief text popup ("Force Collected!").
   - **Result:** Players don't miss critical events due to lack of audio.

2. **Subtitles for Dialogue:**
   - Any narrative text or boss taunts displayed as on-screen text.
   - **Result:** Story/context not lost for deaf players.

3. **Audio Balance Controls:**
   - **Separate sliders:** Music volume, SFX volume, Master volume.
   - **Result:** Players can mute music if distracting or boost SFX for clarity.

4. **Visual Hit Feedback:**
   - When player shoots enemy, **enemy flashes white** or shows damage number.
   - When player is hit, **screen flash + player ship blinks**.
   - **Result:** No reliance on hit sounds to confirm actions.

---

### 7.4 Cognitive Accessibility

**Goal:** Support players with cognitive disabilities, learning differences, or those who need clearer information presentation.

**Concrete Features:**

1. **Clear, Simple UI:**
   - **Minimalist HUD:** Only essential information (score, lives, weapon type).
   - **No information overload:** Avoid cluttered menus or rapid text scrolling.
   - **Result:** Players can focus on gameplay without distraction.

2. **Tutorial/Tooltips:**
   - **Optional tutorial mode:** On first play, overlay prompts explain controls and mechanics step-by-step.
   - **Example:** "Press ↑ to move up" appears on screen, waits for player to comply before proceeding.
   - **Tooltips (toggle in settings):** Small text boxes explain HUD elements ("Lives: Number of respawns remaining").
   - **Result:** New players learn without external documentation.

3. **Predictable Enemy Patterns:**
   - Enemies follow **learnable patterns**, not chaotic randomness.
   - **Example:** Snake enemies always weave at same frequency; players can memorize timing.
   - **Result:** Players improve through pattern recognition, reducing frustration.

4. **Adjustable Game Speed (Server Settings):**
   - **Settings:** 75% / 100% / 125% game speed.
   - **Effect:** Slows down or speeds up all entities (including player).
   - **Result:** Players who need more time to process can play at reduced speed.

---

## 8. Multiplayer & Networking Considerations

### 8.1 Player Count & Roles

- **Up to 4 players** can join a single game session.
- All players are **against** each other;
- Players share the same screen space (no split-screen; all players visible in the same scrolling viewport).

### 8.2 Server-Authoritative Model

**Design Principle:**
The server is the **single source of truth** for all gameplay-critical state.

**Server Responsibilities:**

- Compute all **collision detection** (player vs. enemy, projectile vs. target).
- Manage **entity spawning** (enemies, powerups, bosses).
- Track **health, lives, score** for all players.
- Broadcast **game state updates** to all clients (entity positions, health, events).

**Client Responsibilities:**

- Render game state received from server.
- Capture **local player input** (movement, shooting).
- Send input to server.
- **Predict local player movement** (for responsiveness) and reconcile with server updates.

**Why Server Authority?**

- Prevents cheating (clients can't fake score or invincibility).
- Ensures all players see consistent game state.
- Simplifies conflict resolution (server decides what happens in ambiguous cases).

### 8.3 Client-Side Prediction & Reconciliation

**Problem:** Network latency means input sent to server takes time to process and return. Without prediction, player movement feels sluggish.

**Solution:**

1. **Client-side prediction:** When player presses a movement key, client immediately updates local player position (before server acknowledges).
2. **Server validation:** Server receives input, computes authoritative position, sends update to client.
3. **Reconciliation:** Client compares predicted position with server's authoritative position. If mismatch (e.g., server detected collision), client smoothly corrects position.

**Result:** Player experiences &lt;50ms local responsiveness while maintaining server authority.

### 8.4 Visual Identification in Multiplayer

**Challenge:** With 4 players on screen, distinguishing "which ship is mine" is critical.

**Solutions:**

- **Color coding:** Each player assigned a distinct color (P1=Blue, P2=Green, P3=Yellow, P4=Red).
- **Sprite variation:** Slight differences in ship sprite (antenna, wing shape) per player.
- **Nameplate (optional):** Small text label above each player ship showing username or "P1", "P2", etc.
- **Camera focus (optional):** Slight zoom or highlight effect on local player's ship.

### 8.5 Friendly Fire Rules

**Default:** Friendly fire **OFF** (player bullets cannot damage other players).

**Ghosting:** Players are physically non-blocking to teammates (they can fly through each other) to prevent griefing in a competitive scrolling environment.

### 8.6 Disconnect Handling

**Scenario:** A player's connection drops mid-game.

**Server Behavior:**

1. Detect disconnect (no packets received for X seconds or explicit disconnect message).
2. Remove player's ship from game (despawn).
3. Broadcast to remaining players: "Player 2 disconnected."
4. Game continues for remaining players.

---

## 9. Playtest Checklist (Accessibility & Feel Verification)

Use this checklist during playtests to verify the game meets stated goals.

### 9.1 Responsiveness

- [ ] **No stuttering or frame drops:** Game maintains stable 60 FPS during intense action (many enemies + bullets on screen).
- [ ] **Immediate feedback on actions:** Pressing shoot button produces muzzle flash and sound immediately.

### 9.2 Visual Clarity

- [ ] **Player can distinguish own bullets from enemy bullets:** Colors are distinct (e.g., blue vs. red).
- [ ] **Players in multiplayer are visually distinct:** Each player has different color/sprite; easy to identify own ship.
- [ ] **HUD elements are readable:** Score, lives, weapon status visible without squinting; text does not overlap gameplay area.
- [ ] **Background does not obscure foreground:** Starfield is dim and does not distract from entities.
- [ ] **High contrast mode functional:** Toggling high contrast mode increases entity visibility noticeably.

### 9.3 Accessibility

- [ ] **Controls can be remapped:** Player opens settings, rebinds movement to WASD and shoot to F; changes persist client side.
- [ ] **High contrast mode improves visibility:** Colorblind tester confirms entities are distinguishable in high contrast mode.
- [ ] **Visual cues replace critical audio cues:** Deaf tester confirms boss warning is visible (text + screen flash) without audio.
- [ ] **Adjustable difficulty works:** Player on Easy mode survives first stage with minimal challenge; Normal mode provides balanced difficulty.

### 9.4 Gameplay Feel

- [ ] **Enemy patterns are learnable:** After 2-3 encounters, player can predict snake enemy movement and dodge effectively.
- [ ] **Force mechanics are intuitive:** Player understands attach/detach within one wave of experimentation.
- [ ] **Boss fights feel challenging but fair:** Boss encounter might require multiple attempts but players see improvement each try (not random difficulty).

### 9.5 Multiplayer & Networking

- [ ] **Players can join and leave without crashing server:** Connect 4 players, disconnect one mid-game; server continues smoothly.
- [ ] **No visual desync between clients:** Players in same game see enemies in same positions (within acceptable latency margin).
- [ ] **Collision detection feels fair:** Player feels hit registration is accurate (no "I wasn't touching that!" moments beyond lag explanation).

---

## Conclusion

This Game Design Document defines the core vision, mechanics, entity roles, and player experience goals for R-Type J.A.M.E.S. By staying faithful to the original R-Type's tight gameplay while embracing modern networked multiplayer and accessibility standards, the project aims to deliver a fun, challenging, and inclusive shmup experience.

**Key Takeaways:**

- **Server-authoritative multiplayer** ensures fair, consistent gameplay.
- **The Force mechanic** remains central to player strategy and identity.
- **Visual clarity and accessibility** are first-class design priorities, not afterthoughts.
- **Responsive controls** with client-side prediction provide the immediate feel essential to shmups.

Developers, designers, and testers should reference this document throughout implementation to ensure all features align with the stated design intent. Playtests using the provided checklist will validate whether the game achieves its accessibility and feel goals.

---

**Document Version:** 1.0
**Authors:** R-Type J.A.M.E.S. Team
**Last Updated:** December 1, 2025
