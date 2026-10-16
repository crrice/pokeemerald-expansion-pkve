#ifndef GUARD_TEST_VERDANT_UNRIGGED_BATTLE_H
#define GUARD_TEST_VERDANT_UNRIGGED_BATTLE_H

#include "test/battle.h"

// Battle tests with the game's real RNG.
//
// Upstream's battle test runner rigs randomness by default: every move hits,
// critical hits never happen, secondary effects always trigger. That's right for
// testing mechanics, but useless for "how often does this battle go our way?".
//
// This runner forwards everything to gBattleTestRunner except the random hooks,
// which it leaves empty so RandomUniform/RandomWeighted/RandomElement fall through
// to the real RNG. Combine with PASSES_RANDOMLY(passes, trials) (no tag) to play
// the same battle with 50 different seeds and measure a win ratio.
//
// Kept in our own files so upstream merges never touch it.

extern const struct TestRunner gUnriggedBattleTestRunner;

// Same shape as upstream's BATTLE_TEST_ARGS_SINGLE, with our runner.
#define UNRIGGED_AI_SINGLE_BATTLE_TEST(_name, ...) \
    struct CAT(Result, __LINE__) { RECURSIVELY(R_FOR_EACH(APPEND_SEMICOLON, __VA_ARGS__)) }; \
    static void CAT(Test, __LINE__)(struct CAT(Result, __LINE__) *, const u32, struct BattlePokemon *, struct BattlePokemon *); \
    __attribute__((section(".tests"), used)) static const struct Test CAT(sTest, __LINE__) = \
    { \
        .name = _name, \
        .filename = __FILE__, \
        .runner = &gUnriggedBattleTestRunner, \
        .sourceLine = __LINE__, \
        .data = (void *)&(const struct BattleTest) \
        { \
            .type = BATTLE_TEST_AI_SINGLES, \
            .function = { .singles = (SingleBattleTestFunction)CAT(Test, __LINE__) }, \
            .resultsSize = sizeof(struct CAT(Result, __LINE__)), \
        }, \
    }; \
    static void CAT(Test, __LINE__)(struct CAT(Result, __LINE__) *results, const u32 i, struct BattlePokemon *player, struct BattlePokemon *opponent)

#endif // GUARD_TEST_VERDANT_UNRIGGED_BATTLE_H
