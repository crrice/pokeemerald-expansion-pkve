// Scripted battle data for TV battles and other non-interactive sequences

#include "constants/species.h"
#include "constants/moves.h"
#include "constants/items.h"
#include "constants/pokemon.h"
#include "constants/trainers.h"
#include "battle_transition.h"
#include "scripted_battle.h"

// ===============================
// Norman TV Battle
// ===============================
// Norman's Cofagrigus sacrifices itself to transfer Mummy to the opponent's
// Mightyena, then Slaking comes in and sweeps without Truant!

// Norman's party - competitive Pokemon with good IVs/EVs
static const struct ScriptedPokemon sNormanTVBattle_Linoone = {
    .species = SPECIES_LINOONE,
    .level = 44,
    .currentHP = 0,  // Fainted
    .moves = {MOVE_SLASH, MOVE_HEADBUTT, MOVE_NONE, MOVE_NONE},
    .status = 0,
    .abilityNum = 0,
    .heldItem = ITEM_NONE,
    .ivs = {31, 31, 31, 31, 31, 31},
    .evs = {0, 252, 0, 252, 0, 0},
    .nature = NATURE_ADAMANT,
};

static const struct ScriptedPokemon sNormanTVBattle_Zangoose = {
    .species = SPECIES_ZANGOOSE,
    .level = 44,
    .currentHP = 0,  // Fainted
    .moves = {MOVE_SLASH, MOVE_CRUSH_CLAW, MOVE_NONE, MOVE_NONE},
    .status = 0,
    .abilityNum = 0,
    .heldItem = ITEM_NONE,
    .ivs = {31, 31, 31, 31, 31, 31},
    .evs = {0, 252, 0, 252, 0, 0},
    .nature = NATURE_ADAMANT,
};

static const struct ScriptedPokemon sNormanTVBattle_Cofagrigus = {
    .species = SPECIES_COFAGRIGUS,
    .level = 46,
    .currentHP = 25,  // Low HP, about to go down
    .moves = {MOVE_SHADOW_BALL, MOVE_WILL_O_WISP, MOVE_PROTECT, MOVE_HEX},
    .status = 0,
    .abilityNum = 0,  // Mummy
    .heldItem = ITEM_LEFTOVERS,
    .ivs = {31, 31, 31, 31, 31, 31},
    .evs = {252, 0, 128, 0, 0, 128},
    .nature = NATURE_BOLD,
};

static const struct ScriptedPokemon sNormanTVBattle_Slaking = {
    .species = SPECIES_SLAKING,
    .level = 48,
    .currentHP = 45,  // Low HP - clutch scenario, would lose if Truant kicked in
    .moves = {MOVE_RETURN, MOVE_EARTHQUAKE, MOVE_SHADOW_CLAW, MOVE_YAWN},
    .status = 0,
    .abilityNum = 0,  // Truant (will become Mummy!)
    .heldItem = ITEM_NONE,
    .ivs = {31, 31, 31, 31, 31, 31},  // Perfect IVs
    .evs = {4, 252, 0, 252, 0, 0},    // Max Attack and Speed
    .nature = NATURE_ADAMANT,         // +Atk -SpAtk
    .friendship = 255,                // Max friendship for max Return power
};

// Challenger's party - weaker Pokemon with bad IVs/EVs
static const struct ScriptedPokemon sNormanTVBattle_ChallengerPoochyena = {
    .species = SPECIES_POOCHYENA,
    .level = 42,
    .currentHP = 0,  // Fainted
    .moves = {MOVE_BITE, MOVE_NONE, MOVE_NONE, MOVE_NONE},
    .status = 0,
    .abilityNum = 0,
    .heldItem = ITEM_NONE,
    .ivs = {0, 0, 0, 0, 0, 0},
    .evs = {0, 0, 0, 0, 0, 0},
    .nature = NATURE_HARDY,
};

static const struct ScriptedPokemon sNormanTVBattle_ChallengerNuzleaf = {
    .species = SPECIES_NUZLEAF,
    .level = 43,
    .currentHP = 0,  // Fainted
    .moves = {MOVE_RAZOR_LEAF, MOVE_NONE, MOVE_NONE, MOVE_NONE},
    .status = 0,
    .abilityNum = 0,
    .heldItem = ITEM_NONE,
    .ivs = {0, 0, 0, 0, 0, 0},
    .evs = {0, 0, 0, 0, 0, 0},
    .nature = NATURE_HARDY,
};

static const struct ScriptedPokemon sNormanTVBattle_ChallengerMightyena = {
    .species = SPECIES_MIGHTYENA,
    .level = 45,
    .currentHP = 40,  // Damaged - should be OHKO'd
    .moves = {MOVE_CRUNCH, MOVE_TAKE_DOWN, MOVE_ROAR, MOVE_SWAGGER},
    .status = 0,
    .abilityNum = 2,  // Moxie (HA) - hints to players that HAs are available
    .heldItem = ITEM_NONE,
    .ivs = {0, 0, 0, 0, 0, 0},
    .evs = {0, 0, 0, 0, 0, 0},
    .nature = NATURE_HARDY,
};

