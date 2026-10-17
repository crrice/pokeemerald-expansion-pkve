---
name: notebook-config
description: Player bedroom notebook system for game configuration options. Use when adding new player options, modifying the notebook menu, or working with the Exp Share mode selection.
---

# Notebook Configuration System

The notebook in the player's bedroom provides configuration options that persist throughout the game. Players can return anytime to change settings.

## Key Files

| File | Purpose |
|------|---------|
| `data/scripts/players_house.inc` | Notebook script (`PlayersHouse_2F_EventScript_VerdantNotebook`) and option handlers |
| `data/maps/LittlerootTown_{Brendans,Mays}House_2F/map.json` | The notebook bg_event in each house points at our script |
| `include/config/verdant.h` | Config flags we assign (e.g. `I_EXP_SHARE_FLAG`) |
| `tools/replay/scenarios/notebook_exp_share.replay` | Replay test: each Exp. Share choice in both houses |

## Current Options

### Exp Share Mode
Lets players choose between:
- **Single** - Held item, gives exp to holder only
- **All** - Key item, toggleable, all party Pokemon gain exp

A `dynmultichoice` menu; swaps between `ITEM_EXP_SHARE` and `ITEM_EXP_SHARE_ALL` and clears/sets `I_EXP_SHARE_FLAG`. A new game starts with Exp. Share All, on (`src/new_game.c`).

## Adding a New Notebook Option

### Step 1: Menu
Every custom menu uses `dynmultichoice`: the choices are text labels in the script, so there are no `MULTI_*` constants or `sMultichoiceLists` entries in upstream's files (those caused merge conflicts and an ID collision). B returns 127 (`MULTI_B_PRESSED`) unless `ignoreBPress` is TRUE.

### Step 2: Add Flag or Var (if needed)
For persistent settings, take the next free flag or var as the `save-system` skill's ID registry says (top-down, through upstream's `FLAG_UNUSED_*` / `VAR_UNUSED_*` name) and add a row there:
```c
// In include/constants/flags.h
#define FLAG_MY_SETTING    FLAG_UNUSED_0x4EF

// Or in include/constants/vars.h for multiple states
#define VAR_MY_SETTING     VAR_UNUSED_0x40FF
```

### Step 3: Add Script Handler
In `data/scripts/players_house.inc`, add to the notebook script:
```
PlayersHouse_2F_EventScript_VerdantNotebook::
    lockall
    @ ... existing options ...
    msgbox PlayersHouse_2F_Text_MyOptionQuestion, MSGBOX_DEFAULT
    dynmultichoice 14, 6, FALSE, 2, 0, DYN_MULTICHOICE_CB_NONE, PlayersHouse_2F_Text_MyOptionA, PlayersHouse_2F_Text_MyOptionB
    switch VAR_RESULT
    case 0, PlayersHouse_2F_EventScript_MyOptionA
    case 1, PlayersHouse_2F_EventScript_MyOptionB
    case 127, PlayersHouse_2F_EventScript_MyOptionCancel
    end

PlayersHouse_2F_EventScript_MyOptionA::
    setflag FLAG_MY_SETTING
    msgbox PlayersHouse_2F_Text_MyOptionAConfirm, MSGBOX_DEFAULT
    releaseall
    end
```

### Step 4: Add Text Strings
```
PlayersHouse_2F_Text_MyOptionA:
    .string "Choice A$"

PlayersHouse_2F_Text_MyOptionB:
    .string "Choice B$"

PlayersHouse_2F_Text_MyOptionQuestion:
    .string "Description of the option.$"

PlayersHouse_2F_Text_MyOptionAConfirm:
    .string "Setting changed to A.$"
```

## Utility: RemoveItemFromEverywhere

For options that swap items (like Exp Share), use:
```
setvar VAR_0x8004, ITEM_OLD_ITEM
callnative Script_RemoveItemFromEverywhere
giveitem ITEM_NEW_ITEM
```

This removes the item from:
- Player's bag (all pockets)
- PC storage
- Held by party Pokemon
- Held by Pokemon in storage boxes
- Held by Pokemon in daycare

Defined in `src/item.c` with script wrapper `Script_RemoveItemFromEverywhere`.

## Design Notes

- Notebook is in both Brendan's and May's house (shared script). Upstream's own `PlayersHouse_2F_EventScript_Notebook` and texts in `LittlerootTown_BrendansHouse_2F/scripts.inc` stay untouched (unused script, reused texts), so upstream edits there merge cleanly.
- Options should work regardless of game progress
- Use flags for on/off settings, vars for multiple states
- Cancel (case 127) should exit cleanly without changes
- Keep text concise for GBA screen width
