## Objective <!-- Mandatory -->

Implement a data-driven configuration system that describes how world generation works, based on WorldGen Frames (WGF) — reusable building blocks that represent one segment, obstacle layout, or environment slice.

WorldGen must be:
- Deterministic for any seed
- Extendable so users can drop new WGF files into a folder
- Stable so adding new files never changes existing seeds/levels
- Configurable via JSON (or similar) without compilation
- Compatible with future level editing tools

This includes defining the full data model for WGF files, seed metadata, difficulty weighting, and the integration architecture (ECS system vs external manager).
Enemy generation is out of scope !

## Context <!-- Optional -->

World generation currently has no implementation. This issue introduces a foundational structure for:
- obstacle definitions
- reusable environmental frames (WGF)
- difficulty scaling
- deterministic selection
- seed-based level generation
- user-provided/drag-and-drop content
- future editor support

Design constraints to consider:

1. Each WGF is a standalone JSON file

Located in a folder such as:
`/assets/worldgen/core/`

Additional user files can be in:
`/assets/worldgen/user/`

2. WGFs must have a stable UUID

This ensures old seeds remain valid even if users add new files (very important)

3. A seed must embed its own metadata:

- integer seed value
- difficulty target
- endless / fixed level flag
- the exact list of WGF UUIDs available at generation time
-> this ensures determinism forever -> this also is way easier that dealing with thsi on the WGF side

4. Each WGF needs metadata

- UUID
- name
- difficulty rating (int, float...)
- biome/theme/category tags ?
- asset references
- collision/obstacle layout data
- optional spawn rules (min distance, max frequency, etc.)

5. Level editor compatibility

A level file will simply list WGF UUIDs in order:
```
{
    "frames": [
        "uuid-1",
        "uuid-2",
        "uuid-3"
    ]
}
```
6. Architecture considerations

config should be loaded once on server start

ECS should not store JSON blobs, only typed data

a dedicated WorldGenManager should serve data to ECS

WGF files must be validated, with fallback for missing fields

## Steps <!-- Mandatory -->
 Define the high-level structure of the config file (keys, data schema, obstacle definitions, spawn rules, seed handling, etc.)

### Config File Structure

- [] Define the WGF schema (keys, types, required vs optional fields) 
- []Define the seed metadata schema
- [] Define the worldgen configuration root file (if needed)
### Library & Parsing
- [] Choose serialization/parsing library (nlohmann JSON, rapidjson, toml++, etc.)
### Modules / Architecture
- [] Create a WorldGenConfigLoader module to:
    - scan both core and user WGF directories
    - load & validate each file
    - assign default values
    - maintain UUID → WGF mapping
    - compute a deterministic list for seeds
- [] Create internal data structures:
    - WGFDefinition
    - SeedMetadata
    - WorldGenConfig
    - WorldGenManager (runtime access & selection logic)
### Deterministic Worldgen Logic
- [] Implement a WGF-selection algorithm that uses:
    - seed value
    - difficulty weight
    - WGF difficulty metadata
    - allowed WGF UUID list embedded in the seed
- [] Implement endless mode rules:
    - infinite selection
    - difficulty scaling per distance/time
### Integration into the ECS
- [] Decide the integration strategy:
    - ECS system calls WorldGenManager each tick
    - OR WorldGenManager generates frames externally, ECS only spawns entities
- [] Document the chosen architecture
### Error Handling
- [] Add fallback behavior if:
    - user folder is empty
    - WGF file is corrupt
    - required fields missing
    - duplicate UUIDs
- [] Add clear logging for:
    - load success
    - invalid entries
    - skipped files
### User Mod Support
- [] Implement scanning of two directory layers: core + user
- [] Ensure user WGFs never override core WGFs unless explicitly allowed
- [] Ensure seeds stay fully deterministic even if new WGFs appear later
#### Documentation
- [] Document all schemas
- [] Document user mod folder structure
- [] Document seed behavior
- [] Document stability/determinism rules


## Acceptance Criteria <!-- Mandatory -->
- [] A working, validated configuration system loads at server startup
- [] WGF files are discoverable, parsed, and stored with correct metadata
- [] Seeds embed their WGF UUID list, difficulty, and endless flag
- [] Adding new WGF files does not impact old seeds
- [] World generation uses the configuration instead of hardcoded values
- [] Error handling prevents crashes even with broken WGF files
- [] Logs clearly report load success/failure and skipped fields
- [] Architecture choice (manager vs ECS integration) is documented
- [] System supports future level editor (ordered list of WGF UUIDs)

## Relevant Links <!-- Optional -->

nlohmann JSON

rapidjson

toml++

UUID generation libraries

Examples from moddable games (RimWorld my beloved, Factorio, Minecraft datapacks, Starsector also my beloved)
