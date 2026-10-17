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

### Measuring odds: unrigged battle tests

Upstream's battle runner rigs randomness (every move hits, no crits, secondary effects always trigger), which is right for mechanics but useless for "how often does this go our way?". `test/verdant/unrigged_battle.h` provides `UNRIGGED_AI_SINGLE_BATTLE_TEST`: same DSL, real RNG. With `PASSES_RANDOMLY(passes, trials)` (no tag) it plays the battle with 50 seeds and compares the win ratio, ±2%.

- To measure, use `PASSES_RANDOMLY(0, 1)`: the failure message reports `observed X` (50/50 shows as 0.989 due to fixed-point rounding).
- Give the opponent a Splash-only filler so the battle survives its lead fainting and every scripted TURN runs; a player loss ends the battle early and fails the run.
- Check results on the live battlers (`opponent->species`, `player->hp` in THEN), not `OPPONENT_PARTY` (party HP isn't synced from the battle).
- Parties are built once per test, so random creation details (IVs, nature, ability) are fixed per test; make one test per profile.
- Keep scripted TURNs close to the real fight length: every extra turn is filler time (16 turns ran ~3x slower than 6).
- Example guard: `test/verdant/route103_rival.c`.

Measurement suites (many rolls, report a table) follow `test/verdant/route103_rival_measure.c`: tests named `Verdant: [measure] ...`, one test per (bucket, starter roll), starter rolled per `ScriptGiveMon` rules from a fixed seed, one `MEASURE-WIN` line printed per won battle. Run and summarize:

```bash
TESTS="Verdant: [measure] Route 103" make check -j$(nproc) > /tmp/measure.log 2>&1
python3 test/verdant/summarize_measure.py /tmp/measure.log
```

Don't rely on the runner's "observed X" messages for many tests: its failure summary lists at most 50 tests per process. When not selected by name, measurement tests play a trivial one-turn battle and pass, so normal runs stay clean (an empty battle test is INVALID; ASSUME would flood the skipped list). About 60 ms per battle on 4 cores: the 4000-battle Route 103 suite takes ~4-5 minutes including the build.

### Triage

Known pre-existing failures (expansion 1.17.1, 5946 tests: 9 failed, 2 assumptions failed):

- `test/save.c`: SaveBlock1/2/3 "is backwards compatible" — expected, we added save fields.
- `test/text.c`: six item name/description width tests reporting a NULL item — our custom items count down from ID 999 (merge-safe numbering), leaving empty IDs 874-995 that these tests walk.
- `test/battle/move_effect/steal_item.c` assumptions — `B_TRAINERS_KNOCK_OFF_ITEMS` is FALSE.

A new failure is either a bug (fix it), an upstream test contradicting an intentional design change (record it here with the reason), or a test bug in our own file (fix the test).

## Tier 2: replay harness

Headless emulator replays (`tools/replay/`): libmgba plays the real ROM, unthrottled, from a scripted scenario, and checks game RAM. Roughly 30x real time (the TV broadcast test plays ~190 s of game time in ~6 s).

```bash
make                                                     # the harness tests the built ROM
tools/replay/replay.py run tools/replay/scenarios/       # every scenario, every variant
tools/replay/replay.py run tools/replay/scenarios/tv_broadcast.replay --param gender=girl --param house=MAP_LITTLEROOT_TOWN_MAYS_HOUSE_1F
tools/replay/replay.py names FLAG_SYS_TV_HOME CB2_Overworld   # what a name resolves to in this build
```

Exit code 0 = pass. Each step prints `ok` with its frame count. On failure: the failing step, a state dump (callbacks by function name, map by name, player position and facing, script/controls/fade state, battle flags, party count) and `tools/replay/build/artifacts/<scenario>/failure.png`.

Requires `libmgba-dev`, `libpng-dev` and the ARM toolchain (`apt install ...`); in cloud sessions they must be in the environment setup script.

### Layout

- `runner.c`: game-agnostic C runner (libmgba). Executes a compiled plan of input steps and RPN memory predicates.
- `replay.py`: compiles scenarios, resolves every name against the current build, runs the runner. All game knowledge lives here.
- `lib/`: reusable routines (`new_game`, `moving_in_<gender>`, `set_clock`, `new_game_to_tv`).
- `scenarios/`: the tests.

Names resolve against the build, never hardcoded: symbols (including statics) from `pokeemerald.elf` via `nm`; constants, enums, struct offsets and bitfield positions by compiling a small file with the ARM toolchain against the project headers. Upstream renumbering or struct changes don't break scenarios.

### Scenario language

One step per line, `#` comments.

| Step | Meaning |
|------|---------|
| `tap A [N]` / `hold UP N` / `wait N` | Fixed input: tap N times (default 1), hold N frames, idle N frames |
| `tap A until COND [every N] [for N] [timeout N]` | Tap every N frames (default 8) until COND has held for N frames |
| `tap A when GUARD until COND ...` | Only press while GUARD holds (e.g. `when script-running`) |
| `hold DIR until COND [timeout N]` / `wait until COND ...` | Hold a button / idle until COND |
| `walk DIR to X Y` | Hold DIR until on tile (X, Y) (map coordinates, as in Porymap), then let the step finish |
| `face DIR` | Wait for the player to stop, then turn in place |
| `expect COND` | Assert now |
| `watch NAME COND` / `expect-seen NAME` | COND must have been true on some frame since the watch started |
| `expect-control` | The player can move: tries each direction until one moves them |
| `record-battle start` / `record-battle stop` | Record a battle transcript (see below) |
| `expect-logged "TEXT"` / `expect-not-logged "TEXT"` | A battle-event line containing TEXT was / wasn't recorded |
| `expect-text "TEXT"` | An on-screen battle message containing TEXT was recorded |
| `expect-transcript golden/NAME.log` | The recording matches the reviewed golden transcript exactly |
| `state` | Print the state dump (for writing scenarios) |
| `screenshot PATH` / `export-save PATH` | Write the current frame as PNG / the game's save memory |
| `use lib/x.replay k=v` | Include a routine; `$k` substitutes parameters, `param k=v` sets defaults |
| `variant k=v ...` | Run the scenario once per variant line (unless `--param` is given) |
| `header foo.h` | Extra header for constant lookup |

Conditions combine with `and` / `or` (`and` binds tighter) and `!` / `not`:

| Condition | True when |
|-----------|-----------|
| `callback1 == SYM` / `callback2 == SYM` | Main callbacks (screens: `CB2_Overworld`, `CB2_MainMenu`, `CB2_NamingScreen`, `CB2_WallClock`, ...) |
| `task-active SYM` | Any active task runs that function (menus inside a screen) |
| `map == MAP_X` | Current map. Changes before the warp finishes; pair with `field-idle` |
| `player-at X Y`, `player-facing DIR_NORTH`, `player-stopped`, `player-moved` | Player avatar (`player-moved` is relative to the step's start) |
| `field-idle` | Overworld, no script, controls free, no fade |
| `script-running`, `controls-locked`, `fading`, `text-printing` | Script/field state. `text-printing` is false once a message finishes printing, even while it waits for A |
| `in-battle`, `battle-type has BATTLE_TYPE_X` | Battle state |
| `flag FLAG_X`, `var VAR_X == N` | Event flags and vars (also special flags and `VAR_0x8000`-style vars) |
| `party-count == N` | Party size |
| `battler player\|opponent FIELD == N` | Active battler data from `gBattleMons[]`: `species`, `hp`, `maxhp`, `ability`, `level`, `status`, `pp0`-`pp3`, `move0`-`move3` (singles) |
| `battle-outcome == B_OUTCOME_X` | Battle result (0 while the battle is running) |
| `mem8/mem16/mem32 SYM[+OFF] == N` | Escape hatch: raw memory |

### Battle transcripts

`record-battle start` (once both battlers are on the field) logs two sections until `record-battle stop`:

- **Battle events**, from battle memory: `turn N SIDE sends out SPECIES (hp N)`, `... uses MOVE`, `... hp A -> B`, `... faints`, `... ability A -> B`, `... status A -> B`, `result OUTCOME`. Move use is detected from PP deductions, so repeated moves are each logged and a move that never goes off (Truant loafing, flinching) isn't.
- **On-screen text**: every battle message, decoded from `gDisplayedStringBattle` with `charmap.txt` (health-box nickname redraws are skipped).

`expect-transcript golden/NAME.log` compares both sections against a reviewed file in `tools/replay/golden/`. A battle-event mismatch means the battle played out differently; a text-only mismatch is reported as "only the wording changed". Accept an intentional change with `--update-golden`, then review the diff before committing it. Pair the golden file with a few `expect-logged` / `battler` checks for the headline moments, so a failure says what broke.

### Writing scenarios: lessons so far

- Wait for a specific screen or task before acting, not for "enough frames". Blind A at the naming screen types letters; blind A at the wall clock answers NO (its prompt starts on NO) and loops forever.
- Input is dropped during fades and while a step is finishing: `wait until !fading`, `face` (waits for the player to stop).
- Advance cutscenes with `tap A when script-running until field-idle for 30`: scripts ignore A except at their input waits, tapping stops when the script ends (no accidental interactions), and `for 30` keeps "not started yet" from passing as "finished".
- Explore with `state` and `screenshot` steps, then replace them with conditions.

### Principles

- Start from boot or from an in-game save, never from a save state (save states snapshot build-specific RAM and break on rebuilds).
- Prefer memory checks; keep screenshot checks rare (any visual change breaks them).
- Same ROM + same save + same inputs is deterministic (RNG advances per frame).
