# Backlog

High-level tasks to implement.

## To Do

- [ ] Enable DexNav
  - Set `DEXNAV_ENABLED` to `TRUE` in `include/config/dexnav.h`
  - Assign flags/vars (`DN_FLAG_*`, `DN_VAR_*`)
  - Optionally enable `USE_DEXNAV_SEARCH_LEVELS` for per-species tracking
  - Grant access via script (or deduce from `FLAG_RECEIVED_POKEDEX`)
  - Consider custom modifications: show all species as silhouettes, allow search when seen (with penalties)

- [ ] Early Route Pokemon Additions (in progress)
  - Goal: Give each starter player options to cover weaknesses
  - **Done:**
    - Skwovet - Route 101 day (20%)
    - Wooper - Route 103 night (20%) - Ground for Pikipek vs Roxanne
    - Azurill - Route 102 day (9%) - Huge Power payoff
    - Oddish/Hoothoot - Route 102 night (20% each)
  - **Remaining:** Route 104, Petalburg Woods, etc.

- [ ] Buff existing early route Pokemon
  - Goal: Make vanilla Gen 3 mons competitive with new additions - avoid "old mon bad, new mon good"
  - Candidates (Routes 101-103):
    - Poochyena/Mightyena - ?
    - Zigzagoon/Linoone - already has Pickup utility, maybe enough?
    - Wurmple line - probably hopeless? or lean into early game niche
    - Lotad line - ?
    - Seedot line - ?
    - Wingull/Pelipper - Drizzle already strong (Gen 7+), maybe fine
    - Ralts line - already popular, probably fine
    - Surskit/Masquerain - notoriously underwhelming, good buff target
  - Options: stat tweaks, better abilities, earlier/better learnsets, new evolutions?
  - TODO: Review what pokeemerald-expansion already changed for these (may have Gen 6+ buffs baked in)

- [ ] Weather system balance pass
  - Goal: Weather prominent but not mandatory - clear counterplay options
  - Current plan: Permanent ability weather (Gen 3 style), move weather stays 5/8 turns
  - Map out availability timeline:
    - When do players get weather setters? (Pelipper ~25, others?)
    - When do enemies start using weather?
    - When is counterplay available?
  - Counterplay options to make accessible:
    - Utility Umbrella (Gen 8 item) - holder ignores rain/sun effects, give early-ish
    - Cloud Nine / Air Lock users - Psyduck early is good, who else?
    - Safety Goggles - for sand/hail chip
  - Investigate: Give underused mons weather counter/synergy abilities
    - Candidates for Cloud Nine or similar?
    - Mons that could get Swift Swim / Sand Rush / Slush Rush as new ability?
    - Other weather-based buffs to unused Pokemon?
  - Note: Early routes already have rain team skeleton (Wingull, Lotad, Surskit) - consider if that's too easy to assemble

- [ ] Weather as core design pillar
  - Philosophy: Weather is THE system of this hack - players must engage (join or counter)
  - Story arc:
    - Early: Weather emerges, players notice synergies
    - Mid: Weather tyranny - commit to a team or find counterplay
    - Late: Rayquaza = "I'm done with this" button - Air Lock feels EARNED
  - Rival weather identities:
    - Brendan (aggro): Sun early → Rain late (sweeper weathers)
    - May (stall): Sand early → Hail late (defensive weathers)
  - Team Aqua/Magma:
    - Aqua grunts: Rain / Aqua admins: Hail (ice+water theme)
    - Magma grunts: Sun / Magma admins: Sand (ground+fire theme)
  - Gym leaders: Some should use weather strategically (not all)
  - Expand weather ability distribution:
    - Weather setters (Drizzle/Drought/Sand Stream/Snow Warning) are too monopolized
    - Give more mons access to setting abilities - lots of options, not "must-haves"
    - Same for Swift Swim/Chlorophyll/Sand Rush/Slush Rush and defensive options
    - Goal: Many viable weather team cores, not just "Pelipper or bust"
  - Thematic payoff: Groudon/Kyogre war is something player LIVED, not watched
  - Ties into: Weather balance pass, early route mons, buff existing mons

- [ ] Wally's Ralts - gender-based trainer splits
  - Core system implemented (personality/IVs persist, upscaler applies to all Wally battles)
  - TODO: Create 12 trainer definitions (6 battles × 2 genders) for Gardevoir/Gallade split
  - TODO: Add gender check in map scripts to select correct trainer ID

- [ ] Oldale Town Enhancements (partially done)
  - [x] Oldale Ruins area east of Oldale with Berry Farmer's House
  - [x] Girl NPC hints at berry quest ("farmer hasn't been at market")
  - [ ] Mart potion guy: "Berries have been scarce lately, these are in demand" → gives potion
  - [ ] Footprint guy: Keep the gag initially, bring back later
    - He's found REAL footprints somewhere, needs help investigating
    - Could lead to rare encounter or hidden area
    - "30 years and FINALLY" energy

- [ ] Oldale Ruins Expansion (anime-inspired from EP276 "A Ruin with a View")
  - [ ] Azurill encounter - add to Serene Pond or ruins water area
  - [ ] "Living fossils" theme - Pokemon unchanged since ancient times (Relicanth, etc.)
  - [ ] Four keys mechanic - sealed inner door, keys found throughout Hoenn
    - Could tie to weather conditions, story beats, or Team Magma/Aqua encounters
  - [ ] Main chamber with murals - ancient weather war, Groudon/Kyogre, Rayquaza foreshadowing
  - [ ] Time-locked secret - sunrise/morning triggers hidden staircase (uses time-of-day system)
  - [ ] Underground lake - Relicanth static encounter as puzzle reward
  - [ ] Team Magma/Aqua connection - possibly involved in key collection or ruins storyline

