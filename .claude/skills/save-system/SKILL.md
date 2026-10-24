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
    // Upstream's struct ends here (0xF2C). Ours, appended:
    u8 questData[QUEST_FLAGS_COUNT * QUEST_STATES];  // Quest menu state
    u8 subQuests[SUB_FLAGS_COUNT];
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
#if APRICORN_TREE_COUNT > 0
    u8 apricornTrees[NUM_APRICORN_TREE_BYTES];
#endif
    struct WallyRaltsData wallyRalts;   // ours: last, so it moves when a config-gated block above is enabled
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
// In include/constants/flags.h: rename the next free flag from the top of
// 0x493-0x4EF (see "ID registry" below) and add a row to the registry
#define FLAG_MY_EVENT_HAPPENED FLAG_UNUSED_0x4EF

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
// In include/constants/vars.h: the next free var from 0x40FF down
#define VAR_MY_COUNTER VAR_UNUSED_0x40FF

// In scripts:
setvar VAR_MY_COUNTER, 5
addvar VAR_MY_COUNTER, 1
goto_if_eq VAR_MY_COUNTER, 10, Label

// In C:
VarSet(VAR_MY_COUNTER, 5);
VarGet(VAR_MY_COUNTER);
```

## ID registry

Every ID we claim from a range upstream also allocates from. This table is the one place to look before claiming a new one, and the checklist for the ID-collision audit of every upstream merge (`upstream-merge` skill). Add a row in the same commit that claims the ID. Build-time checks on these live in `src/verdant_asserts.c`.

| Kind | ID | Name | Defined in |
|------|----|------|------------|
| Flag | 0x020 | `DN_FLAG_SEARCHING` | `include/config/verdant.h` (as `FLAG_UNUSED_0x020`) |
| Flag | 0x021 | `I_EXP_SHARE_FLAG` (Exp. Share All on) | `include/config/verdant.h` |
| Flag | 0x022 | `DN_FLAG_DETECTOR_MODE` | `include/config/verdant.h` |
| Flag | 0x023 | `DN_FLAG_DEXNAV_GET` (also `setflag FLAG_UNUSED_0x023` in Birch's lab) | `include/config/verdant.h` |
| Flag | 0x02A | `B_FLAG_FOLLOWERS_DISABLED` | `include/config/verdant.h` |
| Flag | 0x1AA | `FLAG_RECEIVED_MACHO_BRACE_OLDALE` | `include/constants/flags.h` |
| Flag | 0x1AB | `FLAG_RECEIVED_WIMPY_BRACE_OLDALE` | `include/constants/flags.h` |
| Flag | 0x2D5 | `FLAG_HIDE_ROUTE104_DEVON_RESEARCHER` (upstream's `FLAG_HIDE_PETALBURG_WOODS_AQUA_GRUNT`, renamed) | `include/constants/flags.h` |
| Flag | 0x2D9 | `FLAG_HIDE_ROUTE104_AQUA_GRUNT` | `include/constants/flags.h` |
| Flag | 0x4EE | `FLAG_STAT_EDITOR_IV_UNLOCK` (story Stat Editor edits IVs and natures) | `include/constants/flags.h` |
| Flag | 0x4EF | `FLAG_STAT_EDITOR_NATURE_UNLOCK` (story Stat Editor edits natures) | `include/constants/flags.h` |
| Flag | `SYSTEM_FLAGS` + 0x3 (0x863) | `FLAG_SYS_QUEST_MENU_GET` | `include/constants/flags.h` |
| Var | 0x404E | `DN_VAR_SPECIES` | `include/config/verdant.h` |
| Var | 0x4083 | `DN_VAR_STEP_COUNTER` | `include/config/verdant.h` |
| Var value | `VAR_STARTER_MON` 0 / 1 | player picked Axew / Deino (2 is dead) | `CLAUDE.md` → Custom Starters |
| Script opcode | 0xf0-0xf2 | `questmenu`, `subquestmenu`, `returnqueststate` | `data/script_cmd_table.inc` |
| Script opcode | 0xe7-0xef | reserved `ScrCmd_nop` placeholders: upstream's next opcodes replace them in order | `data/script_cmd_table.inc` |
| Item | 995-999 | `ITEM_STAT_EDITOR`, `ITEM_QOL_STAT_EDITOR`, `ITEM_WIMPY_BRACE`, `ITEM_EXP_SHARE_ALL`, `ITEM_INFINITE_CANDY` | `include/constants/items.h` |
| Item | 874-994 | empty slots, filled with one "????????" range entry | `src/data/items.h` |
| Species | 1550, 1551 | `SPECIES_MAGIKARP_AEVIAN`, `SPECIES_GYARADOS_AEVIAN` (upstream's Z-A megas shifted to 1552-1574) | `include/constants/species.h` |
| Berry tree | 90 | `BERRY_TREE_OLDALE_RUINS_1` | `include/constants/berry.h` |
| Battle-type bit | `1 << 30` | `BATTLE_TYPE_SCRIPTED` (upstream's `BATTLE_TYPE_30`; bit 31 is taken, so it's the last free bit) | `include/constants/battle.h` |
| Map | group 0, nums 57, 58 | `MAP_SERENE_POND`, `MAP_OLDALE_RUINS` (appended to `gMapGroup_TownsAndRoutes`) | `data/maps/map_groups.json` |
| Map group | 34, num 0 | `gMapGroup_IndoorOldaleRuins`: `MAP_OLDALE_RUINS_BERRY_FARMERS_HOUSE` | `data/maps/map_groups.json` |
| Layout | 442-444 | `LAYOUT_SERENE_POND`, `LAYOUT_OLDALE_RUINS`, `LAYOUT_OLDALE_RUINS_BERRY_FARMERS_HOUSE` | `data/layouts/layouts.json` |
| Enum entry | last before `_COUNT` | `STRINGID_CHALLENGERLOSTTOENEMYTRAINER`, `TRAINER_SLIDE_BATTLE_WON` (not saved; renumbering is harmless) | `battle_string_ids.h`, `trainer_slide.h` |
| Save field | SaveBlock2 0xF2C, 20 bytes | `questData` (upstream's struct ends at 0xF2C) | `include/global.h` |
| Save field | SaveBlock2 0xF40, 4 bytes | `subQuests` (SaveBlock2 is 3908 bytes) | `include/global.h` |
| Save field | SaveBlock3 0x10, 12 bytes | `wallyRalts` (`struct WallyRaltsData`), last member, after the config-gated blocks: enabling one moves it (SaveBlock3 is 28 bytes with our configs) | `include/global.h` |

Map, layout and save offsets come from the build: `include/constants/map_groups.h` and `layouts.h` are generated, and `tools/save_convert/compare_builds.py` measures the save layout.

### Allocation rules

- Fixed-size ranges are allocated from the top down, so upstream (which fills from the bottom) and we meet last: items from 999 down, berry trees from 127 down, opcodes from 0xf0 up past upstream's reserved block. Every low-first claim we made was hit by upstream (opcode 0xe5, battle-type bit 28, `MULTI` 115, species 1550); the high-first ones never were.
- New flags: 0x4EF down to 0x493 (`FLAG_UNUSED_*`). New vars: 0x40FF down. Don't take 0x264 or 0x404E for anything new: upstream's config comments suggest them as examples (we already hold 0x404E).
- Claim a free ID through upstream's `FLAG_UNUSED_*` / `VAR_UNUSED_*` name, so the build breaks if upstream takes it.
- New custom menus use `dynmultichoice`, not a `MULTI_*` ID. New script commands are `callnative`, not opcodes.
- Enum entries that aren't saved (string IDs, trainer slides) can be appended; prefer keeping them out of upstream's enums entirely.
- IDs are frozen at the first public release; until then a planned save break may move them (merge hygiene item 5).

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
    u8 ivs[NUM_STATS];  // HP, Atk, Def, Spe, SpA, SpD
    u8 hiddenNature;    // For mint system
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
