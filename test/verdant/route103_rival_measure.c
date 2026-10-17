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
// Informational only: there is no target rate, and nothing here should block a change.
// Each test is one battle: a won battle passes and prints a MEASURE-WIN line, a lost
// one fails (the summarizer counts both). The tests run on gMeasureBattleTestRunner:
// in normal runs (not selected by name) they pass without playing.
//
// Axes: player gender (picks the rival: May or Brendan) x the player's starter, each
// with and without one buff/debuff move on turn 1 (Axew: Leer, Deino: Focus Energy),
// then strongest STAB every turn. 8 buckets x 500 battles = 4000 battles.
//
// Everything else is rolled per battle, as the game would per save:
// - the player's starter, per ScriptGiveMon (pure random, no IV floor): IVs 0-31 each,
//   nature, gender by the species ratio, ability slot 0 or 1;
// - the rival's starter: level, IVs (0), ability and moves come from the trainer data;
//   nature and gender aren't in it, so the game rolls them (gender by the species ratio);
// - the battle's RNG seed.
// A party is built once per test (PASSES_RANDOMLY would replay the same party 50 times),
// hence one battle per test. Roll N is the same in every bucket, so buckets differ only
// in their axes. Rolls come from a fixed seed per roll number: runs are reproducible.

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

static rng_value_t BattleSeed(u32 rollIndex)
{
    rng_value_t seed = {0, 0, 0x5EED + rollIndex, 1};
    for (u32 i = 0; i < 16; i++)
        _SFC32_Next(&seed);
    return seed;
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
    u32 stabMove, setupMove, rivalGender = MON_MALE;

    GetDefaultMoves(starter, PLAYER_LEVEL, moves);
    stabMove = GetStrongestStabMove(starter, moves);
    setupMove = setupFirst ? GetSetupMove(moves) : MOVE_NONE;
    MakeStarterRoll(starter, rollIndex, &roll);

    GIVEN {
        RNGSeed(BattleSeed(rollIndex));
        ASSUME(stabMove != MOVE_NONE);
        ASSUME(!setupFirst || setupMove != MOVE_NONE);
        CreateNPCTrainerPartyFromTrainer(sRivalParty, GetGameTrainer(trainerId));
        rivalGender = roll.rivalGenderRoll < gSpeciesInfo[GetMonData(rival, MON_DATA_SPECIES)].genderRatio ? MON_FEMALE : MON_MALE;
        DebugPrintf("MEASURE-ROLL %s|%d|%d", gTestRunnerState.test->name, roll.gender, rivalGender);
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
            Gender(rivalGender);
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
        // party HP isn't synced from the battle). A loss ends the battle early and fails.
        // summarize_measure.py counts MEASURE-WIN lines: the runner's failure summary is
        // capped at 50 tests per process.
        if (opponent->species == SPECIES_WOBBUFFET)
            DebugPrintf("MEASURE-WIN %s", gTestRunnerState.test->name);
        EXPECT_EQ(opponent->species, SPECIES_WOBBUFFET);
    }
}

// One test per battle; ids must be unique per expansion, so they're passed in.
#define UNRIGGED_AI_SINGLE_BATTLE_TEST_ID(_id, _name) \
    static void CAT(MeasureTest, _id)(void *, const u32, struct BattlePokemon *, struct BattlePokemon *); \
    __attribute__((section(".tests"), used)) static const struct Test CAT(sMeasureTest, _id) = \
    { \
        .name = _name, \
        .filename = __FILE__, \
        .runner = &gMeasureBattleTestRunner, \
        .sourceLine = __LINE__, \
        .data = (void *)&(const struct BattleTest) \
        { \
            .type = BATTLE_TEST_AI_SINGLES, \
            .function = { .singles = (SingleBattleTestFunction)CAT(MeasureTest, _id) }, \
            .resultsSize = 0, \
        }, \
    }; \
    static void CAT(MeasureTest, _id)(void *results, const u32 i, struct BattlePokemon *player, struct BattlePokemon *opponent)

