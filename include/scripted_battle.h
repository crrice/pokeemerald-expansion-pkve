#ifndef GUARD_SCRIPTED_BATTLE_H
#define GUARD_SCRIPTED_BATTLE_H

#include "pokemon.h"

// Action types for scripted battles
#define SCRIPTED_ACTION_END         0xFF
#define SCRIPTED_ACTION_USE_MOVE    0
#define SCRIPTED_ACTION_SWITCH      1

// A single action in a scripted battle.
// Each entry represents ONE turn's worth of action for a battler.
// The battle controller will:
//   1. Peek at actionType when ChooseAction is called
//   2. Consume the full action when ChooseMove/ChoosePokemon is called
// So each ScriptedBattleAction entry = one turn, NOT two entries per turn.
struct ScriptedBattleAction
{
    u8 actionType;      // SCRIPTED_ACTION_* constant
    u8 moveSlotOrPartyIndex;  // Move slot (0-3) for moves, party index for switches
    u8 target;          // Target battler position (for moves)
};

// Pre-configured Pokemon state for scripted battles
struct ScriptedPokemon
{
    u16 species;
    u8 level;
    u16 currentHP;      // 0 = fainted, 0xFFFF = max HP
    u16 moves[MAX_MON_MOVES];
    u8 status;          // STATUS1_* constants (0 = healthy)
    u8 abilityNum;      // 0, 1, or 2 (hidden)
    u16 heldItem;
    u8 ivs[NUM_STATS];  // 0-31 for each stat (HP/Atk/Def/Speed/SpAtk/SpDef)
    u8 evs[NUM_STATS];  // 0-252 for each stat
    u8 nature;          // NATURE_* constant
    u8 friendship;      // 0-255, affects Return/Frustration power (255 = max Return)
};

// Full scripted battle definition
struct ScriptedBattle
{
    // Parties - use NULL entries for empty slots
    const struct ScriptedPokemon *playerParty[PARTY_SIZE];
    const struct ScriptedPokemon *opponentParty[PARTY_SIZE];

    // Action scripts - array of actions, terminated by SCRIPTED_ACTION_END
    const struct ScriptedBattleAction *playerScript;
    const struct ScriptedBattleAction *opponentScript;

    // Display info
    const u8 *playerName;
    const u8 *opponentName;
    u8 playerBackPic;       // TRAINER_BACK_PIC_* constant
    u8 opponentTrainerPic;  // TRAINER_PIC_* constant
    u8 announcerTrainerPic; // TRAINER_PIC_* constant for slide-in announcer (0 = use opponent pic)
    u16 opponentTrainerId;  // TRAINER_* constant for name display (e.g., TRAINER_NORMAN_1)

    // Battle configuration
    u32 battleFlags;        // Additional flags to OR with BATTLE_TYPE_SCRIPTED
    u32 rngSeed;            // Fixed RNG seed for deterministic playback (0 = use current RNG)
    u8 transitionId;        // B_TRANSITION_* constant (0 = no transition)

    // Battle intro text (NULL = use default "The challenger faces X!")
    const u8 *introText;

    // Announcer messages (NULL = no message for that event)
    const u8 *announcer_BeforeFirstTurn;    // Trainer slide-in before first turn
    const u8 *announcer_LastSwitchIn;       // When last mon switches in
    const u8 *announcer_BattleEnd;          // When battle ends (custom, not standard slide)
};

// Controller setup function
void SetControllerToScripted(u32 battler);

// Get custom intro text for scripted battles (returns NULL if not set or not scripted)
const u8 *GetScriptedBattleIntroText(void);

// Get announcer message for scripted battles (returns NULL if not scripted or no message)
const u8 *GetScriptedBattleAnnouncerMsg(u32 slideId);

// Start a scripted battle
void PlayScriptedBattle(const struct ScriptedBattle *battle, void (*callback)(void));

// Get the next action for a battler (used internally by controller)
const struct ScriptedBattleAction *GetNextScriptedAction(u32 battler);

// Pre-defined scripted battles
extern const struct ScriptedBattle gScriptedBattle_NormanTV;

// Script-callable battle starters (for use with callnative)
struct ScriptContext;
void Script_StartNormanTVBattle(struct ScriptContext *ctx);

#endif // GUARD_SCRIPTED_BATTLE_H
