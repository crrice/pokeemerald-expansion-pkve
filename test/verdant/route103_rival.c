#include "global.h"
#include "test/battle.h"
#include "battle_main.h"
#include "data.h"
#include "move.h"
#include "starter_choose.h"
#include "constants/opponents.h"
#include "unrigged_battle.h"

// Route 103 rival battle balance, measured with the game's real RNG
// (accuracy, crits, damage rolls, secondary effects, AI) over 50 seeds.
//
// Scenario: the player's starter is level 6 and only ever uses its strongest
// STAB move. The rival's party is generated from the real trainer data, so its
// level, IVs, nature, gender, ability and moves are exactly what the game builds.
//
// Axes (see CLAUDE.md, Custom Starters):
//   player gender picks the rival: boy -> May, girl -> Brendan
//   starter slot (VAR_STARTER_MON): 0 = TREECKO slot = player picked Axew,
//                                   1 = TORCHIC slot = player picked Deino
//
// Player starter profile (fixed per test; the real game rolls these randomly):
// level 6, all IVs 15, neutral nature, first ability slot, male.
//
// How a win is counted: the rival also gets a filler Pokemon that only Splashes,
// so the battle survives the rival's lead fainting and all scripted turns run.
// If the player's starter faints, the battle ends early and the run fails
// (= a loss). THEN requires the rival's real lead to have fainted.

#define PLAYER_LEVEL    6
#define PLAYER_IV       15
#define BATTLE_TURNS    6

static struct Pokemon sRivalParty[PARTY_SIZE];

// The moves a freshly created Pokemon knows at this level (last four learned).
static void GetDefaultMoves(u32 species, u32 level, u16 *moves)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    u32 count = 0;
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        moves[i] = MOVE_NONE;
    for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END && learnset[i].level <= level; i++)
    {
        bool32 known = FALSE;
        for (u32 j = 0; j < MAX_MON_MOVES; j++)
            known |= (moves[j] == learnset[i].move);
        if (known)
            continue;
        if (count < MAX_MON_MOVES)
        {
            moves[count++] = learnset[i].move;
        }
        else
        {
            for (u32 j = 0; j < MAX_MON_MOVES - 1; j++)
                moves[j] = moves[j + 1];
            moves[MAX_MON_MOVES - 1] = learnset[i].move;
        }
    }
}

// Strongest damaging move whose type matches one of the species' types.
static u32 GetStrongestStabMove(u32 species, const u16 *moves)
{
    u32 best = MOVE_NONE;
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
    {
        u32 move = moves[i];
        if (move == MOVE_NONE || GetMovePower(move) == 0)
            continue;
        if (GetMoveType(move) != GetSpeciesType(species, 0) && GetMoveType(move) != GetSpeciesType(species, 1))
            continue;
        if (best == MOVE_NONE || GetMovePower(move) > GetMovePower(best))
            best = move;
    }
    return best;
}

#define ROUTE103_RIVAL_BATTLE(trainerId, starterSlot)                                                       \
    u32 starter = GetStarterPokemon(starterSlot);                                                            \
    u16 playerMoves[MAX_MON_MOVES];                                                                          \
    u32 stabMove;                                                                                            \
    struct Pokemon *rival = &sRivalParty[0];                                                                 \
    GetDefaultMoves(starter, PLAYER_LEVEL, playerMoves);                                                     \
    stabMove = GetStrongestStabMove(starter, playerMoves);                                                   \
    /* Pinned to the measured rate (Oct 2026: 50/50 in every combination). Pending a design decision on   \
       what's acceptable; set PASSES_RANDOMLY(0, 1) to re-measure (the failure message reports the ratio). */ \
    PASSES_RANDOMLY(50, 50);                                                                                 \
    GIVEN {                                                                                                  \
        ASSUME(stabMove != MOVE_NONE);                                                                       \
        CreateNPCTrainerPartyFromTrainer(sRivalParty, GetTrainerStructFromId(trainerId), TRUE, BATTLE_TYPE_TRAINER); \
        AI_FLAGS(GetTrainerStructFromId(trainerId)->aiFlags);                                                \
        PLAYER(starter) {                                                                                    \
            Level(PLAYER_LEVEL);                                                                             \
            Nature(NATURE_HARDY);                                                                            \
            Ability(GetSpeciesAbility(starter, 0));                                                          \
            Gender(MON_MALE);                                                                                \
            HPIV(PLAYER_IV); AttackIV(PLAYER_IV); DefenseIV(PLAYER_IV);                                      \
            SpAttackIV(PLAYER_IV); SpDefenseIV(PLAYER_IV); SpeedIV(PLAYER_IV);                               \
            Moves(playerMoves[0], playerMoves[1], playerMoves[2], playerMoves[3]);                           \
        }                                                                                                    \
        OPPONENT(GetMonData(rival, MON_DATA_SPECIES)) {                                                      \
            Level(GetMonData(rival, MON_DATA_LEVEL));                                                        \
            Nature(GetNature(rival));                                                                        \
            Ability(GetMonAbility(rival));                                                                   \
            Gender(GetMonGender(rival));                                                                     \
            HPIV(GetMonData(rival, MON_DATA_HP_IV));                                                         \
            AttackIV(GetMonData(rival, MON_DATA_ATK_IV));                                                    \
            DefenseIV(GetMonData(rival, MON_DATA_DEF_IV));                                                   \
            SpAttackIV(GetMonData(rival, MON_DATA_SPATK_IV));                                                \
            SpDefenseIV(GetMonData(rival, MON_DATA_SPDEF_IV));                                               \
            SpeedIV(GetMonData(rival, MON_DATA_SPEED_IV));                                                   \
            Moves(GetMonData(rival, MON_DATA_MOVE1), GetMonData(rival, MON_DATA_MOVE2),                      \
                  GetMonData(rival, MON_DATA_MOVE3), GetMonData(rival, MON_DATA_MOVE4));                     \
        }                                                                                                    \
        OPPONENT(SPECIES_WOBBUFFET) { Level(100); Moves(MOVE_SPLASH); } /* filler, see above */              \
    } WHEN {                                                                                                 \
        for (u32 turn = 0; turn < BATTLE_TURNS; turn++)                                                      \
            TURN { MOVE(player, stabMove); }                                                                 \
    } THEN {                                                                                                 \
        /* party HP isn't synced from the live battle; the filler being out means the lead fainted */   \
        EXPECT_EQ(opponent->species, SPECIES_WOBBUFFET);                                                    \
    }

UNRIGGED_AI_SINGLE_BATTLE_TEST("Verdant: Route 103 rival win rate - boy, Axew vs May")
{
    ROUTE103_RIVAL_BATTLE(TRAINER_MAY_ROUTE_103_TREECKO, 0);
}

UNRIGGED_AI_SINGLE_BATTLE_TEST("Verdant: Route 103 rival win rate - boy, Deino vs May")
{
    ROUTE103_RIVAL_BATTLE(TRAINER_MAY_ROUTE_103_TORCHIC, 1);
}

UNRIGGED_AI_SINGLE_BATTLE_TEST("Verdant: Route 103 rival win rate - girl, Axew vs Brendan")
{
    ROUTE103_RIVAL_BATTLE(TRAINER_BRENDAN_ROUTE_103_TREECKO, 0);
}

UNRIGGED_AI_SINGLE_BATTLE_TEST("Verdant: Route 103 rival win rate - girl, Deino vs Brendan")
{
    ROUTE103_RIVAL_BATTLE(TRAINER_BRENDAN_ROUTE_103_TORCHIC, 1);
}

