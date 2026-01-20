---
name: save-system
description: Save file structure and persistence system. Use when adding new persistent data, understanding save blocks, or debugging save-related issues.
---

# Save System

The GBA Pokemon games use a sector-based flash memory system with redundant save slots for corruption recovery. This skill covers the complete save architecture.

## Key Files

| File | Purpose |
|------|---------|
| `include/global.h` | SaveBlock1, SaveBlock2, SaveBlock3 struct definitions |
| `include/save.h` | Sector constants, save status codes, sector structure |
| `include/config/save.h` | Feature flags to free up save space |
| `src/save.c` | Flash read/write, checksum, slot management |
| `src/load_save.c` | RAM pointers, copy to/from save structs |
| `include/constants/flags.h` | FLAG_* definitions stored in SaveBlock1 |
| `include/constants/vars.h` | VAR_* definitions stored in SaveBlock1 |

## Physical Layout: 128 KB Flash

```
┌─────────────────────────────────────────────────────────────┐
│ Sectors 0-13:   Save Slot 1 (56 KB)                         │
│ Sectors 14-27:  Save Slot 2 (56 KB)  ← Alternates on save   │
│ Sector 28-29:   Hall of Fame (8 KB)                         │
│ Sector 30:      Trainer Hill (4 KB)                         │
│ Sector 31:      Recorded Battle (4 KB)                      │
└─────────────────────────────────────────────────────────────┘
```

**Two save slots** alternate on each save. If current slot corrupts mid-write, previous slot remains intact.

## Sector Structure (4096 bytes)

```c
struct SaveSector {
    u8 data[3968];              // 0x000-0xF7F: Primary save data
    u8 saveBlock3Chunk[116];    // 0xF80-0xFF3: SaveBlock3 fragment
    u16 id;                     // 0xFF4: Sector ID (0-13)
    u16 checksum;               // 0xFF6: Data checksum
    u32 signature;              // 0xFF8: Magic (0x08012025)
    u32 counter;                // 0xFFC: Increments each save
};
```

Each sector carries 116 bytes of SaveBlock3 in addition to its primary payload.

## Save Slot Contents (14 sectors)

| Sector | Content | Max Size |
|--------|---------|----------|
| 0 | SaveBlock2 | 3968 bytes |
| 1-4 | SaveBlock1 | 15872 bytes (4 × 3968) |
| 5-13 | PokemonStorage (PC boxes) | 35712 bytes (9 × 3968) |
| All | SaveBlock3 (distributed) | 1624 bytes (14 × 116) |

## The Four Save Blocks

### SaveBlock2 (Sector 0) - Player Identity

Global progress that rarely changes. Accessed via `gSaveBlock2Ptr`.

```c
struct SaveBlock2 {
    u8 playerName[8];           // Player name + terminator
    u8 playerGender;            // MALE (0) or FEMALE (1)
    u8 playerTrainerId[4];      // Visible ID (0-1) + Secret ID (2-3)
    u16 playTimeHours;
    u8 playTimeMinutes;
    u8 playTimeSeconds;
    u16 options;                // Text speed, sound, battle style, etc.
    struct Pokedex pokedex;     // Dex mode, special personalities
    u32 encryptionKey;          // XOR key for money/items
    struct BattleFrontier frontier;  // All Battle Frontier data
    u8 questData[];             // Quest menu state (expansion)
};
```

### SaveBlock1 (Sectors 1-4) - Game State

Main gameplay data. Accessed via `gSaveBlock1Ptr`.

```c
struct SaveBlock1 {
    // Position (0x00-0x2B)
    struct Coords16 pos;                // Current X/Y on map
    struct WarpData location;           // Current map ID
    struct WarpData continueGameWarp;   // Resume point
    struct WarpData lastHealLocation;   // White-out destination
    struct WarpData escapeWarp;         // Dig/Escape Rope target

    // Party (0x234-0x48F)
    u8 playerPartyCount;
    struct Pokemon playerParty[6];      // 100 bytes each = 600 total

    // Inventory (0x490-0x847)
    u32 money;                          // Encrypted
    u16 coins;                          // Game Corner
    struct ItemSlot pcItems[50];
    struct Bag bag;                     // All pockets

    // FLAGS AND VARS - Most commonly modified
    u8 flags[NUM_FLAG_BYTES];           // ~300 bytes of bit flags
    u16 vars[VARS_COUNT];               // ~512 bytes of variables

    // World State
    u32 gameStats[64];                  // Steps, battles, etc.
    struct BerryTree berryTrees[128];
    struct SecretBase secretBases[20];
    struct DayCare daycare;
    struct Roamer roamer[ROAMER_COUNT];

    // Pokedex flags
    u8 dexSeen[NUM_DEX_FLAG_BYTES];
    u8 dexCaught[NUM_DEX_FLAG_BYTES];
};
```