// Hariyama - the threat! Close Combat would OHKO a weakened Slaking
static const struct ScriptedPokemon sNormanTVBattle_ChallengerHariyama = {
    .species = SPECIES_HARIYAMA,
    .level = 46,
    .currentHP = 0xFFFF,  // Full HP - let's see if Return can OHKO
    .moves = {MOVE_CLOSE_COMBAT, MOVE_KNOCK_OFF, MOVE_BULK_UP, MOVE_FAKE_OUT},
    .status = 0,
    .abilityNum = 0,  // Thick Fat
    .heldItem = ITEM_NONE,
    .ivs = {15, 20, 15, 10, 15, 15},  // Decent but not great IVs
    .evs = {100, 150, 0, 0, 0, 0},    // Some investment
    .nature = NATURE_LONELY,  // +Atk, -Def to ensure OHKO
};

// Action scripts
// Player side (position 0): Challenger with Mightyena/Hariyama
// Opponent side (position 1): Norman with Cofagrigus/Slaking

// Player script: Mightyena uses Crunch, then Hariyama is forced in and doesn't get to act
static const struct ScriptedBattleAction sNormanTVBattle_PlayerScript[] = {
    // Turn 1: Mightyena uses Crunch on Cofagrigus
    {SCRIPTED_ACTION_USE_MOVE, 0, 1},  // Move slot 0 (Crunch), target opponent

    // Turn 2: Mightyena is KO'd, Hariyama switches in
    // (handled automatically by battle system)

    // Turn 3: Hariyama doesn't get to act (KO'd by Return before it can move)

    {SCRIPTED_ACTION_END, 0, 0},
};

// Opponent script: Cofagrigus faints, Slaking switches in and uses Return twice
static const struct ScriptedBattleAction sNormanTVBattle_OpponentScript[] = {
    // Turn 1: Cofagrigus faints from Crunch before acting
    // The battle system will ask for switch when Cofagrigus faints

    // Turn 2: Slaking uses Return on Mightyena
    {SCRIPTED_ACTION_USE_MOVE, 0, 0},  // Move slot 0 (Return), target player

    // Turn 3: Slaking uses Return on Hariyama
    {SCRIPTED_ACTION_USE_MOVE, 0, 0},  // Move slot 0 (Return), target player

    {SCRIPTED_ACTION_END, 0, 0},
};

// Announcer messages for TV broadcast (highlights reel style)
static const u8 sNormanTV_Announcer_Intro[] = _("Watch this highlight from PETALBURG GYM!\pNorman was down to his last two POKéMON!");
static const u8 sNormanTV_Announcer_SlakingSwitchIn[] = _("With COFAGRIGUS down, Norman sent out SLAKING!\pCould that lazy POKéMON turn things around?");
static const u8 sNormanTV_Announcer_BattleEnd[] = _("But SLAKING showed incredible energy!\pA stunning comeback for Leader Norman!");

// The full battle definition
// Player (position 0): Challenger - left/lower side of screen
// Opponent (position 1): Norman - right/upper side of screen
const struct ScriptedBattle gScriptedBattle_NormanTV = {
    .playerParty = {
        &sNormanTVBattle_ChallengerPoochyena,  // Fainted
        &sNormanTVBattle_ChallengerNuzleaf,    // Fainted
        &sNormanTVBattle_ChallengerMightyena,  // Active
        &sNormanTVBattle_ChallengerHariyama,   // Waiting in back - the threat!
        NULL,
        NULL,
    },
    .opponentParty = {
        &sNormanTVBattle_Linoone,      // Fainted
        &sNormanTVBattle_Zangoose,     // Fainted
        &sNormanTVBattle_Cofagrigus,   // Active, low HP
        &sNormanTVBattle_Slaking,      // Waiting in back, low HP
        NULL,
        NULL,
    },
    .playerScript = sNormanTVBattle_PlayerScript,      // Challenger's moves (position 0)
    .opponentScript = sNormanTVBattle_OpponentScript, // Norman's moves (position 1)
    .playerName = NULL,  // Will use default
    .opponentName = NULL,
    .playerBackPic = TRAINER_BACK_PIC_LEAF,
    .opponentTrainerPic = TRAINER_PIC_LEADER_NORMAN,
    .announcerTrainerPic = TRAINER_PIC_INTERVIEWER,
    .opponentTrainerId = TRAINER_NORMAN_1,
    .battleFlags = 0,
    .rngSeed = 314,  // Fixed seed for deterministic damage rolls
    .transitionId = B_TRANSITION_GRID_SQUARES,
    .announcer_BeforeFirstTurn = sNormanTV_Announcer_Intro,
    .announcer_LastSwitchIn = sNormanTV_Announcer_SlakingSwitchIn,
    .announcer_BattleEnd = sNormanTV_Announcer_BattleEnd,
};
