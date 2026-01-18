## Objective <!-- Mandatory -->

Implement the foundational system responsible for deterministic world generation based on a seed.
This system must be able to, given a seed, produce a reproducible sequence of obstacle “slots” or “spawn events” from which the game will later select actual obstacles.

Endless mode will generate or select a seed at runtime; fixed levels will supply a predefined seed.

This issue does not handle loading config files or selecting actual obstacle assets — that is part of #125 .

Keep in mind that enemies will be spawned in a similar fashion, so keeping the code modular, layered and clean is a good first step toward this part (and future proofing in general)

We will call a single part of a whole environment a worldgen frame, and the seed must simply pick a worldgen frame in a deterministic way, no matter how much worldgen frame are available (having a standard lib of WGF, and then all the user ones, available only through endless mode and map editor ?)



## Context <!-- Optional -->

The server needs a predictable and deterministic way to generate obstacle layouts across all clients, ensuring that every client sees the same world with no desync.
To achieve this, the worldgen logic must rely on a PRNG seeded once at level start, and all subsequent obstacle spawn decisions must be derived solely from that seed.

The output of this system will later integrate with the config file system (separate issue), which will dictate what obstacle types correspond to generated spawn “slots” or probability tables.

This issue focuses only on the seed-driven logic and the system’s responsibilities within the ECS or game server.

if it wasn't clear, there is two benefits for going seed based : determinist code will allow the creation of levels very easily (simple as generating a few seeds and having a pre-defined ending that happens after x worldgen frame) and both client and server will have the same map without ever communicating anything other than the seed !



## Steps <!-- Mandatory -->

- [ ] Define the responsibilities and boundaries of the worldgen logic system
(pure logic? ECS system? server-side service?).
- [ ]  Implement a deterministic PRNG wrapper (or choose an existing one) suitable for reproducible worldgen, adding new WGF (by the user for example) SHOULD NOT change anything.
- [ ]  Implement a WorldGen module (or ECS system) that:
    - Accepts a seed at initialization.
    - Generates a timeline or sequence of obstacle spawn markers (example: “spawn slot every X units,” “choose spawn category,” etc.).
    - Produces deterministic outputs for the same seed.
- [ ] Expose a minimal API for the server to request:
    - “next spawn event”
    - “reset with new seed”
    - “advance worldgen state”
- [ ] Ensure worldgen does not depend on frame rate or tick-rate (avoid nondeterministic input).
- [ ]  Add unit tests validating:
    - Same seed → identical generation results.
    - Different seeds → different generation.
    - Long runs (stress test) remain deterministic.
- [ ]  Integrate the system into the game loop:
    - Server ticks should call the worldgen at appropriate intervals.
    - Generated spawn events should be passed to the part of the system that instantiates obstacles (integration done in later issues).




## Acceptance Criteria <!-- Mandatory -->

- [ ]   A deterministic worldgen logic system exists and can be initialized with a seed.
- [ ]   Given a seed, worldgen outputs the same sequence every run (verified via tests).
- [ ]   Worldgen produces spawn events independently of client state, frame rate, or other nondeterministic factors.
- [ ]   The server can request spawn events through a clean API (no direct coupling to obstacle assets).
- [ ]   The design is documented to prevent misuse (e.g., modifying seed mid-game, relying on nondeterministic inputs).
- [ ]   No dependency yet on config file structure (handled in separate issue).

## Relevant Links <!-- Optional -->

PRNG determinism overview (LCG, XorShift, PCG, etc.)
