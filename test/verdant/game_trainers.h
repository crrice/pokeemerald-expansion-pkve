#ifndef GUARD_TEST_VERDANT_GAME_TRAINERS_H
#define GUARD_TEST_VERDANT_GAME_TRAINERS_H

// The game's own trainer data, for tests about our trainers.
//
// Test builds replace gTrainers with upstream's test-only trainers
// (test/test_runner_battle.c, since expansion 1.16), so GetTrainerStructFromId
// can't see the real rivals or gym leaders there. This compiles a second copy of
// the real table (src/data/trainers.h, generated from trainers.party).

const struct Trainer *GetGameTrainer(u32 trainerId);

#endif // GUARD_TEST_VERDANT_GAME_TRAINERS_H
