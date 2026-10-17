---
name: upstream-merge
description: Merging a new pokeemerald-expansion release into Verdant Emerald. Use when upgrading upstream, resolving conflicts against upstream code, resuming an interrupted upgrade, or checking that an upgrade lost none of our customizations.
---

# Upstream Merges

How we bring in new pokeemerald-expansion releases. The ranked list of practices that keep merges cheap came out of the 1.14 → 1.17.1 upgrade (59 conflicts, 22 breakages in our code): `hygiene-1.17.md`, with the evidence for each item. This skill is the procedure.

## Setup (every new container)

Containers are fresh clones, so local git config and the conflict-resolution cache don't survive. Before any merge:

```bash
git config rerere.enabled true       # remember conflict resolutions; replays them on a redo
git config rerere.autoUpdate true    # stage replayed resolutions
git config merge.conflictStyle zdiff3  # show the base: most conflicts are upstream rewording a comment next to a value we changed
```

Upstream release tags live as local refs `refs/upstream/<version>`:

```bash
git fetch --no-tags https://github.com/rh-hideout/pokeemerald-expansion \
    refs/tags/expansion/1.18.0:refs/upstream/1.18.0 refs/tags/expansion/1.18.2:refs/upstream/1.18.2
```

## Procedure

1. Branch `upgrade-<target>` from current master, in its own worktree: `git worktree add ../pve-upgrade -b upgrade-<target> master`.
2. Merge in stages: the latest patch of each minor in turn (1.14.4 → 1.15.3 → 1.16.4 → 1.17.1). Each stage: merge, run that range's `migration_scripts/`, build, `make check`, audit, push.
3. Changelogs come from the tag, before the merge: `git show refs/upstream/<tag>:docs/changelogs/<minor>.x/<version>.md`.
4. Big upgrades run as the `upstream-merge` workflow (`.claude/workflows/upstream-merge.js`), ideally from a separate cloud session: interrupting the conversation that runs a workflow kills it.
5. Save format: build the old and new trees and run `tools/save_convert/compare_builds.py OLD NEW`. If anything moved, write a converter for the owner's dev saves (`tools/save_convert/README.md`).
6. Compare ROMs only from clean builds (`make clean`): the source tree caches generated files (audio samples) that `make tidy` keeps.
7. When every stage is green, the owner reviews, then master gets a `Merge upgrade-<target> into master.` commit.

Commit style is the usual (`commit-style` skill). Merge commits are titled `Merge pokeemerald-expansion <version>.` (see `d987fb4465` for the body: preserved features, notable upstream changes). Migrations and build fixes are separate commits after the merge commit.

## Resuming an interrupted merge

Never redo a merge by hand that was already done once. Before `git merge refs/upstream/<tag>`:

```bash
tag=$(git rev-parse refs/upstream/<tag>)
git log --merges --format='%H %P' upgrade-<target> wip/<tag>-interrupted 2>/dev/null | awk -v t=$tag '$3 == t'
```

- Found on the upgrade branch: the merge is done; continue from migrations/build.
- Found only on `wip/<tag>-interrupted`: fast-forward to it (`git merge --ff-only wip/<tag>-interrupted`), rebuild, continue.
- Interrupted mid-resolution (merge in progress): run `git rerere` to record the hunks already resolved, then `git merge --abort`. Redoing the merge replays them.
- Interrupted after the merge commit: commit the work in progress and push it as `wip/<tag>-interrupted` before resetting anything.

## Resolution rules

- Take upstream's structure and API, then re-apply ours on top. Never drop a customization.
- Comment-only, whitespace-only and ordering-only text: always take upstream's version.
- Data files (wild_encounters.json, map_groups.json, layouts.json, trainers.party, items, species): keep both sides' entries.
- New upstream config options: take upstream's default, and list any that change gameplay for the owner (catch rates, weather, Synchronize, ...).
- When a release did a whitespace or formatting sweep, merge with `-Xignore-space-change`, then take upstream's whitespace for those hunks in a separate commit.
- Stage resolutions by path or `git add -u`, never `git add -A`.
- Adapt our code to upstream's new API rather than editing upstream code to fit ours.

### Before the merge commit: no leftovers, no generated files

Run on the staged merge; each command must print what the comment says:

