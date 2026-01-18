# R-Type J.A.M.E.S. Map Editor

A web-based visual tool for creating custom levels by combining World Generation Frames (WGFs).

## 🚀 Quick Start

1. **Open the editor**: Simply open `index.html` in your web browser (Chrome/Firefox/Edge recommended)

2. **Load WGF files**:
   - Click the file input in the "Load WGF Files" section
   - Navigate to `server/assets/worldgen/core/`
   - Select the folder (not individual files)

3. **Build your level**:
   - Browse frames in the library (left panel)
   - Click a frame to add it to the timeline, or drag & drop
   - Reorder frames by dragging within the timeline
   - Click the × button to remove a frame

4. **Configure level settings**:
   - **Level Name**: Display name in the server menu
   - **Author**: Your name
   - **Description**: Optional level description
   - **Target Difficulty**: Affects enemy/powerup spawning (0-10)
   - **Continue with infinite generation**: If checked, random frames spawn after your sequence ends

5. **Export**:
   - Click "Export as .level.json"
   - The editor will now try to save directly into the server assets folder if a local save server is running (recommended).
   - If the save server is not running, the editor will download the file locally instead.
   - To enable direct-saving, run the provided local save server before exporting:

```bash
node tools/map_editor/save_server.js
```

   - Restart the server to see your level in the menu!

## 📁 File Structure

```
tools/map_editor/
├── index.html    # Main editor page
├── styles.css    # Visual styling
├── editor.js     # Editor logic
└── README.md     # This file
```

## ✨ Features

- **Visual Frame Library**: Browse all WGF frames with difficulty, tags, and content info
- **Filtering**: Search by name, filter by difficulty range, or filter by tag
- **Drag & Drop**: Intuitive level building by dragging frames
- **Frame Details**: See enemies, obstacles, and metadata for each frame
- **Live Preview**: JSON preview updates in real-time
- **Import/Export**: Save and load level files
- **Responsive**: Works on different screen sizes

## 🎮 Level Format

The editor creates `.level.json` files with this structure:

```json
{
  "uuid": "generated-uuid-here",
  "name": "My Custom Level",
  "author": "Player Name",
  "description": "A challenging adventure...",
  "target_difficulty": 5.0,
  "is_endless": false,
  "frames": [
    "wgf-uuid-1",
    "wgf-uuid-2",
    "wgf-uuid-3"
  ]
}
```

### Fields

| Field | Type | Description |
|-------|------|-------------|
| `uuid` | string | Unique identifier (auto-generated) |
| `name` | string | Level display name |
| `author` | string | Level creator |
| `description` | string | Optional description |
| `target_difficulty` | float | Difficulty rating (0-10) |
| `is_endless` | boolean | If true, infinite generation continues after frames |
| `frames` | array | Ordered list of WGF UUIDs |

## 🔧 Tips

- **Pacing**: Alternate between high-intensity and breather frames
- **Difficulty Curve**: Start easy, gradually increase difficulty
- **Powerups**: Place powerup frames before difficult sections
- **Testing**: Keep levels short initially to test the flow
- **Tags**: Use the tag filter to find frames with specific content

## 🐛 Troubleshooting

### "No .wgf.json files found"
Make sure you selected the `core/` folder, not individual files.

### Frames missing after import
The imported level references WGF UUIDs that aren't in your loaded library. Make sure you load the same WGF files that were used to create the level.

### Level doesn't appear in server
1. Make sure the file is in `server/assets/worldgen/levels/`
2. File must end with `.level.json`
3. Restart the server after adding new levels

## 📝 Creating Custom WGF Frames

If you want to create new frames (not just combine existing ones), see the WGF documentation in `server/assets/worldgen/user/README.md`.

## 🤝 Contributing

Improvements to the map editor are welcome! Key areas:
- Visual frame preview (showing enemy/obstacle positions)
- Frame validation (check if UUIDs exist)
- Undo/Redo functionality
- Level testing integration
