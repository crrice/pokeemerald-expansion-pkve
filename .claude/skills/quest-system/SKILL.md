---
name: quest-system
description: Quest menu system for tracking player missions. Use when implementing questlines, modifying quest states, or adding new quests.
---

# Quest System

Ported from [ghoulslash/pokeemerald quest-menu](https://github.com/ghoulslash/pokeemerald/tree/pokemon_unbound/quest-menu). Provides an Unbound-style quest tracker accessible from the start menu.

## Key Files

| File | Purpose |
|------|---------|
| `src/quests.c` | Main quest menu UI and state management (~2800 lines) |
| `include/quests.h` | Public API |
| `include/constants/quests.h` | Quest IDs and command constants |
| `src/strings.c` | Quest string definitions (gText_SideQuestName_*, etc.) |
| `src/scrcmd.c` | Script command implementations |
| `asm/macros/event.inc` | Script macros for quest commands |
| `data/event_scripts.s` | Must include `constants/quests.h` for scripts to work |
| `data/script_cmd_table.inc` | Opcode definitions (0xe5-0xe7) |
| `include/global.h` | Save data in SaveBlock2 |
| `graphics/quest_menu/` | Menu graphics |

## Quest States

```
LOCKED → ACTIVE → REWARD → COMPLETE
```

- **Locked**: Shows as "??????" in menu (hidden from player)
- **Active**: Quest name/description visible, player is working on it
- **Reward**: Quest objective complete, reward available to claim
- **Complete**: Finished and claimed

## Enabling the Quest Menu

The menu only appears in the start menu when `FLAG_SYS_QUEST_MENU_GET` is set:

```
setflag FLAG_SYS_QUEST_MENU_GET
```

## Script Macros

### Basic Quest Control

```asm
@ Activate a quest (Locked → Active)
@ Shows announcement with fanfare
startquest QUEST_ID

@ Complete a quest (Reward → Complete)
@ Shows announcement with fanfare
completequest QUEST_ID

@ Set quest to reward state (Active → Reward)
@ Use this when player completes objective but hasn't talked to NPC yet
@ No announcement - silent state change
questmenu QUEST_MENU_SET_REWARD, QUEST_ID
```

**Note**: There is no `checkquest` macro. Use the conditional branching macros below.

### Conditional Branching

```asm
@ Branch based on quest state
goto_if_quest_active QUEST_ID, Label
goto_if_quest_inactive QUEST_ID, Label
goto_if_quest_complete QUEST_ID, Label
goto_if_quest_reward QUEST_ID, Label

@ Negated versions
goto_if_quest_not_active QUEST_ID, Label
goto_if_quest_not_inactive QUEST_ID, Label
goto_if_quest_not_complete QUEST_ID, Label
goto_if_quest_not_reward QUEST_ID, Label

@ Call variants (return after label)
call_if_quest_active QUEST_ID, Label
call_if_quest_inactive QUEST_ID, Label
call_if_quest_complete QUEST_ID, Label
call_if_quest_reward QUEST_ID, Label
```

### Low-Level Quest Menu Commands

```asm
@ Direct questmenu command (case, questId)
questmenu QUEST_MENU_SET_ACTIVE, QUEST_ID    @ Start quest
questmenu QUEST_MENU_SET_REWARD, QUEST_ID    @ Set to reward state
questmenu QUEST_MENU_COMPLETE_QUEST, QUEST_ID @ Complete quest
questmenu QUEST_MENU_CHECK_ACTIVE, QUEST_ID  @ Sets VAR_RESULT
questmenu QUEST_MENU_CHECK_REWARD, QUEST_ID  @ Sets VAR_RESULT
questmenu QUEST_MENU_CHECK_COMPLETE, QUEST_ID @ Sets VAR_RESULT
questmenu QUEST_MENU_BUFFER_QUEST_NAME, QUEST_ID @ Buffer to STR_VAR_1
```

### Subquests

```asm
@ For multi-part quests
startsubquest QUEST_ID, SUBQUEST_INDEX
completesubquest QUEST_ID, SUBQUEST_INDEX
goto_if_subquest_complete QUEST_ID, SUBQUEST_INDEX, Label
```

## Adding a New Quest

### Step 1: Define Quest ID

In `include/constants/quests.h`, replace a placeholder or add new:
```c
#define QUEST_BERRY_THIEF 0   // Replace QUEST_1
#define QUEST_2           1
// ...
```

### Step 2: Update Quest Strings

In `src/strings.c`, find and update the corresponding numbered strings:
```c
// For QUEST_BERRY_THIEF (index 0), update these:
const u8 gText_SideQuestName_1[] = _("Berry Thief");
const u8 gText_SideQuestDesc_1[] = _("Help the old farmer\nfind his berry thief.");
const u8 gText_SideQuestDoneDesc_1[] = _("Caught the berry thief\nfor the old farmer.");
const u8 gText_SideQuestMap1[] = _("Oldale Ruins");
```

**Note**: The strings use numbered suffixes (_1, _2, etc.) matching the quest index + 1.

### Step 3: Update Quest Data

In `src/quests.c`, find the `sSideQuests` array and update the corresponding entry:
```c
side_quest(
    gText_SideQuestName_1,
    gText_SideQuestDesc_1,
    gText_SideQuestDoneDesc_1,
    gText_SideQuestMap1,
    SPECIES_PHANTUMP,    // sprite ID
    PKMN,                // sprite type: OBJECT, ITEM, or PKMN
    NULL,                // subquests array (or pointer to subquest array)
    0                    // number of subquests
),
```

### Step 4: Use in Map Scripts

```asm
MyMap_EventScript_QuestGiver::
    lock
    faceplayer
    goto_if_quest_complete QUEST_MY_QUEST, MyMap_EventScript_QuestDone
    goto_if_quest_reward QUEST_MY_QUEST, MyMap_EventScript_QuestReward
    goto_if_quest_active QUEST_MY_QUEST, MyMap_EventScript_QuestInProgress
    @ Quest not started yet
    msgbox MyMap_Text_QuestIntro, MSGBOX_DEFAULT
    startquest QUEST_MY_QUEST
    release
    end

MyMap_EventScript_QuestInProgress::
    msgbox MyMap_Text_QuestReminder, MSGBOX_DEFAULT
    release
    end

MyMap_EventScript_QuestReward::
    @ Player completed objective, give reward
    msgbox MyMap_Text_QuestReward, MSGBOX_DEFAULT
    completequest QUEST_MY_QUEST
    release
    end

MyMap_EventScript_QuestDone::
    msgbox MyMap_Text_QuestThanks, MSGBOX_DEFAULT
    release
    end
```

## Quest Sprite Types

The sprite field uses separate fields for ID and type:

```c
.sprite = OBJ_EVENT_GFX_WOMAN_1,  // NPC sprite
.spritetype = OBJECT,

.sprite = ITEM_POKEBALL,          // Item icon
.spritetype = ITEM,

.sprite = SPECIES_PIKACHU,        // Pokemon icon
.spritetype = PKMN,
```

## C API for Quest State

To check quest state from C code (e.g., for time-based events):

```c
#include "quests.h"

// Check quest states
bool8 isComplete = QuestMenu_GetSetQuestState(QUEST_ID, FLAG_GET_COMPLETED);
bool8 isReward = QuestMenu_GetSetQuestState(QUEST_ID, FLAG_GET_REWARD);
bool8 isActive = QuestMenu_GetSetQuestState(QUEST_ID, FLAG_GET_ACTIVE);

// Available flag constants (from quests.h):
// FLAG_GET_UNLOCKED, FLAG_GET_INACTIVE, FLAG_GET_ACTIVE
// FLAG_GET_REWARD, FLAG_GET_COMPLETED, FLAG_GET_FAVORITE
// FLAG_SET_* variants for setting state
```

## Save Data

Quest state is stored in `SaveBlock2`:
- `questData[QUEST_FLAGS_COUNT * QUEST_STATES]` - Main quest flags
- `subQuests[SUB_FLAGS_COUNT]` - Subquest completion flags

## Common Gotchas

### 1. "undefined reference to QUEST_*" linker errors
Ensure `#include "constants/quests.h"` is in `data/event_scripts.s` (around line 52, with other constant includes).

### 2. No `checkquest` macro exists
Use `goto_if_quest_*` macros for branching, or `questmenu QUEST_MENU_CHECK_*, QUEST_ID` which sets `VAR_RESULT`.

### 3. Setting REWARD state silently
`startquest` and `completequest` show announcements. To silently advance to REWARD (e.g., after catching a Pokemon), use:
```asm
questmenu QUEST_MENU_SET_REWARD, QUEST_ID
```

### 4. Quest strings are numbered, not named
Strings in `src/strings.c` use `gText_SideQuestName_1`, `gText_SideQuestDesc_1`, etc. - not custom names.

## Current Quests

30 placeholder quests are defined (QUEST_1 through QUEST_30). Replace these with actual quest content as you implement questlines.

## Design Notes

- All quests are visible in "All Missions" tab from the start (as "??????" until active)
- To hide locked quests entirely, modify the list-building functions in `src/quests.c`
- Quest menu uses opcodes 0xe5, 0xe6, 0xe7 (avoiding conflicts with pokeemerald-expansion)
- Subquests are optional - set `numSubquests = 0` for simple quests