### SaveBlock3 (Distributed) - Expansion Features

For pokeemerald-expansion additions. Accessed via `gSaveBlock3Ptr`. **Max 1624 bytes.**

```c
struct SaveBlock3 {
#if OW_USE_FAKE_RTC
    struct SiiRtcInfo fakeRTC;
#endif
#if FNPC_ENABLE_NPC_FOLLOWERS
    struct NPCFollower NPCfollower;
#endif
#if OW_SHOW_ITEM_DESCRIPTIONS == OW_ITEM_DESCRIPTIONS_FIRST_TIME
    u8 itemFlags[ITEM_FLAGS_COUNT];
#endif
#if USE_DEXNAV_SEARCH_LEVELS == TRUE
    u8 dexNavSearchLevels[NUM_SPECIES];
#endif
    u8 dexNavChain;
    // Add new expansion features here
};
```

### PokemonStorage (Sectors 5-13) - PC Boxes

```c
struct PokemonStorage {
    u8 currentBox;
    struct BoxPokemon boxes[14][30];    // 14 boxes × 30 Pokemon
    u8 boxNames[14][9];
    u8 boxWallpapers[14];
};
```

Each BoxPokemon is 80 bytes (encrypted). Total: ~33 KB.

## Adding New Persistent Data

### Option 1: SaveBlock3 (Recommended for new features)

Best for expansion features. Conditionally compiled, doesn't affect vanilla saves.

```c
// In include/global.h, add to struct SaveBlock3:
struct MyFeatureData {
    u32 someValue;
    u8 someFlags[4];
};

struct SaveBlock3 {
    // ... existing fields ...
#if MY_FEATURE_ENABLED
    struct MyFeatureData myFeature;
#endif
};
```

**Limit:** Total SaveBlock3 must stay under 1624 bytes.

### Option 2: SaveBlock1

For core game features that should always exist.

```c
// In include/global.h, add to struct SaveBlock1:
struct SaveBlock1 {
    // ... existing fields ...
    struct MyNewData myData;  // Add at end
};
```

**Limit:** Total SaveBlock1 must stay under 15872 bytes (4 sectors).

### Option 3: SaveBlock2

For player identity/options that rarely change.

**Limit:** Total SaveBlock2 must stay under 3968 bytes (1 sector).

## Freeing Save Space

`include/config/save.h` provides flags to remove unused features:

```c
// SaveBlock1 - up to 2516 bytes
#define FREE_TRAINER_HILL           TRUE  // 28 bytes
#define FREE_MYSTERY_EVENT_BUFFERS  TRUE  // 1104 bytes
#define FREE_MATCH_CALL             TRUE  // 104 bytes
#define FREE_UNION_ROOM_CHAT        TRUE  // 212 bytes
#define FREE_ENIGMA_BERRY           TRUE  // 52 bytes
#define FREE_LINK_BATTLE_RECORDS    TRUE  // 88 bytes
#define FREE_MYSTERY_GIFT           TRUE  // 876 bytes

// SaveBlock2 - up to 1274 bytes
#define FREE_BATTLE_TOWER_E_READER  TRUE  // 188 bytes
#define FREE_POKEMON_JUMP           TRUE  // 16 bytes
#define FREE_RECORD_MIXING_HALL_RECORDS TRUE // 1032 bytes
```

## Flags and Vars

The most common way to persist simple data.

### Flags (1 bit each)

```c
// In include/constants/flags.h
#define FLAG_MY_EVENT_HAPPENED (FLAG_UNUSED_0x001 + 0)

// In scripts:
setflag FLAG_MY_EVENT_HAPPENED
clearflag FLAG_MY_EVENT_HAPPENED
goto_if_set FLAG_MY_EVENT_HAPPENED, Label

// In C:
FlagSet(FLAG_MY_EVENT_HAPPENED);
FlagClear(FLAG_MY_EVENT_HAPPENED);
if (FlagGet(FLAG_MY_EVENT_HAPPENED)) { ... }
```

### Vars (16 bits each)

