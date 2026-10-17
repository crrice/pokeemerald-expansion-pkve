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

// Measurement tests ("Verdant: [measure] ..."): informational battles that only run when
// selected by name (TESTS="Verdant: [measure]..."). In every other run they pass at once,
// so normal make check runs don't pay for thousands of battles.
static bool32 IsMeasureRun(void)
{
    return strncmp(gTestRunnerArgv, MEASURE_TEST_PREFIX, sizeof(MEASURE_TEST_PREFIX) - 1) == 0;
}

static u32 Measure_EstimateCost(void *data)
{
    return IsMeasureRun() ? Unrigged_EstimateCost(data) : 1;
}

static void Measure_SetUp(void *data)
{
    if (IsMeasureRun())
        Unrigged_SetUp(data);
}

static void Measure_Run(void *data)
{
    if (IsMeasureRun())
        Unrigged_Run(data);
}

static void Measure_TearDown(void *data)
{
    if (IsMeasureRun())
        Unrigged_TearDown(data);
}

static bool32 Measure_CheckProgress(void *data)
{
    return IsMeasureRun() && Unrigged_CheckProgress(data);
}

static bool32 Measure_HandleExitWithResult(void *data, enum TestResult result)
{
    return IsMeasureRun() && Unrigged_HandleExitWithResult(data, result);
}

const struct TestRunner gMeasureBattleTestRunner =
{
    .estimateCost = Measure_EstimateCost,
    .setUp = Measure_SetUp,
    .run = Measure_Run,
    .tearDown = Measure_TearDown,
    .checkProgress = Measure_CheckProgress,
    .handleExitWithResult = Measure_HandleExitWithResult,
    .randomUniform = NULL,
    .randomWeightedArray = NULL,
    .randomElementArray = NULL,
};
