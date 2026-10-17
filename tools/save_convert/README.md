# Save conversion

Tools for keeping dev saves working across save format changes. Python 3; not part of the ROM build.

| Script | What it does |
|--------|--------------|
| `expansion_1_17.py OLD.sav NEW.sav` | Converts a save from a build before the expansion 1.17.1 upgrade (master before `52907a127a`). Standard library only. |
| `compare_builds.py OLD_TREE NEW_TREE` | Measures what changed in the save format between two built source trees: field layout of every save block (from debug info) and every ID a save stores. Needs `pip install pyelftools`. |

## Converting a save to expansion 1.17.1

```bash
python3 tools/save_convert/expansion_1_17.py "Verdant Emerald.sav" converted.sav
```

- Keep the original: the script never modifies it, refuses to overwrite the output, and refuses a save that's already converted.
- It prints a before/after report per save slot (player name, party, boxes, Pokédex counts, quest data, Wally's Ralts, checksums of SaveBlock2 and the PC). Every line should say `ok`; if not, nothing is written.
- Put `converted.sav` where your emulator expects the save (for mGBA: next to the ROM, same base name).
- Works with 128 KB flash saves, with or without the 16-byte RTC block mGBA appends.

What it changes, from `compare_builds.py` on the two trees:

- SaveBlock1: the Pokédex seen/caught flags grew (National Dex 905 → 1019), so the fields after them moved 28 bytes. 4 unused bytes became `dailySeed` (set at random), and Mass Outbreak TV shows changed layout (taken off the air).
- The 16 dynamic overworld sprite IDs `OBJ_EVENT_GFX_VAR_0..F` moved from 244-259 to 389-404.
- Unchanged: SaveBlock2 (quests), SaveBlock3 (Wally's Ralts), the PC, the Pokémon format, and every item, move, species, flag, var, map and trainer ID. `SPECIES_EGG` moved, but eggs are stored as their real species with an egg flag.

How it was tested:

- Mapping: old saves filled with random data, converted, and every SaveBlock1 field compared by name against both builds' debug layouts (20 runs).
- End to end: the old ROM played a new game to the TV broadcast and saved in-game (replay harness, both genders); after conversion the 1.17.1 ROM continues it on the same map and tile, with flags and vars intact and the player in control.

## Next time the format changes

1. Build the old and new trees, run `compare_builds.py`.
2. Write a converter for what it lists (copy `expansion_1_17.py`: the sector reading/writing and report carry over; the SaveBlock1 mapping is the part that changes).
3. Test it the same two ways. The replay harness can make old-format saves (`lib/save_game.replay`, `export-save`) and load converted ones (`--save`, `lib/continue.replay`).