```bash
git grep -nE '^(<<<<<<<|>>>>>>>) '            # nothing: no conflict markers
git ls-files -ci --exclude-standard            # only upstream's own: tools/mgba/mgba-rom-test.exe, tools/compresSmol/*
git diff --cached --name-only --diff-filter=d refs/upstream/<tag> -- . ':!.claude' \
    | xargs -d '\n' grep -l -I -E 'DO NOT MODIFY|auto-generated'    # nothing: no generated file differs from upstream
ls *.backup *.orig *.rej 2>/dev/null; git ls-files '*.backup' '*.orig' '*.rej'   # nothing
```

A build output or generated file in that list gets `git rm --cached` (it stays ignored). Past cases: the generated `include/constants/script_commands.h` committed in the 1.14.1 merge (a modify/delete conflict once upstream untracked it), the `tools/aif2pcm/aif2pcm` binary, and `.backup` files in the repo root.

## Our customizations (audit checklist)

Every stage's audit diffs our delta before (`git diff refs/upstream/<prev> <pre-merge head>`) against after (`git diff refs/upstream/<new> HEAD`), hunk by hunk. Each change must survive, be matched by upstream, or have moved somewhere named. Watch especially:

- **Starters:** `src/starter_choose.c` (Axew/Deino, `STARTER_MON_COUNT` 2); Route 103 rival teams in `src/data/trainers.party`.
- **Quest menu:** `src/quests.c`, `include/quests.h`, `include/constants/quests.h`, `FLAG_SYS_QUEST_MENU_GET`. Quest save data is at the end of SaveBlock2. Script opcodes `questmenu`/`subquestmenu`/`returnqueststate` are 0xf0–0xf2 in `data/script_cmd_table.inc`, with 0xe7–0xef reserved as `ScrCmd_nop` placeholders: upstream's new opcodes replace placeholders in order. If upstream ever reaches 0xf0, stop.
- **Scripted battles:** `src/battle_controller_scripted.c`, `include/scripted_battle.h`, `src/data/scripted_battles.h`, `BATTLE_TYPE_SCRIPTED`. The controller copies code from upstream's recorded-battle controllers, so diff those between the old and new tag and port the changes.
- **Wally's Ralts:** `struct WallyRaltsData` at the end of SaveBlock3, `TryUpscaleWallyRalts`.
- **Items:** Wimpy Brace, Exp. Share All, Infinite Candy, EV Trainer key item and the stat editor UI (`src/ui_stat_editor.c`).
- **Species:** Aevian Magikarp/Gyarados forms; the Axew line's Dragon/Steel typing; Axew and Deino evolution levels.
- **Learnsets:** edits live in `src/data/pokemon/level_up_learnsets/gen_9.h`.
- **Encounters:** time-of-day tables in `src/data/wild_encounters.json` for Routes 101–104, Serene Pond, Oldale Ruins.
- **Maps:** new OldaleRuins, OldaleRuins_BerryFarmersHouse, SerenePond; edited Oldale, Routes 101/103/104, Petalburg Woods, Littleroot (notebook config in `data/scripts/players_house.inc`), Birch's lab. Story scripts: Devon Goods theft, Petalburg Woods, Berry Thief, Phantump.
- **Other code:** `src/time_events_custom.c`, start menu layout, party menu follower toggle, hold-B-to-walk in `src/field_player_avatar.c`.
- **Testing values to keep:** `SHINY_ODDS` 21845, the B-button catch boost in `Cmd_handleballthrow`.
- **Configs:** our values live in `include/config/verdant.h`, which fails the build if upstream renames an option (carry our value to the new name). Two exceptions stay in upstream files: OW_TIME_OF_DAY_ENCOUNTERS at the bottom of `config/overworld.h`, and the Gen 9 / Gigantamax / Tera toggles in `config/species_enabled.h`. Every other `include/config/*.h` should match upstream exactly; take upstream's side on any conflict there.
- **Claimed IDs** (check upstream hasn't claimed them): every row of the ID registry in the `save-system` skill (flags, vars, opcodes, items, species, berry tree, battle-type bit, maps, layouts, save offsets). `src/verdant_asserts.c` checks some of them at build time.

Then: `make` succeeds, `TESTS="Verdant:" make check` passes, and the Norman TV replay matches its golden transcript (`testing` skill).
