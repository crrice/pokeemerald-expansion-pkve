# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Pokemon Verdant Emerald is a ROM hack based on pokeemerald-expansion (from RH-Hideout), which itself is a decompilation of Pokemon Emerald. The project targets GBA hardware and produces a `pokeemerald.gba` ROM file.

## Design Documentation

Project-specific docs (not from upstream pokeemerald-expansion):

| File | Purpose |
|------|---------|
| `DIVERGENCE_THEORY.md` | Core design document. Defines the dual-path system: Hero route (traditional) vs Anti-hero route (skip Roxanne via Rusturf Tunnel). Covers gym order, story divergence, Devon Goods consequences, Roxanne as secret final boss, NG+ system. **Read this first for project vision.** |
| `ROADMAP.md` | Gameplay design by act. Currently covers Act 1 (Gyms 1-2): starter balance, weather introduction, rival fair-play doctrine. Will expand as development progresses. |
| `BALANCE.md` | Encounter tables, per-Pokemon balance notes, and gameplay pillars (weather, berries, doubles, etc.). Reference when working on specific routes or Pokemon. |
| `BACKLOG.md` | Task tracking, grouped by milestone (Pre-Demo, Demo 1, Later) with maturity tags. "Done" is a one-line inventory of what exists. Check here for current implementation status. |
| `ART_ASSETS.md` | Wishlist of custom art the project needs (brief for a future sprite artist): what, where used, format constraints, placeholder. |
| `WALKTHROUGH.md` | Player-facing walkthrough and FAQ for this hack. |
| `JOURNAL.md` | Dev journal / scratch pad. Stream-of-consciousness design thoughts. Not structured, but captures rationale for early decisions (starter selection, evolution changes, etc.). |

## Build Commands

```bash
# Standard build (produces pokeemerald.gba)
make

# Parallel build (faster - replace N with core count from `nproc`)
make -jN

# Build with debug symbols
make debug

# Run test suite (requires mgba-rom-test)
make check

# Build test ROM only (without running tests)
make pokeemerald-test.elf

# Clean build artifacts
make clean        # Full clean including tools
make tidy         # Clean ROM/ELF only, keep tools
```

**Build gotcha:** Incremental builds do not reliably pick up changes to `.s` (assembly) files. Always run `make tidy` before `make` after editing any `.s` file. If a code change has zero observable effect in-game, suspect a stale build before investigating further.

**Build command pattern:** When piping build output (e.g. to `tail`), the pipe does not wait for `make` to finish, giving misleading results. Always redirect to a file first, then read it:
```bash
make tidy && make -j$(nproc) > /tmp/build.log 2>&1 && tail -5 /tmp/build.log
```
Prefer `make tidy` before building to avoid stale artifact issues.

## Testing

After building, copy the ROM to your Windows Roms folder for testing. In WSL, this is typically:
```bash
cp pokeemerald.gba /mnt/c/Users/<your-windows-username>/Documents/Roms/
```
Ask the user for their specific path if not known.

## Running Tests

The project uses a custom test runner with mgba-rom-test. Tests are in `test/` and organized by feature:
- `test/battle/` - Battle mechanics tests (abilities, moves, items, etc.)
- Individual test files for Pokemon, daycare, text, etc.

To run a specific test, you need to rebuild with the test filter:
```bash
TESTS="test_name_pattern" make check
```

## Architecture

### Core Directories

- `src/` - C source files (battle system, overworld, Pokemon data, UI)
- `include/` - Header files and configuration
- `include/config/` - Feature toggles and generation settings (battle.h, pokemon.h, etc.)
- `data/` - Assembly data files, maps, scripts, tilesets
- `asm/` - Assembly source (mostly macros)
- `graphics/` - PNG/palette assets
- `sound/` - Audio files and MIDI

### Battle System

The battle engine uses a controller pattern where each battler (player, opponent, link, etc.) has a controller that handles `CONTROLLER_*` commands. Key files:
- `src/battle_main.c` - Main battle loop
- `src/battle_script_commands.c` - Battle script interpreter
- `src/battle_controller_*.c` - Per-battler-type controllers
- `data/battle_scripts_1.s` - Battle script definitions
- `asm/macros/battle_script.inc` - Battle script macros

### Event Scripting

Map events use a custom scripting language compiled from `.inc` files:
- `data/maps/*/scripts.inc` - Per-map event scripts
- `data/scripts/` - Shared script includes
- Use `callnative FunctionName` to call C functions from scripts

### Configuration System

Generation-specific behaviors are controlled by `GEN_LATEST` or specific `GEN_N` values in `include/config/`:
- `battle.h` - Damage calc, exp gain, type matchups
- `pokemon.h` - Stats, abilities, evolution methods
- `item.h` - Item behaviors
- `species_enabled.h` - Which Pokemon are available

### Trainer Data

Trainers are defined using `.party` files processed by trainerproc:
- `src/data/trainers.party` - Trainer party definitions
- Supports custom IVs, EVs, moves, abilities, held items

## Project-Specific Features

### Custom Starters

Two starters, a pair rather than a trio: **Axew** (Dragon/Steel) and **Deino** (Dark/Dragon). Picked in `src/starter_choose.c` (`STARTER_MON_COUNT` 2).

Upstream's three-slot names are kept (renaming them would cause conflicts in every upstream merge). Slots are named after the *player's* pick:

| Slot name | `VAR_STARTER_MON` | Meaning | Rival uses |
|-----------|-------------------|---------|------------|
| `TREECKO` | 0 | Player picked Axew | Deino |
| `TORCHIC` | 1 | Player picked Deino | Axew |
| `MUDKIP` | 2 | Dead, never set | - |

Dead-slot script branches can be deleted, but keep the `*_MUDKIP` trainer entries defined: removing them shifts the trainer IDs after them.

## How We Track Work

Each kind of information has exactly one home:

| Level | Where it lives |
|-------|----------------|
| Principle / design goal | `DIVERGENCE_THEORY.md`, principles in `ROADMAP.md`, pillars in `BALANCE.md` |
| Vague plan, open design question | Open-question sections of the design docs, or `JOURNAL.md` |
| Idea needing breakdown | `BACKLOG.md`, tagged `[idea]` |
| Needs refinement (questions open) | `BACKLOG.md`, tagged `[refine]`, open questions listed on the item |
| Ready to work | `BACKLOG.md`, tagged `[ready]`: one-line goal, how we know it's done, files involved, no open questions |

`(you: design call)` / `(you: playtest)` mark items waiting on the project owner.

Rules:
- Update the backlog in the same commit as the work: check items off, add follow-ups, move finished work to Done.
- Design docs state intent ("Axew gets Steel STAB before Roxanne"); code holds the values and tests pin them. Don't restate code values in docs.
- Decisions made in conversation go into the relevant design doc in the repo, not only chat or local memory. Cloud sessions can't see local memory.
- Encounter tables are designed in `BALANCE.md` first, then implemented.
- New features come with tests that pin their design values. How to write and run them: the `testing` skill (`.claude/skills/testing/SKILL.md`).
- Art blockers go to `ART_ASSETS.md`.
- At the start of a session, compare the current milestone against `git log` since `BACKLOG.md` last changed and flag drift.
- Trim Done at each milestone; git history is the permanent record.

## Important Conventions

- Graphics are PNG files converted to GBA formats (`.4bpp`, `.gbapal`, etc.) at build time
- String encoding uses `charmap.txt` for character mappings
- Assembly files use custom preprocessor macros extensively
- Map data is generated from JSON via mapjson tool (Porymap-compatible)
