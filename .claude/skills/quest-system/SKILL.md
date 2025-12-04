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
| `src/scrcmd.c` | Script command implementations |
| `asm/macros/event.inc` | Script macros for quest commands |
| `data/script_cmd_table.inc` | Opcode definitions (0xe5-0xe7) |
| `include/global.h` | Save data in SaveBlock2 |
| `graphics/quest_menu/` | Menu graphics |

## Quest States

```
LOCKED → ACTIVE → REWARD → COMPLETE
```

- **Locked**: Shows as "??????" in menu (hidden from player)
- **Active**: Quest name/description visible, player is working on it
- **Reward**: Quest complete, reward available to claim
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
startquest QUEST_ID

@ Complete a quest (Active → Reward → Complete)
completequest QUEST_ID

@ Check quest state and store in VAR_RESULT
checkquest QUEST_ID
```

### Conditional Branching

```asm
@ Branch based on quest state
goto_if_quest_active QUEST_ID, Label
goto_if_quest_inactive QUEST_ID, Label
goto_if_quest_complete QUEST_ID, Label
goto_if_quest_reward QUEST_ID, Label

@ Call variants
call_if_quest_active QUEST_ID, Label
call_if_quest_inactive QUEST_ID, Label
call_if_quest_complete QUEST_ID, Label
call_if_quest_reward QUEST_ID, Label
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

In `include/constants/quests.h`:
```c
#define QUEST_BERRY_THIEF    0
#define QUEST_LOST_POKEMON   1
// ... existing quests ...
#define QUEST_MY_NEW_QUEST   30  // Use next available

#define QUEST_COUNT          31  // Update count
```

### Step 2: Add Quest Data

In `src/quests.c`, find the `sQuestList` array and add entry:
```c
[QUEST_MY_NEW_QUEST] = {
    .name = sQuestName_MyNewQuest,
    .desc = sQuestDesc_MyNewQuest,
    .map = sQuestMap_MyNewQuest,
    .sprite = {OBJECT, OBJ_EVENT_GFX_WOMAN_1},  // or ITEM/PKMN
    .numSubquests = 0,  // or number of subquests
},
```

### Step 3: Add Quest Strings

In `src/quests.c`, add string definitions:
```c
static const u8 sQuestName_MyNewQuest[] = _("Quest Name");
static const u8 sQuestDesc_MyNewQuest[] = _("Description of what\nthe player needs to do.");
static const u8 sQuestMap_MyNewQuest[] = _("Location Name");
```

### Step 4: Use in Map Scripts

```asm
MyMap_EventScript_QuestGiver::
    lock
    faceplayer
    goto_if_quest_complete QUEST_MY_NEW_QUEST, MyMap_EventScript_QuestDone
    goto_if_quest_active QUEST_MY_NEW_QUEST, MyMap_EventScript_QuestInProgress
    @ Quest not started yet
    msgbox MyMap_Text_QuestIntro, MSGBOX_DEFAULT
    startquest QUEST_MY_NEW_QUEST
    release
    end

MyMap_EventScript_QuestInProgress::
    msgbox MyMap_Text_QuestReminder, MSGBOX_DEFAULT
    release
    end

MyMap_EventScript_QuestDone::
    msgbox MyMap_Text_QuestThanks, MSGBOX_DEFAULT
    release
    end
```

## Quest Sprite Types

The `sprite` field uses a type and ID:

```c
.sprite = {OBJECT, OBJ_EVENT_GFX_WOMAN_1},  // NPC sprite
.sprite = {ITEM, ITEM_POKEBALL},             // Item icon
.sprite = {PKMN, SPECIES_PIKACHU},           // Pokemon icon
```

## Save Data

Quest state is stored in `SaveBlock2`:
- `questData[QUEST_FLAGS_COUNT * QUEST_STATES]` - Main quest flags
- `subQuests[SUB_FLAGS_COUNT]` - Subquest completion flags

## Current Quests (Placeholders)

30 placeholder quests are defined (QUEST_1 through QUEST_30). Replace these with actual quest content as you implement questlines.

## Design Notes

- All quests are visible in "All Missions" tab from the start (as "??????" until active)
- To hide locked quests entirely, modify the list-building functions in `src/quests.c`
- Quest menu uses opcodes 0xe5, 0xe6, 0xe7 (avoiding conflicts with pokeemerald-expansion)
- Subquests are optional - set `numSubquests = 0` for simple quests
