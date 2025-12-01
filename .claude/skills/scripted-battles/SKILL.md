---
name: scripted-battles
description: Custom scripted battle system for TV replays and non-interactive battle sequences. Use when discussing, debugging, or extending scripted battles, the Norman TV battle, or adding new pre-choreographed battle sequences.
---

# Scripted Battle System

Fully deterministic, pre-choreographed battles that play out automatically. Used for TV replays, cutscenes, or any non-interactive battle sequence.

## Key Files

| File | Purpose |
|------|---------|
| `include/scripted_battle.h` | Data structures and API declarations |
| `src/battle_controller_scripted.c` | Battle controller + `PlayScriptedBattle()` entry point |
| `src/data/scripted_battles.h` | Battle definitions (Pokemon, moves, action scripts) |

## Architecture Overview

### Battle Controller Pattern
The scripted controller follows pokeemerald's controller pattern with a command table mapping `CONTROLLER_*` commands to handlers. It reuses shared `BtlController_Handle*` functions where possible.

### Battle Type Flags
`BATTLE_TYPE_SCRIPTED` (bit 28) combined with `BATTLE_TYPE_RECORDED` provides:
- No save data modifications (money, party unchanged)
- Fixed RNG via `gRecordedBattleRngSeed`
- "Set" battle style (no switch prompts)
- Disabled low HP beeping

### Action Script Flow
Each `ScriptedBattleAction` entry = one turn per battler:
1. `ChooseAction` **peeks** at `actionType` (doesn't consume)
2. `ChooseMove`/`ChoosePokemon` **consumes** the entry

This matches how the battle engine calls controllers: action type first, then details.

### Party Save/Restore
Player's real party is saved (dynamically allocated), replaced with scripted parties during battle, then restored after. Memory is freed on restore.

## Adding a New Scripted Battle

### Step 1: Define Pokemon
```c
static const struct ScriptedPokemon sMyBattle_Pokemon1 = {
    .species = SPECIES_PIKACHU,
    .level = 50,
    .currentHP = 0xFFFF,  // Full HP (0 = fainted, specific value = that HP)
    .moves = {MOVE_THUNDERBOLT, MOVE_QUICK_ATTACK, MOVE_NONE, MOVE_NONE},
    .abilityNum = 0,      // 0/1 = normal, 2 = hidden
    .ivs = {31, 31, 31, 31, 31, 31},
    .evs = {0, 0, 0, 252, 252, 0},
    .nature = NATURE_TIMID,
    .friendship = 255,    // For Return power
};
```

### Step 2: Define Action Scripts
```c
static const struct ScriptedBattleAction sMyBattle_PlayerScript[] = {
    {SCRIPTED_ACTION_USE_MOVE, 0, 1},  // Move slot 0, target battler 1
    {SCRIPTED_ACTION_USE_MOVE, 1, 1},  // Move slot 1, target battler 1
    {SCRIPTED_ACTION_END, 0, 0},
};
```

### Step 3: Create Battle Definition
```c
const struct ScriptedBattle gScriptedBattle_MyBattle = {
    .playerParty = {&sMyBattle_Pokemon1, NULL, NULL, NULL, NULL, NULL},
    .opponentParty = {&sMyBattle_Pokemon2, NULL, NULL, NULL, NULL, NULL},
    .playerScript = sMyBattle_PlayerScript,
    .opponentScript = sMyBattle_OpponentScript,
    .playerBackPic = TRAINER_BACK_PIC_BRENDAN,
    .opponentTrainerPic = TRAINER_PIC_LEADER_ROXANNE,
    .opponentTrainerId = TRAINER_ROXANNE_1,  // For name display
    .rngSeed = 12345,  // Fixed seed for determinism
    .transitionId = B_TRANSITION_BLUR,
    // Optional custom messages:
    .introText = NULL,  // NULL = default "The challenger faces X!"
    .announcer_BeforeFirstTurn = sMyAnnouncer_Intro,
    .announcer_LastSwitchIn = NULL,
    .announcer_BattleEnd = sMyAnnouncer_Ending,
};
```

### Step 4: Add Extern and Starter Function
In `scripted_battle.h`:
```c
extern const struct ScriptedBattle gScriptedBattle_MyBattle;
void Script_StartMyBattle(struct ScriptContext *ctx);
```

In `battle_controller_scripted.c`:
```c
void Script_StartMyBattle(struct ScriptContext *ctx)
{
    PlayScriptedBattle(&gScriptedBattle_MyBattle, CB2_ReturnToFieldContinueScript);
}
```

### Step 5: Trigger from Event Script
```
callnative Script_StartMyBattle
waitstate
```

## Announcer Messages

Three slide-in message points available:
- `announcer_BeforeFirstTurn` - After intro, before first turn
- `announcer_LastSwitchIn` - When last Pokemon enters
- `announcer_BattleEnd` - After battle concludes

These use the trainer slide system with a custom announcer sprite (`announcerTrainerPic`).

## Terminology Note

"Player" side = left side (back sprite), "Opponent" side = right side (front sprite). This matches battle engine conventions regardless of who "wins" narratively.
