# Backlog

High-level tasks to implement.

## To Do

- [ ] Enable DexNav
  - Set `DEXNAV_ENABLED` to `TRUE` in `include/config/dexnav.h`
  - Assign flags/vars (`DN_FLAG_*`, `DN_VAR_*`)
  - Optionally enable `USE_DEXNAV_SEARCH_LEVELS` for per-species tracking
  - Grant access via script (or deduce from `FLAG_RECEIVED_POKEDEX`)
  - Consider custom modifications: show all species as silhouettes, allow search when seen (with penalties)

- [ ] Add Quest Menu (Unbound-style)
  - Port from: https://github.com/pret/pokeemerald/compare/master...PokemonSanFran:pokeemerald:unbound-quest-menu
  - ~3,500 lines, 21 files - main implementation in `src/quests.c`
  - Evaluation steps:
    1. Clone branch locally, review `global.h` changes for save data conflicts with pokeemerald-expansion
    2. Check if `start_menu.c` and `scrcmd.c` hooks are compatible
    3. Test graphics assets work as-is
  - Features: quest states (locked/active/reward/complete), subquests, filtering, favorites
  - Adds 3 new script commands for quest control

- [ ] Quest: "The Berry Thief" (Phantump)
  - Location: Farmer in Oldale, Phantump on Route 101 near berry trees
  - Flow: Farmer complains about missing berries → investigate → find Phantump hoarding berries
  - Resolution: Phantump joins party holding Oran Berry, optional farmer reward
  - Tie-in: Harvest ability (thematic), Ghost/Grass typing helps Roggenrola players vs Machop rival
  - Tone: Light, comedic - caught red-handed, not malicious
  - Requires: Quest menu system (or can be standalone with just flags)

- [ ] Early Route Pokemon Additions (balance investigation)
  - Goal: Give each starter player options to cover weaknesses
  - **Routes/rates need careful review** - placeholder values below
  - Candidates:
    - Skwovet (Normal, Cheek Pouch) - Route 101? forest squirrel vibe
    - Cottonee (Grass/Fairy, Prankster) - Route 101 or 102? universal support via status moves
    - Wooper (Water/Ground, Unaware/Water Absorb) - Route 102 pond or 103 coastal? Ground STAB for Pikipek vs Rock
  - Balance considerations:
    - Pikipek player needs Ground coverage (Roxanne, Rock rival)
    - Machop player needs... ?
    - Roggenrola player needs... ?
  - TODO: Review current encounter tables, map out type coverage gaps per starter

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

- [ ] Make Wally's Ralts "real"
  - Generate real mon data during catch tutorial (IVs, nature, gender, PID, shiny)
  - Persist in save (~10 bytes: PID + IVs + shiny flag)
  - Carry forward to all Wally battles - his Gardevoir/Gallade uses these stats
  - Gender determines evo: Male → Gallade, Female → Gardevoir
  - Shiny sparkle shows during catch scene (immediate feedback)
  - Fun implications:
    - Community shiny hunting for Wally becomes a thing
    - "I gave Wally a shiny" wholesome flex
    - Every playthrough his ace is slightly different
    - Makes catch tutorial feel like it mattered
  - Don't expose stats to player - let it be a mystery why his Pokemon hits different

- [ ] Oldale Town Enhancements
  - Mart potion guy: "Berries have been scarce lately, these are in demand" → gives potion
    - Subtle setup for Phantump berry thief quest, rewards attentive players
  - Footprint guy: Keep the gag initially, bring back later
    - He's found REAL footprints somewhere, needs help investigating
    - Could lead to rare encounter or hidden area
    - "30 years and FINALLY" energy
  - Oldale Ruins (anime tie-in): New area north of town
    - Locked initially (Strength? Rock Smash? Story flag?)
    - Ancient Pokemon puzzles, ties into Hoenn archaeology (Relicanth, Regis)
    - Could be weather-related? Ancient weather shrine?
    - Early tease, return later - "milestone 2" content

## Quick Notes

- Buff Castform: +1 Atk/SpAtk/Speed on form change? Rewards weather chaos, fits the weather-heavy design pillar
- Non-Euclidean geometry ideas for ruins/puzzles: Penrose stairs, mirror worlds, wraparound maps (inspired by HyperRogue)

## Pre-Release Checklist

- [ ] Revert shiny odds to normal (`include/constants/pokemon.h:96` - change 21845 back to 8)
- [ ] Reduce B button catch boost to ~1% (`src/battle_script_commands.c:15919` - change `odds *= 10` to `odds = (odds * 101) / 100`)

## Small Features

- [ ] Auto-run toggle - Hold B to walk instead of run (invert check in `src/field_player_avatar.c:685`)
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

