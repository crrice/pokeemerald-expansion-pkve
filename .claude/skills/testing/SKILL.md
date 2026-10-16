---
name: testing
description: Writing and running automated tests for Verdant Emerald - the in-ROM test runner (make check, battle/script/data tests) and the headless emulator replay harness. Use when adding tests for a feature, checking design values, running a subset of tests, or triaging test failures after a change or upstream merge.
---

# Testing

Two tiers. Prefer Tier 1; use Tier 2 only for flows Tier 1 can't reach.

| Tier | What it is | Good for | Can't do |
|------|------------|----------|----------|
| 1. In-ROM test runner | C tests compiled into a test ROM, run headless by `mgba-rom-test` via `make check` | Data/config values, battle behavior, script logic, flags/vars | Anything that waits on a textbox, movement, fade or player input |
| 2. Replay harness | Headless emulator (libmgba) plays the real ROM with scripted button input, then checks RAM | End-to-end flows: cutscenes, TV replay, regaining control | Fast feedback (slower, more brittle); not in `make check` |

Upstream's generic guide to the Tier 1 API: `docs/tutorials/how_to_testing_system.md`. This skill covers our conventions; don't duplicate upstream docs here.

## What to test

Every feature commit should pin its **design values** so later edits and upstream merges can't silently undo them. A test like the one below would have caught the starter learnsets sitting in the wrong file for months, and the Route 104 tables that were designed but never implemented.

- Data: learnsets, types, evolution levels, base stats of reworked species, encounter tables we designed, item data.
- Config: settings the design depends on (e.g. `B_OVERWORLD_WEATHER_OVERRIDE`, `OW_TIME_OF_DAY_ENCOUNTERS`).
- Logic: story flag/var transitions we own (Rubicon, gym gates, quest states).
- Battles: scripted battle setups, custom items/abilities, mechanics we changed.

Design docs state intent; tests pin the values (see `CLAUDE.md` → How We Track Work).

## Tier 1: in-ROM tests

### Where our tests live

- Put project tests in `test/verdant/`, one file per feature (`starters.c`, `weather.c`, `quests.c`, ...). The Makefile picks up `test/*.c`, `test/*/*.c` and `test/*/*/*.c` automatically.
- Prefix every test name with `Verdant:` so ours can be run alone.
- Don't edit upstream test files to fit our changes. If an upstream test fails because of an intentional design change, record it (see Triage) instead of rewriting the test, so merges stay clean.

### Running

```bash
make check -j$(nproc) > /tmp/check.log 2>&1; tail -20 /tmp/check.log    # everything
TESTS="Verdant:" make check > /tmp/check.log 2>&1                       # name prefix
TESTS="*Metal Claw" make check > /tmp/check.log 2>&1                    # name contains (leading *)
TESTS="test/verdant/starters.c" make check > /tmp/check.log 2>&1        # exact file
```

Redirect to a file and read it afterwards; piping `make` gives misleading results (see `CLAUDE.md`). The summary at the end lists FAILED, KNOWN_FAILING, ASSUMPTIONS_FAILED, TO_DO and PASSED counts.

### Writing

Plain tests (`#include "test/test.h"`):

```c
#include "global.h"
#include "test/test.h"

TEST("Verdant: Axew learns Metal Claw at level 6")
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(SPECIES_AXEW);
    bool32 found = FALSE;
    for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].move == MOVE_METAL_CLAW && learnset[i].level == 6)
            found = TRUE;
    }
    EXPECT(found);
}
```

- Assertions: `EXPECT`, `EXPECT_EQ/NE/LT/LE/GT/GE`.
- `ASSUME(cond)` skips a test (reported as ASSUMPTIONS_FAILED) when a precondition doesn't hold, e.g. a config is off. An `ASSUMPTIONS { ... }` block applies to the whole file.
- `PARAMETRIZE { ... }` runs the test once per block, e.g. once per starter.
- `KNOWN_FAILING;` marks a test as expected to fail; `TO_DO;` marks it unwritten.

Script logic (`#include "test/overworld_script.h"`): `RUN_OVERWORLD_SCRIPT( setflag FLAG_X; givemon SPECIES_AXEW, 5; );` runs script commands immediately, then assert on `FlagGet`, `VarGet`, `gPlayerParty`. Only non-blocking commands work: no `msgbox`, `applymovement`, `waitmovement` or fades. Factor the logic you want to test into non-blocking parts, or use Tier 2.

Battles (`#include "test/battle.h"`): `SINGLE_BATTLE_TEST` / `DOUBLE_BATTLE_TEST` / `AI_SINGLE_BATTLE_TEST` with `GIVEN { PLAYER(...); OPPONENT(...); } WHEN { TURN { MOVE(player, ...); } } SCENE { MESSAGE("..."); ANIMATION(...); HP_BAR(...); }`. Copy the shape from a nearby file in `test/battle/`; upstream's guide documents the full DSL.

### Triage

Known pre-existing failures (baseline before the expansion 1.17 upgrade, 4285 tests):

- `test/save.c`: SaveBlock1/2/3 "is backwards compatible" — expected, we added save fields.
- `test/text.c`: six item name/description width tests reporting a NULL item — our custom items count down from ID 999 (merge-safe numbering), leaving empty IDs 855-995 that these tests walk.
- `test/battle/move_effect_secondary/steal_item.c` assumptions — `B_TRAINERS_KNOCK_OFF_ITEMS` is FALSE.

A new failure is either a bug (fix it), an upstream test contradicting an intentional design change (record it here with the reason), or a test bug in our own file (fix the test).

## Tier 2: replay harness

Headless emulator replays using libmgba (mGBA's core as a C library; no window or virtual display needed). Status: being built, see `BACKLOG.md` (Pre-Demo → Polish → automated tests). This section gets the usage once it lands.

Principles:
- Start from boot or from an in-game save (`.sav`) generated by a script, never from a save state (save states snapshot build-specific RAM and break on rebuilds).
- Wait on conditions ("tap A until the battle starts", with a timeout), not fixed frame counts, so timing changes don't break replays.
- Check results by reading RAM through symbols from `pokeemerald.elf` (callbacks, flags, vars, player position), not screenshots.
- Same ROM + same save + same inputs is deterministic (RNG advances per frame).

Requires `libmgba-dev` (`apt install libmgba-dev`); in cloud sessions it must be in the environment setup script, along with the ARM toolchain.
