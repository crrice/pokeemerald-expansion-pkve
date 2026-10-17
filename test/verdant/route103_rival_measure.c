#include "global.h"
#include "test/battle.h"
#include "battle_setup.h"
#include "data.h"
#include "move.h"
#include "starter_choose.h"
#include "constants/opponents.h"
#include "game_trainers.h"
#include "unrigged_battle.h"

// Route 103 rival battle: win-rate MEASUREMENT suite (not a pass/fail test).
//
// Run explicitly, then summarize:
//   TESTS="Verdant: [measure] Route 103" make check -j$(nproc) > /tmp/measure.log 2>&1
//   python3 test/verdant/summarize_measure.py /tmp/measure.log
//
// Every test here "fails" on purpose (PASSES_RANDOMLY(0, 1) runs 50 battles and reports
// the ratio); each won battle also prints a MEASURE-WIN line, which the summarizer counts.
// In normal runs (not selected by name) each plays a trivial one-turn battle and passes.
//
// Buckets: player gender (picks the rival) x starter, each with and without one
// buff/debuff move on turn 1 (Axew: Leer, Deino: Focus Energy), then strongest STAB
// every turn. 8 buckets x 10 starter rolls x 50 battle seeds = 4000 battles.
//
// Starter rolls follow the game's ScriptGiveMon rules (pure random, no IV floor):
// IVs 0-31 each, random nature, gender by the species ratio, ability slot 0 or 1.
// The rival is built from the real trainer data (level, 0 IVs, ability, moves);
// its nature and gender aren't in the data, so the game rolls them per save, and
// each roll here does too (nature, gender by the rival species' ratio).
// Rolls are generated from a fixed seed per roll number, so runs are reproducible.

#define MEASURE_PREFIX  "Verdant: [measure]"
#define PLAYER_LEVEL    6
#define BATTLE_TURNS    8   // decided in 2-4 turns at this level; undecided counts as a loss

struct StarterRoll
{
    u8 ivs[NUM_STATS];
    u8 nature;
    u8 gender;
    u8 abilitySlot;
    u8 rivalNature;
    u8 rivalGenderRoll;     // 0-255, compared with the rival species' gender ratio
};

static struct Pokemon sRivalParty[PARTY_SIZE];
static u32 sRollState;

static bool32 IsMeasureRun(void)
{
    return strncmp(gTestRunnerArgv, MEASURE_PREFIX, sizeof(MEASURE_PREFIX) - 1) == 0;
}

static u32 NextRoll(void)
{
    sRollState = sRollState * 1103515245 + 12345;
    return sRollState >> 16;
}

static void MakeStarterRoll(u32 species, u32 rollIndex, struct StarterRoll *roll)
{
    sRollState = 0x5EED0000 + rollIndex * 7919;
    for (u32 i = 0; i < NUM_STATS; i++)
        roll->ivs[i] = NextRoll() % (MAX_PER_STAT_IVS + 1);
    roll->nature = NextRoll() % NUM_NATURES;
    roll->gender = (NextRoll() % 256) < gSpeciesInfo[species].genderRatio ? MON_FEMALE : MON_MALE;
    roll->abilitySlot = NextRoll() % 2;
    if (GetSpeciesAbility(species, roll->abilitySlot) == ABILITY_NONE)
        roll->abilitySlot = 0;
    roll->rivalNature = NextRoll() % NUM_NATURES;
    roll->rivalGenderRoll = NextRoll() % 256;
}

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

// First non-damaging move (a buff or debuff): Leer for Axew, Focus Energy for Deino.
static u32 GetSetupMove(const u16 *moves)
{
    for (u32 i = 0; i < MAX_MON_MOVES; i++)
        if (moves[i] != MOVE_NONE && GetMovePower(moves[i]) == 0)
            return moves[i];
    return MOVE_NONE;
}

static void RunMeasuredRivalBattle(u32 trainerId, u32 starterSlot, u32 rollIndex, bool32 setupFirst,
                                   struct BattlePokemon *player, struct BattlePokemon *opponent)
{
    u32 starter = GetStarterPokemon(starterSlot);
    u16 moves[MAX_MON_MOVES];
    struct StarterRoll roll;
    struct Pokemon *rival = &sRivalParty[0];
    u32 stabMove, setupMove;

    GetDefaultMoves(starter, PLAYER_LEVEL, moves);
    stabMove = GetStrongestStabMove(starter, moves);
    setupMove = setupFirst ? GetSetupMove(moves) : MOVE_NONE;
    MakeStarterRoll(starter, rollIndex, &roll);

