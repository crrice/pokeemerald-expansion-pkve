---
name: notebook-config
description: Player bedroom notebook system for game configuration options. Use when adding new player options, modifying the notebook menu, or working with the Exp Share mode selection.
---

# Notebook Configuration System

The notebook in the player's bedroom provides configuration options that persist throughout the game. Players can return anytime to change settings.

## Key Files

| File | Purpose |
|------|---------|
| `data/scripts/players_house.inc` | Notebook script and option handlers |
| `src/data/script_menu.h` | Multichoice menu definitions |
| `include/constants/script_menu.h` | Multichoice menu ID constants |
| `include/config/item.h` | Feature flags (e.g., I_EXP_SHARE_FLAG) |

## Current Options

### Exp Share Mode
Lets players choose between:
- **Single** - Held item, gives exp to holder only
- **All** - Key item, toggleable, all party Pokemon gain exp

Uses `MULTI_EXP_SHARE_MODE` multichoice and swaps between `ITEM_EXP_SHARE` and `ITEM_EXP_SHARE_ALL`.

## Adding a New Notebook Option

### Step 1: Define Multichoice Constant
In `include/constants/script_menu.h`:
```c
#define MULTI_MY_OPTION    116
```

### Step 2: Create Menu Entries
In `src/data/script_menu.h`:
```c
static const struct MenuAction MultichoiceList_MyOption[] =
{
    {COMPOUND_STRING("Choice A")},
    {COMPOUND_STRING("Choice B")},
    {COMPOUND_STRING("Choice C")},
};
```

Add to `sMultichoiceLists` array:
```c
[MULTI_MY_OPTION] = MULTICHOICE(MultichoiceList_MyOption),
```

### Step 3: Add Flag or Var (if needed)
For persistent settings, use an unused flag or var:
```c
// In include/constants/flags.h
#define FLAG_MY_SETTING    0x23  // Pick an unused flag

// Or in include/constants/vars.h for multiple states
#define VAR_MY_SETTING    0x40XX
```

### Step 4: Add Script Handler
In `data/scripts/players_house.inc`, add to the notebook script:
```
PlayersHouse_2F_EventScript_Notebook::
    lockall
    @ ... existing options ...
    msgbox PlayersHouse_2F_Text_MyOptionQuestion, MSGBOX_DEFAULT
    multichoice 14, 6, MULTI_MY_OPTION, FALSE
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

### Step 5: Add Text Strings
```
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

- Notebook is in both Brendan's and May's house (shared script)
- Options should work regardless of game progress
- Use flags for on/off settings, vars for multiple states
- Cancel (case 127) should exit cleanly without changes
- Keep text concise for GBA screen width
