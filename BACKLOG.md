# Backlog

Tasks only, grouped by milestone in priority order. Each task appears once.

**Tags:** `[ready]` no open questions, can be picked up · `[refine]` shape known, questions still open · `[idea]` needs breakdown · `(you: …)` waiting on the project owner (design call, playtest, map edit).

Principles and vague plans don't live here: see `DIVERGENCE_THEORY.md`, `ROADMAP.md` and `BALANCE.md`. How this file is kept in sync: `CLAUDE.md` → "How we track work".

## Pre-Demo (current target)

Polished build that ends at **beating Wattson on both routes**. Hero: badge 3 (Roxanne, Brawly, Wattson), almost all vanilla. Anti-hero: badge 1, the Rubicon.

### Story: tunnel pivot (both routes)

Spec: `DIVERGENCE_THEORY.md` (The Tunnel, The Rubicon, The Devon Goods) and `ROADMAP.md` Segment 3.

- [ ] [ready] Rustboro gym closure: sign or attendant ("Leader Roxanne is assisting the Devon Corporation."), Rustboro NPCs and Devon staff point the player toward Route 116. Key off `VAR_PETALBURG_WOODS_STATE` = 2, set by the Route 104 theft and not read yet. The gym reopens once the goods are returned (hero).
- [ ] [refine] Rusturf Tunnel map rework: remove the east-side Rock Smash gate (no Wally event there), add a visible broken-rocks breach at the old gate, add the hidden side passage revealed on retreat after the scene. Open: side passage placement.
- [ ] [refine] Tunnel scene: Roxanne's overworld sprite walks to the tunnel ahead of the player; cutscene at the breach (grunt seen fleeing east, Roxanne's line, she heads back to Rustboro); on retreat, the grunt in the side passage: fight, recover the goods. Open: choreography, and the final flag set (tentative: `FLAG_TUNNEL_SCENE_COMPLETE`, `FLAG_GOODS_RECOVERED`, `FLAG_ANTI_HERO_COMMITTED`, `FLAG_CUT_DIVERGENCE_ANTI_AVAILABLE`) and which free IDs they take.
- [ ] [ready] Hero: return the goods to Devon → Cut HM (plus Exp Share if not already given), Roxanne back in her gym. Depends on the tunnel scene.

### Story: anti-hero route to Wattson

