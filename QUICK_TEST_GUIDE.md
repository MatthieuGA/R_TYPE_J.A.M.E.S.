# WorldGen Manual Testing - Quick Start Guide

## What You'll Test

The WorldGen system has **9 manual tests** organized from easiest to hardest:

| # | Test Name | What It Does | Difficulty |
|---|-----------|--------------|------------|
| 1 | Load WGF Files | Shows loaded content files | ⭐ |
| 2 | Deterministic RNG | Verifies same seed = same output | ⭐⭐ |
| 3 | Endless Mode | Generates infinite random levels | ⭐⭐ |
| 4 | Determinism Across Managers | Two managers with same seed match | ⭐⭐⭐ |
| 5 | Reset Functionality | Reset regenerates same sequence | ⭐⭐⭐ |
| 6 | Save/Restore State | Save and resume from exact position | ⭐⭐⭐⭐ |
| 7 | Fixed Level Mode | Predefined level sequences | ⭐⭐⭐ |
| 8 | Spawn Events | Event generation for entity spawning | ⭐⭐⭐⭐ |
| 9 | Difficulty Scaling | Adaptive frame selection | ⭐⭐⭐⭐⭐ |

## How to Run Tests

### Interactive Mode (Recommended)

```bash
cd build/server
./worldgen_manual_test
```

Then select a test number (1-9) or:
- `0` - Run all tests
- `q` - Quit

### Automated Run (All Tests)

```bash
./run_worldgen_tests.sh
```

This runs all 9 tests automatically and exits.

## Test Order Recommendation

**First Time Testing:**
1. Run test `1` - Load WGF Files (verify files load)
2. Run test `2` - Deterministic RNG (verify randomness works)
3. Run test `3` - Endless Mode (see worldgen in action)
4. Run test `0` - All Tests (comprehensive verification)

**Quick Verification:**
- Run test `0` (takes ~15 seconds, validates everything)

## What Success Looks Like

✅ All tests show:
- Green checkmarks `✓`
- Success messages
- No red crosses `✗`
- No error messages

Example good output:
```
✓ Determinism verified!
✓ Perfect determinism over 20 frames!
✓ Reset works! Sequences match perfectly.
✓ Save/Restore works! Sequences match.
```

## What Failure Looks Like

❌ Failed tests show:
- Red crosses `✗`
- "Failed" messages
- Mismatched values

Example bad output:
```
✗ Determinism failed!
Frame 5: ✗ Kamifish Frame vs Mermaid Frame
```

## Quick Test Descriptions

### TEST 1: Load WGF Files (30 seconds)
**What**: Loads and displays all WGF files  
**Look for**: File count, WGF names, no parse errors  
**Pass**: Shows 2+ WGF files with details

### TEST 2: Deterministic RNG (30 seconds)
**What**: Generates random numbers with same seed twice  
**Look for**: All 10 pairs match exactly  
**Pass**: "Determinism verified!" message

### TEST 3: Endless Mode (30 seconds)
**What**: Generates 5 random frames  
**Look for**: Frame names, difficulties, widths  
**Pass**: Shows 5 frames with varying content

### TEST 4: Determinism Across Managers (1 minute)
**What**: Two managers with same seed generate 20 frames  
**Look for**: All 20 frames match between managers  
**Pass**: "Perfect determinism over 20 frames!"

### TEST 5: Reset Functionality (1 minute)
**What**: Generate 10 frames, reset, generate 10 more  
**Look for**: Both sequences identical  
**Pass**: "Sequences match perfectly"

### TEST 6: Save/Restore State (1 minute)
**What**: Save at frame 5, advance, restore, verify continuation  
**Look for**: Frames after restore match frames after save  
**Pass**: "Save/Restore works! Sequences match"

### TEST 7: Fixed Level Mode (30 seconds)
**What**: Play through predefined level sequence  
**Look for**: Frames in exact order, level completes  
**Pass**: "Level completed!"

### TEST 8: Spawn Events (1 minute)
**What**: Monitor event generation while advancing frames  
**Look for**: Frame start/end events, obstacle events  
**Pass**: Shows event counts, "Spawn events are being generated!"

### TEST 9: Difficulty Scaling (2 minutes)
**What**: Test frame selection at 5 difficulty levels  
**Look for**: Average difficulty trends upward with target  
**Pass**: Higher targets → higher average difficulties

## Troubleshooting

### "Failed to load WGF files!"
**Problem**: Not running from correct directory  
**Solution**:
```bash
cd build/server
./worldgen_manual_test
```

### Program shows errors
**Problem**: Missing WGF files  
**Check**: `assets/worldgen/core/` should have `.wgf.json` files

### Tests keep failing
**Problem**: Actual bug in implementation  
**Action**: Report which test fails and what the error is

## File Locations

```
build/server/
├── worldgen_manual_test      ← Run this
├── assets/worldgen/
│   ├── core/                 ← WGF files
│   │   ├── kamifish.wgf.json
│   │   └── mermaid.wgf.json
│   └── levels/               ← Level files
│       └── tutorial.level.json
```

## Next Steps After Testing

Once all manual tests pass:

1. **Run Unit Tests**: `cd build && ctest`
   - Validates 460 automated tests including 32 worldgen-specific tests

2. **Try Custom Seeds**:
   ```cpp
   manager.InitializeEndless(YOUR_SEED, 5.0f);
   ```

3. **Create Custom Content**:
   - Add WGF files to `assets/worldgen/user/`
   - Create levels in `assets/worldgen/levels/`

4. **Integrate into Game**:
   - Connect `WorldGenSystem` to server game loop
   - Implement obstacle spawning from events

## Color Legend

The test program uses colors for quick visual feedback:

- 🟢 **Green** (`✓`) - Success, all good
- 🔴 **Red** (`✗`) - Failure, needs attention
- 🟡 **Yellow** (`⚠`) - Warning, usually OK
- 🔵 **Blue** (`ℹ`) - Info, FYI messages
- **Cyan** - UUIDs and identifiers
- **Magenta** - Frame start events
- **Bold** - Section headers

## Expected Runtime

- Single test: 30 seconds - 2 minutes
- All tests (option 0): ~15 seconds total
- Interactive mode: As long as you want

## Tips

1. **First time?** Run tests 1, 2, 3 individually to understand each
2. **In a hurry?** Run test 0 (all tests)
3. **Found a bug?** Note which test number fails
4. **Want details?** See `MANUAL_TESTING_GUIDE.md` for in-depth explanations

## Success Criteria

✅ All 9 tests pass with green checkmarks  
✅ No red error messages appear  
✅ Sequences match where expected  
✅ Events are generated  
✅ Files load correctly  

If all the above are true → **WorldGen system is working perfectly!**

---

**Ready to test?** Run `./worldgen_manual_test` and select option `0`!
