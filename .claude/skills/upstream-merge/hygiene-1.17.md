# Merge hygiene: ranked changes from the 1.14 → 1.17.1 upgrade

**Status (Oct 2026):** done: 1 (procedure and workflow in `SKILL.md` / `.claude/workflows/upstream-merge.js`), 2 (`include/config/verdant.h`; the Gen 9 / Gigantamax / Tera toggles stay in `species_enabled.h` because `data/sound_data.s` includes it directly), 3 (`[MENU_TOSS]` already matched upstream), 4 (replay `notebook_exp_share.replay`), 6 (Wimpy Brace pinned by `test/verdant/wimpy_brace.c`), 7 (check in `SKILL.md`), 8 (the 3 `test/save.c` failures stay until item 5) 9 (registry in the `save-system` skill; save offset asserts wait for item 5) and 15 (`test/verdant/customizations.c`). Open items are tracked in `BACKLOG.md` (Later → Merge hygiene). Paths below refer to the upgrade worktree; the tree is now master.

On item 6: hold effects are stored in one byte (`u8 holdEffect`), so the hard limit is 255. The 127 is upstream's documented rule (a comment added in 1.17.1, nothing enforces it). Upstream is at 126. Wimpy Brace used to make ours 127; since item 6 it works without taking a slot in upstream's list, and `src/verdant_asserts.c` checks the 127.

This list combines the postmortem, IDs and structure reviews of the four merges (1.14.4, 1.15.3, 1.16.4, 1.17.1). Those merges had 59 file conflicts and 22 compile or runtime breakages in our code. Items are ranked by merge pain avoided relative to effort. Where two items would both avoid the same conflict, it is counted once, under the item that removes the cause. All paths are relative to `/home/user/pve-upgrade`.

| # | Item | Avoids | Effort |
|---|---|---|---|
| 1 | rerere, zdiff3, automatic resume from WIP | a full second resolution of 1.17.1 (11 conflicts, 2 breaks), plus 1 | minutes |
| 2 | Config override header | 6 | small |
| 3 | Revert edits that change nothing on upstream lines | 8 | small |
| 4 | dynmultichoice for the Exp. Share menu | 3 | small |
| 5 | One planned pre-release save/ID break | 3, plus every future species addition | small |
| 6 | Drop isAevianForm and HOLD_EFFECT_WIMPY_BRACE | 2, plus the 127 cap | small |
| 7 | Untrack generated files, add a check | 1, plus noise in every verify | trivial |
| 8 | Merge master, bring the test baseline to zero | 9 permanent failures | small |
| 9 | ID registry, fix stale docs, asserts file | brief corrections in all 4 verifies | small |
| 10 | Remove scripted battles from upstream enums, tables and bits | 10, plus 2 reduced to one line | medium |
| 11 | Quest commands as callnative | 9, plus the reserved-slot rule | medium |
| 12 | Our functions in our own files | 8, plus 3 reduced to one line | small–medium |
| 13 | Scripted controller as a tracked fork | 8 of 22 breakages flagged before the build | small |
| 14 | Include placement and flag/var aliases | remaining include hunks, flags.h | small |
| 15 | Customization tests | the manual "lost customizations" audit | medium |
| 16 | `verdant_compat.h` | fixes for one API change drop from 6 files to 1 | small |
| 17 | Upstream PRs | 3 permanent hunks | small |
| 18 | Our map scripts in our own .s file, existing map group | 2 | small |
| 19 | Spike: play scripted battles through recorded-battle playback | about 6 breakages | large |
| 20 | Plan custom trainer IDs | future overflow | small |

---

## Do now

### 1. Turn on rerere and zdiff3, and resume an interrupted merge from its WIP branch automatically
- **Change:** In the shared git dir (`/home/user/pokeemerald-expansion-pkve/.git`) run `git config rerere.enabled true`, `git config rerere.autoUpdate true` and `git config merge.conflictStyle zdiff3`. Put the resume check in the workflow script so nobody has to remember it. Before `git merge refs/upstream/<step>`, look for a merge commit whose second parent is `refs/upstream/<step>`, on the branch or on `wip/<step>-interrupted`. If one exists, check it out, rebuild, and continue from there. When a release's notes mention whitespace or format sweeps, merge with `-Xignore-space-change`. Then, in a separate commit, take upstream's whitespace for each hunk where `git diff` and `git diff -w` against the tag differ only in whitespace. Resolution rule: for comment-only, whitespace-only and ordering-only text, always take upstream's version.
- **Why:** 1.17.1 was merged twice. The interrupted merge `ae529e8949` and the final merge `3d40888d95` have the same tree (`d662b6b41a`). The WIP adaptation `c7e268b3b4` and the final `fad893b7ba` also have the same tree (`720d18dda3`). So all 11 conflicts and both breakages were resolved by hand a second time, and the result was byte-identical. The 11 files: battle_scripts_1.s, config/battle.h, config/item.h, battle_string_ids.h, trainer_slide.h, vars.h, item.h, battle_main.c, battle_message.c, battle_script_commands.c, data/items.h. rerere is unset and there is no `rr-cache`. Separately, the 1.14.4 conflict in `src/field_player_avatar.c` came only from #9011 stripping trailing whitespace next to our `!(heldKeys & B_BUTTON)` line, and part of the 1.15.3 `src/data/items.h` conflict was a re-indent. zdiff3's base section shows that most of our conflicts are upstream rewording a comment on a line where we changed only the value.
- **Payoff:** high. **Effort:** minutes.