// Roll number _h_t_u (three digits; the value is computed, the name keeps the digits).
#define MEASURE(_id, _bucket, _h, _t, _u, _trainer, _slot, _setup) \
    UNRIGGED_AI_SINGLE_BATTLE_TEST_ID(_id, MEASURE_TEST_PREFIX " Route 103 " _bucket " #" #_h #_t #_u) \
    { RunMeasuredRivalBattle(_trainer, _slot, (_h) * 100 + (_t) * 10 + (_u), _setup, player, opponent); }

#define MEASURE_10(_b, _bucket, _h, _t, ...) \
    MEASURE(_b##_h##_t##0, _bucket, _h, _t, 0, __VA_ARGS__) MEASURE(_b##_h##_t##1, _bucket, _h, _t, 1, __VA_ARGS__) \
    MEASURE(_b##_h##_t##2, _bucket, _h, _t, 2, __VA_ARGS__) MEASURE(_b##_h##_t##3, _bucket, _h, _t, 3, __VA_ARGS__) \
    MEASURE(_b##_h##_t##4, _bucket, _h, _t, 4, __VA_ARGS__) MEASURE(_b##_h##_t##5, _bucket, _h, _t, 5, __VA_ARGS__) \
    MEASURE(_b##_h##_t##6, _bucket, _h, _t, 6, __VA_ARGS__) MEASURE(_b##_h##_t##7, _bucket, _h, _t, 7, __VA_ARGS__) \
    MEASURE(_b##_h##_t##8, _bucket, _h, _t, 8, __VA_ARGS__) MEASURE(_b##_h##_t##9, _bucket, _h, _t, 9, __VA_ARGS__)

#define MEASURE_100(_b, _bucket, _h, ...) \
    MEASURE_10(_b, _bucket, _h, 0, __VA_ARGS__) MEASURE_10(_b, _bucket, _h, 1, __VA_ARGS__) \
    MEASURE_10(_b, _bucket, _h, 2, __VA_ARGS__) MEASURE_10(_b, _bucket, _h, 3, __VA_ARGS__) \
    MEASURE_10(_b, _bucket, _h, 4, __VA_ARGS__) MEASURE_10(_b, _bucket, _h, 5, __VA_ARGS__) \
    MEASURE_10(_b, _bucket, _h, 6, __VA_ARGS__) MEASURE_10(_b, _bucket, _h, 7, __VA_ARGS__) \
    MEASURE_10(_b, _bucket, _h, 8, __VA_ARGS__) MEASURE_10(_b, _bucket, _h, 9, __VA_ARGS__)

// 500 battles per bucket.
#define BUCKET(_b, _bucket, _trainer, _slot, _setup) \
    MEASURE_100(_b, _bucket, 0, _trainer, _slot, _setup) MEASURE_100(_b, _bucket, 1, _trainer, _slot, _setup) \
    MEASURE_100(_b, _bucket, 2, _trainer, _slot, _setup) MEASURE_100(_b, _bucket, 3, _trainer, _slot, _setup) \
    MEASURE_100(_b, _bucket, 4, _trainer, _slot, _setup)

BUCKET(A, "boy, Axew vs May, STAB only",            TRAINER_MAY_ROUTE_103_TREECKO,     0, FALSE)
BUCKET(B, "boy, Axew vs May, setup then STAB",      TRAINER_MAY_ROUTE_103_TREECKO,     0, TRUE)
BUCKET(C, "boy, Deino vs May, STAB only",           TRAINER_MAY_ROUTE_103_TORCHIC,     1, FALSE)
BUCKET(D, "boy, Deino vs May, setup then STAB",     TRAINER_MAY_ROUTE_103_TORCHIC,     1, TRUE)
BUCKET(E, "girl, Axew vs Brendan, STAB only",       TRAINER_BRENDAN_ROUTE_103_TREECKO, 0, FALSE)
BUCKET(F, "girl, Axew vs Brendan, setup then STAB", TRAINER_BRENDAN_ROUTE_103_TREECKO, 0, TRUE)
BUCKET(G, "girl, Deino vs Brendan, STAB only",      TRAINER_BRENDAN_ROUTE_103_TORCHIC, 1, FALSE)
BUCKET(H, "girl, Deino vs Brendan, setup then STAB", TRAINER_BRENDAN_ROUTE_103_TORCHIC, 1, TRUE)