```c
// In include/constants/vars.h
#define VAR_MY_COUNTER (VAR_UNUSED_0x001 + 0)

// In scripts:
setvar VAR_MY_COUNTER, 5
addvar VAR_MY_COUNTER, 1
goto_if_eq VAR_MY_COUNTER, 10, Label

// In C:
VarSet(VAR_MY_COUNTER, 5);
VarGet(VAR_MY_COUNTER);
```

## How Saving Works

1. **UpdateSaveAddresses()** - Points sector locations to RAM structs
2. **CopyPartyAndObjectsToSave()** - Copies working data to save structs
3. **WriteSaveSectorOrSlot()** - For each sector:
   - Copy data chunk + SaveBlock3 chunk
   - Calculate checksum
   - Write signature (0x08012025) and counter
   - Program to flash

Slot alternates: `gSaveCounter % 2` picks slot 0 (sectors 0-13) or slot 1 (sectors 14-27).

## How Loading Works

1. **GetSaveValidStatus()** - Check both slots, find highest valid counter
2. **CopySaveSlotData()** - For each sector:
   - Verify signature and checksum
   - Copy to RAM structs
3. **CopyPartyAndObjectsFromSave()** - Restore working data

## Encryption

Money and certain items are XOR'd with `gSaveBlock2Ptr->encryptionKey`:

```c
// Reading encrypted money
u32 GetMoney(void) {
    return gSaveBlock1Ptr->money ^ gSaveBlock2Ptr->encryptionKey;
}

// Writing encrypted money
void SetMoney(u32 amount) {
    gSaveBlock1Ptr->money = amount ^ gSaveBlock2Ptr->encryptionKey;
}
```

## Static Assertions

The build will fail if save blocks exceed their limits:

```c
// In src/save.c
STATIC_ASSERT(sizeof(struct SaveBlock2) <= SECTOR_DATA_SIZE, SaveBlock2FreeSpace);
STATIC_ASSERT(sizeof(struct SaveBlock1) <= SECTOR_DATA_SIZE * 4, SaveBlock1FreeSpace);
STATIC_ASSERT(sizeof(struct SaveBlock3) <= SAVE_BLOCK_3_CHUNK_SIZE * 14, SaveBlock3FreeSpace);
```

## Example: Adding Wally's Ralts Data

```c
// include/global.h - Add to SaveBlock3
struct WallyRaltsData {
    u32 personality;    // Nature, ability, gender, shiny
    u8 ivs[6];          // HP, Atk, Def, SpA, SpD, Spe
    bool8 caught;       // Tutorial completed?
    u8 padding;         // Alignment
}; // 12 bytes

struct SaveBlock3 {
    // ... existing ...
    struct WallyRaltsData wallyRalts;
};

// src/battle_setup.c - Capture data after tutorial
void StartWallyTutorialBattle(void)
{
    CreateMon(&gEnemyParty[0], SPECIES_RALTS, 5, ...);

    // Save the Ralts data
    gSaveBlock3Ptr->wallyRalts.personality = GetMonData(&gEnemyParty[0], MON_DATA_PERSONALITY);
    gSaveBlock3Ptr->wallyRalts.ivs[0] = GetMonData(&gEnemyParty[0], MON_DATA_HP_IV);
    // ... etc
    gSaveBlock3Ptr->wallyRalts.caught = TRUE;

    // Continue with battle...
}
```

## Common Gotchas

### 1. Build fails with "SaveBlock*FreeSpace"
You exceeded the size limit. Free space using config flags or move data elsewhere.

### 2. Data doesn't persist
- Ensure you're modifying `gSaveBlock*Ptr`, not a local copy
- Check that the game actually saves (not just soft reset)

### 3. Corrupted saves after adding fields
- Don't reorder existing fields
- Add new fields at the END of structs
- Consider save versioning for released hacks

### 4. SaveBlock3 not saving
SaveBlock3 is distributed across sectors. If you add too much, it silently fails. Check the static assertion.

## Useful Functions

```c
// Trigger a save
TrySavingData(SAVE_NORMAL);

// Check save status
if (gSaveFileStatus == SAVE_STATUS_OK) { ... }

// Copy current state to save structs (before manual sector write)
CopyPartyAndObjectsToSave();

// Restore from save structs (after load)
CopyPartyAndObjectsFromSave();
```

## RAM Pointers

```c
extern struct SaveBlock1 *gSaveBlock1Ptr;  // Main game state
extern struct SaveBlock2 *gSaveBlock2Ptr;  // Player identity
extern struct SaveBlock3 *gSaveBlock3Ptr;  // Expansion features
extern struct PokemonStorage *gPokemonStoragePtr;  // PC boxes
```

These are set during game init and point to EWRAM buffers.