- [ ] [ready] Audit vanilla events that assume the player reaches Mauville from Slateport / Route 110 (Wally's Mauville battle, Verdanturf, etc.) and list what breaks when arriving from Route 117 with 0 badges. Fixes become follow-up items.
- [ ] [ready] Wattson dialogue: pre-fight line when the badge count is 0 ("No badges?! WAHAHA! Prove yourself!"), post-victory "You... you actually did it. WAHAHA!"
- [ ] [ready] Rubicon flag: set `FLAG_ANTI_HERO_COMMITTED` on Wattson's badge if the goods weren't recovered. World-state consequences are Demo 1.
- [ ] [refine] (you: playtest) Wattson difficulty for a 0-badge team. Design is "same team, no scaling" (resolved in `DIVERGENCE_THEORY.md`); this checks it's beatable and fair with what's catchable by then.
- [ ] [refine] (you: design call) End-of-pre-demo stop after Wattson on both routes: what the player sees (message, block, credits?).

### Encounters

One area at a time: design the table in `BALANCE.md` first, then implement.

- [ ] [ready] Route 104: implement the day/night tables and Finneon fishing already designed in `BALANCE.md`. The live table is still vanilla.
- [ ] [refine] Petalburg Woods
- [ ] [refine] Route 116
- [ ] [refine] Rusturf Tunnel interior
- [ ] [refine] Rusturf east-side gauntlet (L15-18, soft gate), anti-hero
- [ ] [refine] Route 117, anti-hero (optional for hero)

### Starter follow-ups

- [ ] [ready] Remove the dead third-starter branches (`MUDKIP` slot, `VAR_STARTER_MON` = 2) from the rival scripts (Routes 104, 110, 119, Rustboro, Lilycove) and the Petalburg Pokemon Center, and write Axew/Deino type lines for the Pokemon Center NPC. Keep the dead `*_MUDKIP` trainer entries (removing them shifts trainer IDs). The Mauville Game Corner keeps its branches until the dolls exist (Later). Slot mapping: `CLAUDE.md` → Custom Starters.
- [ ] [ready] Audit everything players see around the starter choice: Birch's bag art, the rescue dialogue, the lab, Mom/Dad lines. Art problems go to `ART_ASSETS.md`; fixes become separate items.
- [ ] [ready] Code audit for three-starter assumptions: rival scripts, Petalburg Pokemon Center, Game Corner, `battle_setup.c`, `field_specials.c` (`IsStarterInParty`), `credits.c`, the scripted TV battle with Dad. Includes fixing the off-by-one guard in `GetStarterPokemon` (`>` should be `>=`).
- [ ] [ready] (you: playtest) Per starter: Birch's rescue → Route 103 rival → TV battle with Dad → credits show the right starter.

### Polish

- [ ] (you: design call) Rival battles 2-3 (Rustboro, Route 110, hero): undecided, see `DIVERGENCE_THEORY.md` open questions. Until then hero testers meet the vanilla teams (Grovyle/Marshtomp lines).
- [ ] [ready] Oldale Mart: the potion guy says berries have been scarce lately and are in demand, and gives a Potion.
- [ ] [ready] Quest menu: hide locked quests instead of showing "??????" (list building in `src/quests.c`).
- [ ] [ready] Norman's TV replay marks Cofagrigus and Slaking as seen in the Pokédex. Scripted battles shouldn't touch save data: skip dex updates when `BATTLE_TYPE_SCRIPTED` (or `IsScriptedBattle()`). Done when a fresh save after the broadcast has 0 seen.
- [ ] [refine] Story-flag logic tests (Petalburg Woods, Berry Thief). Open: Tier 1 script tests or replays.

### Testing tools

Replay harness in `tools/replay/` (usage: `testing` skill). Remaining build order:

- [ ] [ready] Save depot: `export-save` scenarios that generate saves, committed under `tools/replay/saves/` with a manifest (how it was made, build, save-format fingerprint); the runner refuses a save whose fingerprint doesn't match. First save: `new_game_to_tv` for both genders.
- [ ] [refine] General save converter: copy fields by name between the old and new layouts, remap IDs by constant name, per-migration hooks for meaning changes. Foundation exists: `tools/save_convert/compare_builds.py` measures both; `expansion_1_17.py` is the hand-written one-off it would replace. Build it with the planned pre-release save break (merge hygiene item 5). Open: hook format.
- [ ] [ready] Screenshot checks: `expect-screen NAME [region ...]` against `tools/replay/golden/NAME.png`, exact pixel match, writes actual + diff on mismatch, `--update-golden` to accept.
- [ ] [ready] Failure diagnostic: decode the last dialogue line on screen from game memory (`charmap.txt`).
- [ ] [refine] Speed: test saves use max text speed and battle animations off; `set-time HH:MM` writes the fake RTC directly; run scenarios in parallel. Open: measure whether skipping frame rendering helps.

Stretch goals (after the above):

- [ ] [idea] Shiny Wally's Ralts: a save with a shiny tutorial Ralts, checking it's still shiny in each later Wally battle.
- [ ] [idea] Quest saves: a `quest QUEST_X == state` condition and saves that walk quests through their states.
- [ ] [idea] DexNav encounters: saves that run DexNav searches and check the encountered species, ability and moves.
- [ ] [idea] Custom moves and abilities: Tier 1 battle tests for each (replays only where they touch the overworld).
- [ ] [idea] Balance sanity: a move-choice policy plays a battle N times with different RNG (shifted input timing) and must win at least X% of the time.
- [ ] [ready] Demo build reverts the testing values (see Pre-Release Checklist).

### Merge hygiene

Cheap fixes from the 1.17.1 upgrade review; details and evidence per item in `.claude/skills/upstream-merge/hygiene-1.17.md`. Procedure for the next upgrade: `upstream-merge` skill.

- [ ] (you: design call) One planned pre-release save/ID break: Aevian species into upstream's custom range, our save data as one struct at the start of SaveBlock3, top-down ID allocation, pinned save sizes (item 5).

### Optional

- [ ] [refine] Route 104 weather cycle prototype (sun by day, rain by night; see `BALANCE.md` Area Weather Cycles). Uses `setstartingstatus` (expansion 1.17). Can start without the environmental-weather restore behavior (Demo 1). Open: NPC tutorial dialogue.

## Demo 1

Ends at the **Weather Institute on both routes**, the minimum for a real evaluation, since the anti-hero route gets there so much faster. Hero: after Norman (5 badges, vanilla order). Anti-hero: Wattson → Flannery → Norman (3 badges), then Surf to Route 119. Includes everything in Pre-Demo.

### Story: anti-hero

- [ ] [refine] Anti-hero Flannery and Norman: badge-count dialogue and arrival beats (lines in `DIVERGENCE_THEORY.md` gym order).
- [ ] [idea] (you: design call) Mt. Chimney on the anti-hero route: Aqua, Magma or both (open question in `DIVERGENCE_THEORY.md`).
- [ ] [refine] Rubicon world state after Wattson: grunt removed from the side passage for good, Roxanne's gym dialogue switches to the secret-final-boss waiting state, Devon switches to "we never recovered the goods".
- [ ] [ready] Cut HM audit (Petalburg Woods through Route 121): nothing story-critical behind Cut for anti-hero players before Gym 8; flavor can stay gated; document what's gated for the post-climax "Cut unlocks Hoenn" phase.

### Weather

- [ ] [refine] Environmental weather layer (`BALANCE.md` Weather → Duration): the area weather set at battle start lasts until replaced or dispelled, and comes back when a temporary move/ability weather wears off. Custom battle code on top of `setstartingstatus`. Open: what dispels it for good.
- [ ] [refine] Area weather cycles for the areas reached by the Weather Institute, after the Route 104 prototype (`BALANCE.md` Area Weather Cycles).
- [ ] [refine] Weather counterplay timeline: when players get weather setters (Pelipper ~L25, others?), when enemies start using weather, when counters (Utility Umbrella, Cloud Nine/Air Lock users like Psyduck, Safety Goggles) become available. Feeds item placement.

### Encounters

- [ ] [refine] Granite Cave (hero)
- [ ] [refine] Dewford (hero)
- [ ] [refine] Route 109 fishing (hero)
- [ ] [refine] Route 110 (hero)
- [ ] [refine] Slateport area (hero)
- [ ] [idea] Encounter and trainer passes for the remaining areas up to Route 119 on both routes (break down per area when we get there)

### DexNav

- [ ] [idea] Silhouettes for all species. Open: where they show and what's shown.
- [ ] [idea] Search for species that have only been seen, with penalties. Open: which penalties.
- [ ] [idea] Per-species search levels (`USE_DEXNAV_SEARCH_LEVELS`). Open: costs ~1 save byte per species.

## Later

- [ ] [refine] Mauville Game Corner starter-doll prize: Axew and Deino dolls. Blocked on art (`ART_ASSETS.md`); remove the dead Mudkip-doll branch then.
- [ ] [ready] Wally's Ralts gender split: 12 trainer definitions (6 battles × 2 genders) for Gardevoir/Gallade, plus a gender check in the map scripts. Persistence is done; this only matters once Wally's Ralts evolves.
- [ ] [idea] Buff vanilla early-route Pokemon so they stay competitive with the additions (avoid "old mon bad, new mon good"). Surskit/Masquerain is the clearest target; also Poochyena, Zigzagoon, Wurmple, Lotad, Seedot. First check what expansion already buffed. Options: stats, abilities, learnsets, evolutions.
- [ ] [idea] Spread weather abilities: more setters (Drizzle/Drought/Sand Stream/Snow Warning) and Swift Swim/Chlorophyll/Sand Rush/Slush Rush or defensive weather abilities on underused Pokemon (`BALANCE.md` Weather design goals).
- [ ] [ready] Sunflora rework (spec in `BALANCE.md` Pokemon Reworks). No home yet; it gets placed when an area calls for it.
- [ ] [idea] Castform buff: +1 Atk/SpA/Spe on form change? Rewards weather chaos.
- [ ] [idea] Weather cycle polish: overworld particles/tints, DexNav weather icon, NPC dialogue variation.
- [ ] [refine] Azurill at Serene Pond or the Oldale Ruins water area, in addition to Route 102.
- [ ] [idea] Oldale footprint guy: keep the gag for now; later he's found REAL footprints ("30 years and FINALLY"), leading to a rare encounter or hidden area.
- [ ] [idea] Oldale Ruins expansion (inspired by anime EP276 "A Ruin with a View"):
  - "Living fossils" theme: Pokemon unchanged since ancient times (Relicanth, etc.)
  - Four keys found throughout Hoenn open a sealed inner door; could tie to weather, story beats or Aqua/Magma
  - Main chamber murals: ancient weather war, Groudon/Kyogre, Rayquaza foreshadowing
  - Time-locked secret: sunrise/morning reveals a hidden staircase (time-of-day system)
  - Underground lake with a Relicanth static encounter as the puzzle reward
  - Puzzle geometry ideas: Penrose stairs, mirror worlds, wraparound maps (HyperRogue)
- [ ] [ready] Upstream PR for the `GetStarterPokemon` off-by-one (upstream 1.17.1 has it too).

### Merge hygiene (before the next expansion upgrade)

Larger items from `.claude/skills/upstream-merge/hygiene-1.17.md`, each moving our code out of upstream's files:

- [ ] [ready] Scripted battles out of upstream's string IDs, trainer slides, battle-type bit and battle scripts (item 10).
- [ ] [ready] Quest script commands as `callnative` instead of opcodes; fixes quest state being written during script dry runs (item 11).
- [ ] [ready] Our functions in our own files: `verdant_items.c`, `wally_ralts.c`, testing catch boost behind one call (item 12).
- [ ] [ready] Scripted controller tracked as a fork of the recorded-battle controllers, with a diff step in the merge workflow (item 13).
- [ ] [ready] Our includes as one block per upstream file; flag/var aliases in `include/constants/verdant_flags.h` (item 14).
- [ ] [ready] `include/verdant_compat.h` wrappers for upstream APIs that keep changing (item 16).
- [ ] [ready] Upstream PRs: Wally tutorial back-pic fix, Rare Candy consumability, shared recorded-controller intro helpers. Fix our Rare Candy evolve path consuming an Infinite Candy now (item 17).
- [ ] [ready] Our map scripts in `data/verdant_scripts.s`; Berry Farmer's House into an existing map group with the save break (item 18).
- [ ] [idea] Spike: play scripted battles through upstream's recorded-battle playback (item 19).
- [ ] (you: design call) Where custom trainers go before adding the first: reuse vanilla IDs or raise the trainer count in the save break (item 20).
- [ ] [ready] Hold B during ball shakes (childhood myth), approach B below.

  Today the B-button boost only applies when the ball is selected, because the catch is calculated before the animation.
  - **Remove:** the B-button check in `Cmd_handleballthrow` (`src/battle_script_commands.c:13747-13749`).
  - **Architecture:** `Cmd_handleballthrow` calculates odds and the shake result (0-3 or SUCCESS); `gBattleSpritesDataPtr->animationData->ballThrowCaseId` stores it; in `battle_anim_throw.c`, `SpriteCB_Ball_Release` is the breakout and `SpriteCB_Ball_Capture` the catch.
  - **Approach A (check at breakout):** in `SpriteCB_Ball_Release` (line 1495), if B is held, recalculate with the bonus and re-roll; on success swap to `SpriteCB_Ball_Capture` and redirect `gBattlescriptCurrInstr` to the success script (tricky).
  - **Approach B (preferred, track B during shakes):** in the `BALL_WAIT_NEXT_SHAKE` case (lines 1468-1482) check B each shake and set a flag (new animationData field?); at the final shake, if the flag is set and the throw would fail, apply the bonus and re-roll. Feels like mashing B, which matches the myth.
  - **Complications:** the battle script path is set before the animation and must be redirected on success; the original catch parameters are needed for the recalculation; edge cases (critical captures, Safari Zone).
  - **Effort:** medium, ~20-40 lines across 2 files.

## Pre-Release Checklist

- [ ] Revert shiny odds to normal (`SHINY_ODDS` at `include/constants/pokemon.h:107` - change 21845 back to 8)
- [ ] Reduce B button catch boost to ~1% (`src/battle_script_commands.c:8175` in `Cmd_handleballthrow` - change `odds *= 10` to `odds = (odds * 101) / 100`)

## Done

What exists today, one line per feature. Details live in git history and the design docs.

- Starter swap: Axew/Deino pair (Axew line Dragon/Steel, evolution levels, early learnsets in `gen_9.h`)
- Petalburg Woods rewrite + Route 104 Devon Goods theft (v2), sets `VAR_PETALBURG_WOODS_STATE` = 2
- Time-of-day encounters: Routes 101-103, Serene Pond, Oldale Ruins (tables in `BALANCE.md`), fake RTC at 60x for testing
- Oldale Ruins area and Berry Farmer's House; Oldale girl hints at the berry quest
- Quest menu (ported from [ghoulslash/pokeemerald quest-menu](https://github.com/ghoulslash/pokeemerald/tree/pokemon_unbound/quest-menu)) and quests: Berry Thief (Phantump cutscene, farmer's berry shop), Field Research
- DexNav enabled (flags/vars assigned, granted in Birch's lab, shown in the start menu)
- Wally's Ralts persistence (SaveBlock3; applied to every later Wally battle)
- Aevian Magikarp and Gyarados regional variants
- Items: EV Trainer (IV editing behind a flag), Wimpy Brace, Exp Share All; Oldale Macho/Wimpy Brace gifts
- QoL: compact start menu, auto-run toggle (hold B to walk), notebook config, Gen 8 Pokemon enabled
- Machop learnset tune (Pound at L1, Low Kick at L6)
- Vanilla Emerald walkthrough (`docs/VANILLA_WALKTHROUGH.txt`) and lookup skill
- Testing: `testing` skill; customization tests (`test/verdant/customizations.c`: starters, learnset and evolution edits, Axew typing, Aevian forms, custom items, time-of-day encounter tables); replay harness (headless runner, scenario compiler, intro routines, battle transcripts) with the TV broadcast test for both genders, checking Norman's replay against a reviewed golden transcript; unrigged battle tests (real RNG) with the Route 103 rival guard and win-rate measurement suite
- Expansion 1.17.1 (from 1.14.1); our config values in `include/config/verdant.h`; merge procedure and reusable workflow (`upstream-merge` skill)
- Merge hygiene from the 1.17.1 review (`hygiene-1.17.md`): pre-merge check for generated files (7), empty item IDs named so `test/text.c` passes (8), Wimpy Brace without a hold effect of its own (6), no-op edits to upstream lines reverted and our rewrites turned into added lines (3), notebook Exp. Share menu via `dynmultichoice` with a replay test (4), ID registry in the `save-system` skill and `src/verdant_asserts.c` (9)
- Fixes: scripted battle crash after the upstream merge, quest state on new game, Wally's silhouette sprite, follower/DexNav flag clash, quest menu out-of-bounds sprite write
