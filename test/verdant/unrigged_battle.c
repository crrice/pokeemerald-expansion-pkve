#include "global.h"
#include "test/battle.h"
#include "unrigged_battle.h"

// See unrigged_battle.h. Everything forwards to upstream's battle runner;
// only the random hooks are left empty (= real RNG).

static u32 Unrigged_EstimateCost(void *data)
{
    return gBattleTestRunner.estimateCost(data);
}

static void Unrigged_SetUp(void *data)
{
    gBattleTestRunner.setUp(data);
}

static void Unrigged_Run(void *data)
{
    gBattleTestRunner.run(data);
}

static void Unrigged_TearDown(void *data)
{
    gBattleTestRunner.tearDown(data);
}

static bool32 Unrigged_CheckProgress(void *data)
{
    return gBattleTestRunner.checkProgress(data);
}

static bool32 Unrigged_HandleExitWithResult(void *data, enum TestResult result)
{
    return gBattleTestRunner.handleExitWithResult(data, result);
}

const struct TestRunner gUnriggedBattleTestRunner =
{
    .estimateCost = Unrigged_EstimateCost,
    .setUp = Unrigged_SetUp,
    .run = Unrigged_Run,
    .tearDown = Unrigged_TearDown,
    .checkProgress = Unrigged_CheckProgress,
    .handleExitWithResult = Unrigged_HandleExitWithResult,
    .randomUniform = NULL,
    .randomWeightedArray = NULL,
    .randomElementArray = NULL,
};