- [ ] Sunflora rework (Grass/Fire sun sweeper)
  - **Type:** Grass → Grass/Fire
  - **Abilities:** Chlorophyll / Solar Power / Drought (HA)
  - **Stats:**
    | Stat | Old | New |
    |------|-----|-----|
    | HP | 75 | 95 |
    | Atk | 75 | 55 |
    | Def | 55 | 85 |
    | SpA | 105 | 125 |
    | SpD | 85 | 100 |
    | Spe | 30 | 60 |
    | BST | 425 | 520 |
  - **Level-up (Sunflora):**
    - 0: Sunny Day, Ember, Flower Shield
    - 16: Razor Leaf, 19: Flame Burst, 22: Synthesis, 25: Giga Drain
    - 28: Morning Sun, 31: Fire Spin, 34: Solar Beam, 37: Flamethrower
    - 40: Petal Dance, 43: Heat Wave, 46: Solar Blade, 50: Fire Blast, 55: Overheat
  - **Coverage to add:** Earth Power (TM/tutor), Weather Ball, Sludge Bomb, Dazzling Gleam
  - **Note:** With permanent weather, Chlorophyll = permanent 120 Spe, Solar Power = sustained pressure + HP drain

## Quick Notes

- Buff Castform: +1 Atk/SpAtk/Speed on form change? Rewards weather chaos, fits the weather-heavy design pillar
- Non-Euclidean geometry ideas for ruins/puzzles: Penrose stairs, mirror worlds, wraparound maps (inspired by HyperRogue)

## Pre-Release Checklist

- [ ] Revert shiny odds to normal (`include/constants/pokemon.h:96` - change 21845 back to 8)
- [ ] Reduce B button catch boost to ~1% (`src/battle_script_commands.c:15919` - change `odds *= 10` to `odds = (odds * 101) / 100`)

## Small Features

- [x] Auto-run toggle - Hold B to walk instead of run (`src/field_player_avatar.c:844`)
- [ ] Quest menu: Hide locked quests entirely instead of showing "??????" (modify list-building in `src/quests.c`)

## Feature: Hold B During Ball Shakes (Childhood Myth)

Currently B button catch boost only works when selecting the ball (calculation happens before animation). To make it work during the shake animation like the real myth:

**Current implementation to remove:**
- `src/battle_script_commands.c:15919-15921` - B button check in `Cmd_handleballthrow`

**Architecture:**
- `Cmd_handleballthrow` (battle_script_commands.c) calculates odds, determines shakes (0-3 or SUCCESS), sends to animation
- `gBattleSpritesDataPtr->animationData->ballThrowCaseId` stores predetermined result
- Animation callbacks in `battle_anim_throw.c`:
  - `SpriteCB_Ball_Release` = breakout (failure)
  - `SpriteCB_Ball_Capture` = catch stars/music (success)

**Approach A - Check at breakout moment:**
1. In `SpriteCB_Ball_Release` (battle_anim_throw.c:1494), before breakout animation
2. Check `gMain.heldKeys & B_BUTTON`
3. If held, recalculate odds with bonus, re-roll
4. If success, swap callback to `SpriteCB_Ball_Capture`
5. Also need to change `gBattlescriptCurrInstr` to success script (tricky)

**Approach B - Track B during shakes (preferred):**
1. In `BALL_WAIT_NEXT_SHAKE` case (battle_anim_throw.c:1467-1480), check B each shake
2. Set a flag if B held during any shake (new field in animationData?)
3. At final shake check, if flag set and would fail, apply bonus and re-roll
4. Feels more like "mashing B during shakes" - matches the myth better

**Complications:**
- Battle script path already set before animation - need to redirect on success
- Need access to original catch parameters for recalculation
- Edge cases: critical captures, safari zone, etc.

**Effort:** Medium (~20-40 lines across 2 files)

## Done

- [x] Wally's Ralts persistence system
  - Tutorial Ralts data (personality, IVs, hidden nature) saved to SaveBlock3
  - TryUpscaleWallyRalts() applies saved data to all future Wally battles
  - Supports shiny, gender variance, future mint system
- [x] Route 102 time-of-day encounters
  - Day: Azurill added (9%), Lotad day-only
  - Night: Oddish (20%), Hoothoot (20%) replace Lotad/Azurill
- [x] Add Quest Menu (Unbound-style) - Ported from [ghoulslash/pokeemerald quest-menu](https://github.com/ghoulslash/pokeemerald/tree/pokemon_unbound/quest-menu)
- [x] Wimpy Brace item + Oldale NPC gifts (Macho Brace / Wimpy Brace)
- [x] Machop learnset adjustment (Pound at L1, Low Kick at L6)
- [x] Enable time-based encounters (day/night exclusive Pokemon)
  - Routes 101, 102, 103, Serene Pond, Oldale Ruins have day/night variants
  - Fake RTC at 60x speed for testing
- [x] Quest: "Berry Thief" (Phantump)
  - Old farmer in Oldale Ruins complains about missing Watmel Berries
  - Phantump appears at berry tree at night with full cutscene
  - Guaranteed Harvest ability (hidden ability) - thematic for berry thief
  - Reward: 3 Oran Berries, access to rare Watmel Berry tree
  - Oldale Town girl hints at quest, changes dialogue when complete