### 2. Keep all our config values in one override header
- **Change:** Create `include/config/verdant.h`. It includes `constants/flags.h` and `constants/vars.h`. Then, for each option we set: `#ifndef X` / `#error "X renamed upstream"` / `#endif` / `#undef X` / `#define X <ours>`. The options: B_TRAINERS_KNOCK_OFF_ITEMS, DEXNAV_ENABLED plus the 5 DN_* values, I_EXP_SHARE_FLAG, OW_FOLLOWERS_ENABLED, B_FLAG_FOLLOWERS_DISABLED, OW_USE_FAKE_RTC, OW_ALTERED_TIME_RATIO, P_SHEDINJA_BALL, P_SHUCKLE_BERRY_JUICE, P_CAN_FORGET_HIDDEN_MOVE, P_SHOW_DYNAMIC_TYPES, P_GEN_9_POKEMON, P_GIGANTAMAX_FORMS, P_TERA_FORMS and the two P_SUMMARY_SCREEN_IV_EV_* options.
  - Include it from `include/constants/global.h` directly before the `#if TESTING` include of `config/test.h` (lines 230-232), so test builds still get test.h's values.
  - Revert battle.h, dexnav.h, item.h, pokemon.h, species_enabled.h and summary_screen.h to upstream, including the `#include "constants/flags.h"` lines we added to dexnav.h, item.h and overworld.h.
  - One exception: `OW_TIME_OF_DAY_ENCOUNTERS` stays as text in `include/config/overworld.h`, because `tools/wild_encounters/wild_encounters_to_header.py` regex-parses that file (Makefile line 253). Put it as an `#undef`/`#define` pair at the bottom of overworld.h; the parser keeps the last match.
  - `data/battle_anim_scripts.s` and `data/field_effect_scripts.s` include config/battle.h and config/item.h without constants/global.h. None of our overridden options is used there today, so keep it that way.
