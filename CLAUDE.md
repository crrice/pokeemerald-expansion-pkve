# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Pokemon Verdant Emerald is a ROM hack based on pokeemerald-expansion (from RH-Hideout), which itself is a decompilation of Pokemon Emerald. The project targets GBA hardware and produces a `pokeemerald.gba` ROM file.

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

This hack uses Flying/Fighting/Rock as the starter type triangle:
- Pikipek line (Flying)
- Machop line (Fighting)
- Roggenrola line (Rock)

## Important Conventions

- Graphics are PNG files converted to GBA formats (`.4bpp`, `.gbapal`, etc.) at build time
- String encoding uses `charmap.txt` for character mappings
- Assembly files use custom preprocessor macros extensively
- Map data is generated from JSON via mapjson tool (Porymap-compatible)
