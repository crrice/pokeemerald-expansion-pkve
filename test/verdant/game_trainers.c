#include "global.h"
#include "battle.h"
#include "data.h"
#include "graphics.h"
#include "trainer_pools.h"
#include "battle_transition.h"
#include "constants/abilities.h"
#include "constants/items.h"
#include "constants/moves.h"
#include "constants/trainers.h"
#include "constants/battle_ai.h"
#include "game_trainers.h"

// Same includes and table as src/data.c, which skips it when TESTING.
static const struct Trainer sGameTrainers[DIFFICULTY_COUNT][TRAINERS_COUNT] =
{
#include "../../src/data/trainers.h"
};

const struct Trainer *GetGameTrainer(u32 trainerId)
{
    return &sGameTrainers[DIFFICULTY_NORMAL][trainerId];
}
