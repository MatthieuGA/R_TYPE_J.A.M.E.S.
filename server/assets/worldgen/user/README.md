# User WorldGen Frames

Place your custom `.wgf.json` files in this directory.

## Quick Start

1. Create a new file with the `.wgf.json` extension (e.g., `my_frame.wgf.json`)
2. Generate a unique UUID using https://www.uuidgenerator.net/
3. Follow the schema below
4. Restart the server to load your frame

## Minimal Example

```json
{
  "uuid": "your-unique-uuid-here",
  "name": "My Custom Frame",
  "difficulty": 5.0,
  "obstacles": [
    {
      "type": "static",
      "sprite": "images/obstacle.png",
      "position": { "x": 400, "y": 300 },
      "size": { "width": 64, "height": 64 }
    }
  ]
}
```

## Schema Reference

See the core frames in `../core/` for complete examples.

### Required Fields

- `uuid`: Unique UUIDv4 identifier
- `name`: Human-readable name
- `difficulty`: Float from 0.0 (trivial) to 10.0 (extreme)
- `obstacles`: Array of obstacle definitions

### Optional Fields

- `description`: Frame description
- `width`: Frame width in game units (default: 800)
- `tags`: Array of category strings
- `spawn_rules`: Selection rules (see below)
- `background`: Background layer configuration

### Obstacle Types

- `static`: Indestructible, blocks movement
- `destructible`: Can be destroyed by weapons
- `hazard`: Damages on contact
- `decoration`: Visual only, no collision

## Rules

- UUIDs must be unique - duplicates will be skipped
- User frames cannot override core frames with the same UUID
- Check server logs for load errors