    if (!IsMeasureRun())
    {
        // Not selected by name: a trivial one-turn battle that passes, so normal make check
        // runs aren't flooded with failures or skips (empty battle tests are INVALID).
        GIVEN {
            PLAYER(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
            OPPONENT(SPECIES_WOBBUFFET) { Moves(MOVE_CELEBRATE); }
        } WHEN {
            TURN { MOVE(player, MOVE_CELEBRATE); }
        }
        return;
    }
    PASSES_RANDOMLY(0, 1);
    GIVEN {
        ASSUME(stabMove != MOVE_NONE);
        ASSUME(!setupFirst || setupMove != MOVE_NONE);
        CreateNPCTrainerPartyFromTrainer(sRivalParty, GetGameTrainer(trainerId));
        AI_FLAGS(GetGameTrainer(trainerId)->aiFlags);
        PLAYER(starter) {
            Level(PLAYER_LEVEL);
            Nature(roll.nature);
            Gender(roll.gender);
            Ability(GetSpeciesAbility(starter, roll.abilitySlot));
            HPIV(roll.ivs[STAT_HP]); AttackIV(roll.ivs[STAT_ATK]); DefenseIV(roll.ivs[STAT_DEF]);
            SpeedIV(roll.ivs[STAT_SPEED]); SpAttackIV(roll.ivs[STAT_SPATK]); SpDefenseIV(roll.ivs[STAT_SPDEF]);
            Moves(moves[0], moves[1], moves[2], moves[3]);
        }
        OPPONENT(GetMonData(rival, MON_DATA_SPECIES)) {
            Level(GetMonData(rival, MON_DATA_LEVEL));
            Nature(roll.rivalNature);
            Ability(GetMonAbility(rival));
            Gender(roll.rivalGenderRoll < gSpeciesInfo[GetMonData(rival, MON_DATA_SPECIES)].genderRatio ? MON_FEMALE : MON_MALE);
            HPIV(GetMonData(rival, MON_DATA_HP_IV)); AttackIV(GetMonData(rival, MON_DATA_ATK_IV));
            DefenseIV(GetMonData(rival, MON_DATA_DEF_IV)); SpeedIV(GetMonData(rival, MON_DATA_SPEED_IV));
            SpAttackIV(GetMonData(rival, MON_DATA_SPATK_IV)); SpDefenseIV(GetMonData(rival, MON_DATA_SPDEF_IV));
            Moves(GetMonData(rival, MON_DATA_MOVE1), GetMonData(rival, MON_DATA_MOVE2),
                  GetMonData(rival, MON_DATA_MOVE3), GetMonData(rival, MON_DATA_MOVE4));
        }
        OPPONENT(SPECIES_WOBBUFFET) { Level(100); Moves(MOVE_SPLASH); }  // filler: keeps the battle going
    } WHEN {
        for (u32 turn = 0; turn < BATTLE_TURNS; turn++)
            TURN { MOVE(player, (turn == 0 && setupMove != MOVE_NONE) ? setupMove : stabMove); }
    } THEN {
        // Win: the rival's real lead fainted and the filler is out (read the live battler,
        // party HP isn't synced from the battle).
        // One line per won battle: the runner's failure summary is capped at 50 tests
        // per process, so summarize_measure.py counts these instead.
        if (opponent->species == SPECIES_WOBBUFFET)
            DebugPrintf("MEASURE-WIN %s", gTestRunnerState.test->name);
        EXPECT_EQ(opponent->species, SPECIES_WOBBUFFET);
    }
}

// One test per (bucket, roll); ids must be unique per expansion, so they're passed in.
#define UNRIGGED_AI_SINGLE_BATTLE_TEST_ID(_id, _name) \
    static void CAT(MeasureTest, _id)(void *, const u32, struct BattlePokemon *, struct BattlePokemon *); \
    __attribute__((section(".tests"), used)) static const struct Test CAT(sMeasureTest, _id) = \
    { \
        .name = _name, \
        .filename = __FILE__, \
        .runner = &gUnriggedBattleTestRunner, \
        .sourceLine = __LINE__, \
        .data = (void *)&(const struct BattleTest) \
        { \
            .type = BATTLE_TEST_AI_SINGLES, \
            .function = { .singles = (SingleBattleTestFunction)CAT(MeasureTest, _id) }, \
            .resultsSize = 0, \
        }, \
    }; \
    static void CAT(MeasureTest, _id)(void *results, const u32 i, struct BattlePokemon *player, struct BattlePokemon *opponent)

#define MEASURE(_id, _bucket, _roll, _trainer, _slot, _setup) \
    UNRIGGED_AI_SINGLE_BATTLE_TEST_ID(_id, MEASURE_PREFIX " Route 103 " _bucket " #" #_roll) \
    { RunMeasuredRivalBattle(_trainer, _slot, _roll, _setup, player, opponent); }

#define BUCKET(_b, _name, _trainer, _slot, _setup) \
    MEASURE(_b##0, _name, 0, _trainer, _slot, _setup) MEASURE(_b##1, _name, 1, _trainer, _slot, _setup) \
    MEASURE(_b##2, _name, 2, _trainer, _slot, _setup) MEASURE(_b##3, _name, 3, _trainer, _slot, _setup) \
    MEASURE(_b##4, _name, 4, _trainer, _slot, _setup) MEASURE(_b##5, _name, 5, _trainer, _slot, _setup) \
    MEASURE(_b##6, _name, 6, _trainer, _slot, _setup) MEASURE(_b##7, _name, 7, _trainer, _slot, _setup) \
    MEASURE(_b##8, _name, 8, _trainer, _slot, _setup) MEASURE(_b##9, _name, 9, _trainer, _slot, _setup)

BUCKET(A, "boy, Axew vs May, STAB only",            TRAINER_MAY_ROUTE_103_TREECKO,     0, FALSE)
BUCKET(B, "boy, Axew vs May, setup then STAB",      TRAINER_MAY_ROUTE_103_TREECKO,     0, TRUE)
BUCKET(C, "boy, Deino vs May, STAB only",           TRAINER_MAY_ROUTE_103_TORCHIC,     1, FALSE)
BUCKET(D, "boy, Deino vs May, setup then STAB",     TRAINER_MAY_ROUTE_103_TORCHIC,     1, TRUE)
BUCKET(E, "girl, Axew vs Brendan, STAB only",       TRAINER_BRENDAN_ROUTE_103_TREECKO, 0, FALSE)
BUCKET(F, "girl, Axew vs Brendan, setup then STAB", TRAINER_BRENDAN_ROUTE_103_TREECKO, 0, TRUE)
BUCKET(G, "girl, Deino vs Brendan, STAB only",      TRAINER_BRENDAN_ROUTE_103_TORCHIC, 1, FALSE)
BUCKET(H, "girl, Deino vs Brendan, setup then STAB", TRAINER_BRENDAN_ROUTE_103_TORCHIC, 1, TRUE)