- **Verify:** Do this first, on `upgrade-1.17.1`, before any other change touches the ROM. `make tidy && make` must reproduce the 1.17.1 verify sha1 `9a01056a4063712158b7ea7e92059d2732a50eea`. Any difference means an override did not take effect.
- **Follow-up commit:** Pin the GEN_LATEST values the design depends on: `P_LVL_UP_LEARNSETS GEN_9`, because our learnset edits live in gen_9.h. Also decide on the defaults that flipped without anyone noticing: B_OVERWORLD_WEATHER_OVERRIDE (1.17.1), OW_SYNCHRONIZE_NATURE and P_TM_MOVES_RELEARNER (1.15.3).
- **Why:** Six conflicts came from upstream rewording or adding lines next to lines where we changed only a value:
  - 1.14.4: `battle.h` (B_TRAINERS_KNOCK_OFF_ITEMS) and `pokemon.h` (P_CAN_FORGET_HIDDEN_MOVE, P_SHOW_DYNAMIC_TYPES).
  - 1.16.4: `dexnav.h` (DN_VAR_STEP_COUNTER comment) and `pokemon.h` (P_BASE_HP_1_SHEDINJA_HANDLING).
  - 1.17.1: `battle.h` (#8893 block rewrite) and `item.h` (I_EXP_SHARE_ITEM).

  The `#error` guard turns an upstream rename (like B_BURN_HIT_THAW to B_HIT_THAW) into a compile error instead of a define that silently does nothing. Upstream's own `config/test.h` already works this way.
- **Payoff:** high. **Effort:** small.

### 3. Revert edits that change nothing, and add our lines instead of rewriting upstream's
- **Change:**
  - `src/data/items.h` ITEM_EXP_SHARE: restore upstream's entry. With `I_EXP_SHARE_ITEM GEN_5`, upstream's `ItemUseOutOfBattle_ExpShare` (`src/item_use.c:250-273`) compiles to the same body as `ItemUseOutOfBattle_CannotUse` (`:1475-1478`). Neither calls SetUpItemUseCallback, Exp. Share has no battleUsage, and the Gen 6+ branch we edited is never compiled. Check once in-game that "Use" on the held Exp. Share still shows Dad's advice.
  - `BattlePutTextOnWindow` (`src/battle_message.c`): drop BATTLE_TYPE_SCRIPTED from `(BATTLE_TYPE_LINK | BATTLE_TYPE_RECORDED)`. PlayScriptedBattle always sets RECORDED together with it (`src/battle_controller_scripted.c:955`), and nothing clears it.
  - `src/data/party_menu.h`: move `[MENU_TOSS]` back to upstream's position.
  - `data/maps/LittlerootTown_BrendansHouse_2F/scripts.inc`: restore upstream's Notebook, GameCube, clock and room texts. Give our script a new label in `data/scripts/players_house.inc` (for example `PlayersHouse_2F_EventScript_VerdantNotebook`) and repoint the bg_event `script` fields at `LittlerootTown_BrendansHouse_2F/map.json:254` and `LittlerootTown_MaysHouse_2F/map.json:254`.
  - `include/constants/vars.h`: restore upstream's VAR_STARTER_MON comment. Document 0=Axew, 1=Deino in item 14's alias header instead.
  - `src/battle_script_commands.c`: restore upstream's second `#include "test/battle.h"` (around line 71) and add `scripted_battle.h` as a new line.
  - Add lines rather than editing upstream's:
    - HandleLowHpMusicChange (`src/battle_gfx_sfx_util.c`): keep upstream's two `u16 hp = GetMonData(...)` lines and put the early return after them.
    - BattleTypeAllowsExp: add a separate early `return FALSE` instead of editing upstream's closing `| BATTLE_TYPE_EREADER_TRAINER))` line.
    - `src/battle_controllers.c`: put `IsControllerScriptedPlayer(battler) ||` on its own line right after `if (`, not glued to `|| IsControllerWally(battler))`.
- **Why:** These edits caused 8 conflicts:
  - `src/data/items.h` in 1.14.4, 1.15.3 and 1.17.1.
  - `battle_message.c` hunk 3 in 1.15.3 (against POKEDUDE).
  - `party_menu.h` in 1.16.4.
  - BrendansHouse_2F `scripts.inc` in 1.15.3 (upstream's `{REGION}` edit).
  - `vars.h` in 1.17.1 (#9955 VAR_MIRAGE_RND comments).
  - The HandleHealthBarUpdate hunk in `battle_controllers.c` in 1.15.3.

  The other edits haven't conflicted yet, but they sit on lines upstream edits.
- **Payoff:** high. **Effort:** small.

### 4. Use dynmultichoice for the Exp. Share mode menu
- **Change:** In `data/scripts/players_house.inc:699`, replace `multichoice 14, 6, MULTI_EXP_SHARE_MODE, FALSE` with `dynmultichoice 14, 6, FALSE, 2, 0, DYN_MULTICHOICE_CB_NONE, PlayersHouse_2F_Text_Single, PlayersHouse_2F_Text_All`, and add two `.string` labels. Check that B still returns MULTI_B_PRESSED (127) for the existing `case 127`. Then delete MULTI_EXP_SHARE_MODE, MultichoiceList_ExpShareMode and its sMultichoiceLists entry. Rule: every future custom menu uses dynmultichoice.
- **Why:** 3 conflicts and one ID collision:
  - `include/constants/script_menu.h` in 1.15.3 (FRLG's MULTI_BIKE_SHOP took 115, so ours was renumbered to 159) and in 1.16.4 (#9066 enum conversion).
  - `src/data/script_menu.h` in 1.15.3.
- **Payoff:** medium. **Effort:** small.

### 5. Do one planned pre-release save/ID break, then lock the sizes
- **Change:**
  - a. Write in CLAUDE.md that there is no save compatibility until the first public release. There are no release tags, the BACKLOG.md "Pre-Release Checklist" is unchecked, and SaveBlock1 already shifted in 1.15.3. Until then, never renumber upstream constants or reorder upstream JSON just to keep IDs stable.
  - b. `include/constants/species.h`: restore upstream's numbers for the 23 Z-A megas (1550-1572). Declare SPECIES_MAGIKARP_AEVIAN and SPECIES_GYARADOS_AEVIAN without explicit values, below the "Add any custom species between here and SPECIES_CUSTOM_END" comment (lines 1699-1701). That shrinks our delta from 52 lines to about 2. The verify reports found no hardcoded 1550/1551. The IDs review proposed fixed values 1998/1999 instead. That costs about 90 KB of empty gSpeciesInfo, fusion and bard entries at 72% ROM use, and it only matters once saves must stay stable, so decide on it at the release freeze.
  - c. Define `struct VerdantSaveData { wallyRalts; questData[...]; subQuests[...]; u8 reserved[64]; }` in `include/verdant_save.h`. Make it the **first** member of SaveBlock3, before the `OW_USE_FAKE_RTC` block. Remove our SaveBlock2 tail, restoring `}; // sizeof=0xF2C`. Add STATIC_ASSERTs on its offset (0) and size.
  - d. Move `BERRY_TREE_OLDALE_RUINS_1` from 90 to 127 (BERRY_TREES_COUNT is 128). Only the define changes; map.json and time_events_custom.c use the name.
  - e. Put the post-break measured sizes into T_SAVEBLOCK1/2/3_SIZE and T_POKEMONSTORAGE_SIZE in `test/save.c`. Today's values are 15564/3908/28/34144. Upstream hasn't touched that file since #8339, so this costs no conflicts.
  - f. Add an allocation rule to the save-system skill: in fixed-size saved ranges, allocate from the top down (items from 999, berry trees from 127, flags from the top of a reserved block). IDs are frozen at release.
- **Why:**
  - `species.h` conflicted in 1.15.3, when the Z-A megas took our 1550/1551 and 23 upstream lines were renumbered by hand. It conflicted again in 1.16.4, when #9507 converted the list to an enum and the renumbering was reapplied. Every future mega or form addition would conflict again. In 1.15.3, layouts.json and map_groups.json also needed hand placement in front of about 400 FRLG layouts.
  - wallyRalts currently sits after five config-gated SaveBlock3 blocks (fake RTC, NPC followers, itemFlags, dexNavSearchLevels, apricorns), so enabling any of them moves it silently. Quest data in SaveBlock2 was misplaced in the brief and corrected in all four verifies.
  - The save tests failed at every step against upstream's sizes, so the one real layout break (1.15.3: NATIONAL_DEX_COUNT 905 → 1019, SaveBlock1 grew from 15536 to 15564 bytes) was caught only by sizeof checks compiled by hand.
  - Low-first allocations got hit every time: opcode 0xe5, battle-type bit 28, MULTI 115, species 1550. High-first ones (items 996-999, opcodes 0xf0+) never did.
- **Payoff:** medium-high. It removes a recurring conflict class and turns save drift into a test failure. **Effort:** small (one session plus a new dev save).

### 6. Delete isAevianForm and HOLD_EFFECT_WIMPY_BRACE
- **Change:** Delete `u32 isAevianForm:1` (`include/pokemon.h:490`) and its two writes (`gen_1_families.h:17975`, `:18042`), and restore padding4 to upstream's width. Delete HOLD_EFFECT_WIMPY_BRACE (`include/constants/hold_effects.h:27`), leave ITEM_WIMPY_BRACE without a holdEffect, and test `heldItem == ITEM_WIMPY_BRACE` in MonGainEVs (`src/pokemon.c:4976`). No save impact.
- **Why:** Nothing reads isAevianForm, yet it conflicted in `include/pokemon.h` in 1.15.3 and 1.16.4. WIMPY_BRACE is inserted mid-enum, so it renumbers every later hold effect. It has no gHoldEffectsInfo row or sHoldEffectNames entry, and it puts HOLD_EFFECT_COUNT at exactly the 127 cap that 1.17.1 documents. One new upstream hold effect would push us over.
- **Payoff:** medium. **Effort:** small.

### 7. Never commit generated files or build outputs
- **Change:** `git rm --cached tools/aif2pcm/aif2pcm`. Add a check that runs before each merge commit:
  - `git ls-files -ci --exclude-standard` lists only upstream's own tracked-but-ignored files (mgba-rom-test.exe, the compresSmol sources).
  - No file in `git diff --name-only refs/upstream/<tag> HEAD` contains "DO NOT MODIFY" or "auto-generated".
  - No `*.backup` files exist.

  Stage resolutions by path or with `git add -u`, never with `git add -A`.
- **Why:** The 1.14.1 merge `d987fb4465` committed the generated `include/constants/script_commands.h` with our opcodes in it. When #8899 untracked that file, 1.14.4 got a modify/delete conflict. The same merge committed the aif2pcm ELF, which all four verify steps flagged. 1.15.3 left `.backup` files in the repo root.
- **Payoff:** low. **Effort:** trivial.

### 8. Merge master into the upgrade branch and bring the test baseline to zero
- **Change:** After item 2's sha1 check, merge master into `upgrade-1.17.1`. Master is `5e907fabf6`, 20 commits past the branch point, and it adds `test/verdant/route103_rival.c` and `tools/replay` with `golden/norman_tv_battle.log`. Branch future upgrades from current master. Then fill the empty item slots 874-995 with one range initializer next to our items: `[ITEM_GLIMMORANITE + 1 ... ITEM_EV_TRAINER - 1] = { .name = ITEM_NAME("????????"), .description = COMPOUND_STRING("?????"), .pocket = <upstream's hidden/none pocket> }`. Because the build uses `-Werror -Woverride-init`, it fails the day upstream adds item 874; the fix is to bump the lower bound. Item 5e removes the 3 save.c failures.
- **Why:** 9 baseline failures (6 test/text.c name/description tests on the NULL gap names, plus 3 test/save.c) were re-triaged at every step, and they hid the 1.15.3 save break. debug.c:3703 ("Fill PC items") also adds all 122 nameless items. The upgrade branch has no project tests at all (`git diff refs/upstream/1.17.1 -- test/` is empty).
- **Payoff:** medium-high. **Effort:** small.

### 9. Keep one ID registry, fix the stale docs, and add one asserts file
- **Change:** Add a table to the save-system skill that lists every claimed ID and the allocation rule for each range: flags, vars, opcodes (while they exist), items, species, berry tree, battle-type bit, map group/num, layout IDs, and save fields with offsets. Generate the next upgrade's customization brief from that table. Fix the stale docs:
  - `.claude/skills/quest-system/SKILL.md` says the opcodes are 0xe5-0xe7, which are now upstream's hidefollower and getbraillestringwidth.
  - Its save location needs updating after item 5.
  - `.claude/skills/save-system/SKILL.md` uses `FLAG_UNUSED_0x001`, which doesn't exist.

  Create `src/verdant_asserts.c` (guarded `#if !TESTING`) as the single home for STATIC_ASSERTs from items 5, 6 and 14: save offsets and sizes, `HOLD_EFFECT_COUNT <= 127`, and the config flags being distinct. At the release freeze, add pins for LAYOUT_SERENE_POND/OLDALE_RUINS/BERRY_FARMERS_HOUSE (442-444) and MAP_OLDALE_RUINS / MAP_OLDALE_RUINS_BERRY_FARMERS_HOUSE.
- **Why:** The brief was wrong in ways every verify had to correct. It listed FLAG_UNUSED_0x264 as repurposed when it is unused (corrected 3 times). It put quest data in SaveBlock3 (corrected 4 times). It gave the reserved range as 0xe6-0xef, when it has been 0xe7-0xef since 1.15.3. Map and layout IDs were hand-checked in 1.15.3, 1.16.4 and 1.17.1.
- **Payoff:** medium. **Effort:** small.

---

## Do before the next upgrade

### 10. Remove the scripted-battle system from upstream enums, tables, bits and battle scripts
- **Change:** In order (a is the cheapest and removes 7 conflicts by itself):
  - a. **Strings.** Make "The challenger has no more Pokémon..." a static string in `src/battle_controller_scripted.c`. In BattleScript_ScriptedBattleLostEndNoSlide, replace `printstring STRINGID_CHALLENGERLOSTTOENEMYTRAINER` with a callnative that sets `gBattleStruct->trainerSlideMsg`, then `printstring STRINGID_TRAINERSLIDE` (`src/battle_message.c:2809`; placeholders still expand). Replace TRAINER_SLIDE_BATTLE_WON with a `GetScriptedBattleEndMsg()` in our file. Its only users are GetScriptedBattleAnnouncerMsg (`:991`) and BS_TryScriptedBattleWonSlide, which already sets trainerSlideMsg. Remove our lines from battle_string_ids.h, gBattleStringsTable and trainer_slide.h.
  - b. **Battle-type bit.** Add `bool32 IsScriptedBattle(void) { return sCurrentScriptedBattle != NULL; }` and make sure that pointer is cleared in CB2_AfterScriptedBattle. Replace the 10 C checks with it. Replace the 3 `jumpifbattletype BATTLE_TYPE_SCRIPTED` with a callnative jump (BS_JumpIfScriptedBattle, shaped like jumpifnowhiteout). Stop setting the bit at `battle_controller_scripted.c:955` and restore `BATTLE_TYPE_30`.
  - c. **trainer_slide.c.** Move the 33-line block into `ScriptedBattle_ShouldDoTrainerSlide()` in our file. It needs only the public CanBattlerSwitch, IsTrainerSlidePlayed and MarkTrainerSlideAsPlayed. Leave one line: `if (IsScriptedBattle()) return ScriptedBattle_ShouldDoTrainerSlide(battler, slideId);`.
  - d. **battleStyle.** Drop the ClearSetBScriptingStruct override. Set `gSaveBlock2Ptr->optionsBattleStyle` to SET in PlayScriptedBattle and restore it in CB2_AfterScriptedBattle, next to the existing party restore.
  - e. **Battle scripts.** Move BattleScript_ScriptedBattleWon/LostEnd, the `tryscriptedbattlewonslide` macro and BS_TryScriptedBattleWonSlide into our own files. A new `data/verdant_battle_scripts.s` is picked up by `DATA_ASM_SRCS := $(wildcard data/*.s)` (Makefile:321). Copy the two CMD_ARGS/NATIVE_ARGS lines from `battle_script_commands.c:95-96`, and use `::` labels for anything referenced from another file. Leave one-line jumps in BattleScript_LocalTrainerBattleWon and LocalBattleLost.
- **Why:**
  - `battle_string_ids.h` and the gBattleStringsTable hunk conflicted in 1.15.3, 1.16.4 and 1.17.1, because upstream appends before `*_COUNT` every release. In 1.17.1 upstream also deleted the entry just above ours.
  - `trainer_slide.h` conflicted in 1.17.1 (#9211).
  - `constants/battle.h` conflicted in 1.15.3 when FRLG GHOST took bit 28. We moved to bit 30, the last free one (bit 31 is RECORDED_IS_MASTER), so the next claim leaves nowhere to go.
  - `trainer_slide.c` conflicted in 1.15.3 and broke in 1.17.1 (TRAINER_SLIDE_LAST_SWITCHIN).
  - `battle_main.c` conflicted in 1.15.3.
  - `battle_scripts_1.s` conflicted in 1.15.3, and in 1.17.1 an auto-merged block would not have assembled (end2 removed).
  - BS_TryScriptedBattleWonSlide broke in 1.15.3 with `-Werror=enum-conversion`.
- **Payoff:** high (about 10 conflicts avoided, 2 reduced to one line, 2 breakages moved into our own files). **Effort:** medium.

### 11. Make the quest commands callnative natives
- **Change:**
  - Move ScrCmd_questmenu/subquestmenu/returnqueststate from `src/scrcmd.c` into `src/quests.c` as natives taking `(struct ScriptContext *ctx)`. They use only ScriptReadByte/Halfword, VarGet, ScriptContext_Stop and QuestMenu_*.
  - Add `Script_RequestEffects(SCREFF_V1 | SCREFF_SAVE)` on the paths that set state, and SCREFF_HARDWARE for QUEST_MENU_OPEN. This fixes a latent bug: the table says `requests_effects=1`, but the handlers never request effects. So RunScriptImmediatelyUntilEffect, used for coord events and in trainer_see.c, can write quest state or open the menu during its dry run.
  - Redefine the macros in a new `asm/macros/quests.inc` as `callnative Native_QuestMenu, requests_effects=1` followed by the same `.byte`/`.2byte` arguments. Upstream already does this with setstatus1/setko, createmon and gettimeofday.
  - Start quests.inc with `#include "constants/quests.h"`; preproc inlines `.include` before cpp runs. Include quests.inc from `data/event_scripts.s` right after `.include "asm/macros/event.inc"`, and drop the separate constants include there.
  - Move the ~190 gText_Quest*/gText_SideQuest* strings out of `src/strings.c` and `include/strings.h` into quests.c, or into a `src/data/quest_strings.h` that it includes.
  - Delete SCR_OP_RESERVED_E7..EF and the three opcodes from `data/script_cmd_table.inc`, so that file matches upstream, and retire the "stop at 0xf0" check.
  - No save impact. Replay the Berry Thief quest once.

  Rule: new script commands are always callnative, never opcodes or specials.
- **Why:** This upgrade had 7 conflicts from the quest commands:
  - 1.14.4: `script_commands.h` modify/delete.
  - 1.15.3: `event.inc`, `script_cmd_table.inc` (getbraillestringwidth took 0xe6), `scrcmd.c` (4 hunks) and `strings.c`.
  - 1.16.4: `event.inc` and `scrcmd.c` (2 hunks).

  Before that, the 1.14.1 add/add (our 0xe5 against hidefollower) forced the move behind 9 placeholder slots. The `event_scripts.s` constants/quests.h include conflicted in 1.15.3 and 1.16.4. Meanwhile upstream's opcode table grew by one entry from 1.14 to 1.17, while event.inc gained about 90 callnative macros.
- **Payoff:** high. **Effort:** medium.

### 12. Move our functions and prototypes into our own files
- **Change:**
  - Add `src/verdant_items.c` and `include/verdant_items.h`. They hold RemoveItemFromEverywhere and Script_RemoveItemFromEverywhere, which use only public bag, PC, party and daycare APIs, plus the ItemUseOutOfBattle_EVTrainer/ExpShareAll prototypes. Include the header once, right after global.h, in item.c and item_use.c. Revert item.h, item_use.h, and the item.c tail and its 3 includes.
  - Move the OnTimeOfDayChange and InitBerryThiefTreeState prototypes to `include/time_events_custom.h`, and revert overworld.h.
  - ItemUseCB_EVTrainer stays in party_menu.c because it uses sPartyMenuInternal, but declare it in `include/ui_stat_editor.h` and revert party_menu.h.
  - Declare the scripted controller functions only in `include/scripted_battle.h`, not in battle_controllers.h as well; both copies had to be retyped in 1.15.3.
  - Add `src/wally_ralts.c` with IsRaltsLine, IsWallyTrainer, TryUpscaleWallyRalts and CreateWallyTutorialRalts(mon). DoTrainerBattle gets one call. StartWallyTutorialBattle keeps upstream's CreateMaleMon line and adds one call after it.
  - Reduce the B-button catch boost to `odds = Verdant_TestingCatchOdds(odds);`, gated by a `VERDANT_TESTING` switch in config/verdant.h. That also covers the pre-release revert listed in BACKLOG.md.

  Rule: an upstream file gets at most one include line and one call line per feature, placed away from lines upstream is changing.
- **Why:**
  - `item.h` conflicted in 1.15.3 (enum Item retype) and 1.17.1 (ItemHasVolatileFlag removed next to our line).
  - `party_menu.h` conflicted in 1.15.3.
  - `overworld.h` conflicted in 1.16.4 (#10343).
  - `item.c` had 2 conflict hunks in 1.16.4.
  - `battle_main.c` conflicted in 1.16.4 and 1.17.1. In 1.17.1, #9440 deleted the surrounding block, so about 60 lines had to be moved by hand into battle_setup.c.
  - `battle_setup.c` conflicted in 1.16.4, and Wally's CreateMon code broke in 1.15.3.
  - The catch boost conflicted in 1.15.3 (#8682), 1.16.4 and 1.17.1 (#9008).
  - The 1.16.4 party-global deprecation forced edits in 4 upstream files only because our code lived in them.
- **Payoff:** high (about 8 conflicts avoided, 3 reduced to one-line moves). **Effort:** small to medium.

### 13. Track the scripted controller as a fork of upstream's recorded controllers
- **Change:** Add a header comment to `src/battle_controller_scripted.c` that lists each mirrored upstream function (for example Intro_TryShinyAnimShowHealthbox and Intro_WaitForShinyAnimAndHealthbox, `battle_controller_recorded_player.c:120/186`) and the upstream tag it was last synced to. Add a merge step that runs before the build: `git diff refs/upstream/<old> refs/upstream/<new> -- src/battle_controller_recorded_player.c src/battle_controller_recorded_opponent.c src/battle_controllers.c include/battle_controllers.h`. Port each change it shows, and grep the release's REFACTORS section for the upstream symbols our files use. Add a test or headless replay (see the testing skill) that plays gScriptedBattle_NormanTV to the end and asserts `BattlerIsPlayer(B_BATTLER_0)` and `BattlerIsOpponent(B_BATTLER_1)`.
- **Why:** This file caused 8 of the 22 breakages:
  - 1.15.3: about 100 enum BattlerId errors, #8519 (SUCCESSBALLTHROWANIM), #8204 (CreateMon), and a runtime-only break from #8247 that left gBattlerBattleController at NONE.
  - 1.16.4: #9475 (gTrainerBacksprites), #10542 (BATTLE_PARTNER in the copied intro code), and the party-global deprecation.
  - 1.17.1: #9211.

  The #8247 break was found only by reading code. Yet #8247 is listed in `docs/changelogs/1.15.x/1.15.0.md:18`, and in the same range SetControllerToRecordedPlayer gained the exact line we were missing (`:104`). The diff step would have shown it.
- **Payoff:** high. **Effort:** small (the comment and workflow step); medium with the test.

### 14. Put includes in one place, and alias flags and vars instead of renaming upstream lines
- **Change:**
  - Includes: in upstream files, add our includes as one block right after the first include (global.h, or line 1 of a `.s` file), never inside upstream's sorted lists. The one include we already placed right after global.h (`src/party_menu.c:2`) never conflicted in 4 merges.
  - Flags and vars: restore upstream's lines in `flags.h`, `vars.h` and `berry.h`. Add `include/constants/verdant_flags.h`, included from config/verdant.h and event_scripts.s, with aliases such as:
    - `#define FLAG_STAT_EDITOR_IV_UNLOCK FLAG_UNUSED_0x024`, and the same for 0x1AA, 0x1AB and 0x2D9.
    - `#define FLAG_SYS_QUEST_MENU_GET FLAG_UNUSED_0x863`.
    - `#define FLAG_HIDE_ROUTE104_DEVON_RESEARCHER FLAG_HIDE_PETALBURG_WOODS_AQUA_GRUNT`.
    - Descriptive names for the IDs our configs claim through their FLAG_UNUSED/VAR_UNUSED names: FLAG_DEXNAV_SEARCHING 0x020, FLAG_EXP_SHARE_ALL_ON 0x021, FLAG_DEXNAV_DETECTOR 0x022, FLAG_DEXNAV_GET 0x023, FLAG_FOLLOWERS_DISABLED 0x02A, VAR_DEXNAV_SPECIES 0x404E, VAR_DEXNAV_STEP_COUNTER 0x4083.
    - STARTER_AXEW/STARTER_DEINO, and the berry tree from item 5.
  - Point the configs and the Birch lab script (`ProfessorBirchsLab/scripts.inc:328`, `setflag FLAG_UNUSED_0x023`) at the new names.
  - Add a STATIC_ASSERT (in verdant_asserts.c) that DN_FLAG_*, I_EXP_SHARE_FLAG, B_FLAG_FOLLOWERS_DISABLED and any future B_/OW_/WE_FLAG_* are pairwise distinct and greater than TEMP_FLAGS_END.
  - Allocate new flags from 0x493-0x4EF, top-down, and new vars from 0x40FF downward. Never use 0x264 or 0x404E, which upstream's config comments suggest as examples.
- **Why:** 7 conflict hunks were upstream inserting an include next to ours (event_scripts.s, global.h, scrcmd.c ×2, battle_message.c, item.c); items 5, 11 and 12 remove most of them outright. With aliases, if upstream claims one of our slots, its FLAG_UNUSED_* name disappears and our alias fails to compile. That replaces the collision audit every verify did by hand. Upstream touched flags.h in 8 commits in this range, including #9761, a spelling and spacing pass. We have already collided with ourselves once: B_FLAG_FOLLOWERS_DISABLED started at 0x020, which is DN_FLAG_SEARCHING, and DexNav clears that flag on every map transition. And flags_frlg.h still names 0x863 FLAG_UNUSED_0x863, so the two headers disagree today.
- **Payoff:** medium. **Effort:** small.

### 15. Add customization tests and make them required verify steps
- **Change:** Add `test/verdant/customizations.c` with tests for:
  - GetStarterPokemon(0/1) returning SPECIES_AXEW and SPECIES_DEINO.
  - The learnset edits via GetSpeciesLevelUpLearnset: Metal Claw on the Axew line; Deino's Bite at L6 and Dragon Breath at L16; Machop's Pound at L1 and Low Kick at L6; Roggenrola's Rock Throw at L7.
  - The Axew line's Dragon/Steel typing and the Axew and Deino evolution levels.
  - The Aevian forms in the Magikarp and Gyarados form tables.
  - The custom item fields.
  - The Morning and Night encounter tables for Routes 101-103, Serene Pond and Oldale Ruins in gWildMonHeaders.

  Make `TESTS="Verdant:" make check` and the Norman TV golden replay required verify steps for every merge.
- **Why:** Every verify re-checked "no lost customizations" by hand, with -U0 multiset diffs, sizeof probes against both trees, and greps for each repurposed ID. The 1.15.3 controller regression, and the 1.17.1 `end2` block that git merged cleanly but would not have assembled, were found only by reading code.
- **Payoff:** high (replaces the manual audit). **Effort:** medium.

### 16. Put a compatibility header between our code and upstream APIs that keep changing
- **Change:** Add `include/verdant_compat.h` with static inline wrappers:
  - Player and opponent party and party-count accessors.
  - `Verdant_CreateMon(mon, species, level, personality, otId, ivs)`, implemented as CreateMonWithIVs plus GiveMonInitialMoveset.
  - Trainer back-pic coords and palette.
  - The partner lookup.
  - VERDANT_PIC_* aliases for the trainer pics in `src/data/scripted_battles.h`.

  Route quests.c, ui_stat_editor.c, battle_controller_scripted.c, wally_ralts.c and verdant_items.c through it.
- **Why:**
  - The 1.15.3 CreateMon removal broke two files (2 fixes; with the header, 1).
  - The 1.16.4 party-global deprecation forced edits in 6 files, 6 of that step's 9 breakages.
  - TRAINER_PIC names changed twice (TRAINER_BACK_PIC_LEAF → TRAINER_PIC_BACK_LEAF → TRAINER_PIC_LEAF).
  - gTrainerBacksprites became GetTrainerBackPicCoords/Palette.
- **Payoff:** medium. **Effort:** small.

---

## Opportunistic

### 17. Send our generic fixes upstream
- **Change:** Open PRs to rh-hideout/pokeemerald-expansion for:
  - (1) The `BATTLE_TYPE_SAFARI | BATTLE_TYPE_CATCH_TUTORIAL` guard from `d8409a6c5e` in BtlController_HandleDrawTrainerPic and HandleTrainerSlide. It fixes Wally's tutorial back-pic staying a black silhouette, and the bug is still unfixed in 1.17.1.
  - (2) Honouring GetItemConsumability on both Rare Candy paths.
  - (3) Shared BtlController_ helpers for the recorded controllers' Intro_* callbacks, like the existing BtlController_Intro_DelayAndEnd (`include/battle_controllers.h:479`). That would let us delete about 170 copied lines.

  Fix the Rare Candy evolve path in our tree now regardless: per the 1.16.4 verify, it still consumes an Infinite Candy.
- **Why:** The trainer-pic patch broke in 1.15.3 when BATTLE_TYPE_WALLY_TUTORIAL was renamed to CATCH_TUTORIAL. Once these PRs are merged, 2 hunks in battle_controllers.c and 1 in party_menu.c disappear for good, along with the copied intro code that broke in 1.16.4.
- **Payoff:** low-medium. **Effort:** small.

### 18. Move our map scripts into our own .s file, and put the new house in an existing map group
- **Change:** Create `data/verdant_scripts.s` with event_scripts.s's `#include` preamble, `.include "asm/macros.inc"`, `.include "asm/macros/event.inc"`, `.include "asm/macros/quests.inc"` and `.section script_data`. Move the SerenePond, OldaleRuins and OldaleRuins_BerryFarmersHouse script includes there; labels referenced from other files need `::`. Porymap appends new map includes to event_scripts.s, so move each one after creating a map. Append OldaleRuins_BerryFarmersHouse to `gMapGroup_IndoorOldale`, which upstream never touches, instead of our own group at the end of group_order. This changes that map's group/num, so do it inside item 5's save break if you do it now.
- **Why:** `event_scripts.s` hunk 2 in 1.16.4 (against upstream's wild_encounter.inc append). `map_groups.json` in 1.15.3: our new group was appended exactly where FRLG appended its groups.
- **Payoff:** medium. **Effort:** small.

### 19. Spike: play scripted battles through upstream's recorded-battle playback
- **Change:** Build a `struct RecordedBattleSave` in EWRAM: parties from ScriptedBattle_BuildParty, action bytes from the ScriptedBattleAction lists, rngSeed and battleFlags. Play it through SetVariablesForRecordedBattle and SetPartiesFromRecordedSave (`src/recorded_battle.c`), using upstream's unchanged RecordedPlayer/RecordedOpponent controllers. Keep only our announcer and intro-text hooks. If the frontier-specific parts of recorded playback don't fit (link names, trainer pics, B to stop), keep our controller but restrict it to verdant_compat.h and the BtlController_* helpers. Either way, the Norman TV golden transcript must pass.
- **Why:** This would delete most of the 1005-line clone, the InitBtlControllersInternal branch and the IsControllerScripted* additions in battle_controllers.c (a 1.15.3 conflict). About 6 of the controller's breakages would not have existed.
- **Payoff:** high. **Effort:** large.

### 20. Decide where custom trainers go before adding the first one
- **Change:** Pick one option. (a) Reuse vanilla trainer IDs we will never use, under new names, leaving opponents.h's count lines untouched. (b) Raise MAX_TRAINERS_COUNT_EMERALD once, in a planned save break, and allocate top-down from the new ceiling. Record the choice in the registry.
- **Why:** Only 9 slots (855-863) remain before trainer flags (0x500 + id) overflow into SYSTEM_FLAGS, where FLAG_SYS_QUEST_MENU_GET sits at +0x3. 855 is also upstream's next Emerald trainer slot, and the TRAINERS_COUNT_EMERALD line was rewritten in 1.15. The dual-path gyms and the Roxanne boss will need new trainers.
- **Payoff:** low now. **Effort:** small.
