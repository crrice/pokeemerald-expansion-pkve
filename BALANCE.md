
# Pokemon Verdant Emerald

Balance and gameplay notes. A mix of structured balance examination for configured and confirmed changes and overall notes and ramblings on possible balance or gameplay changes.

## Gameplay Pillars

Note for AI:

---

This section is a "notes" section and is subject to frequent change. The following sections (locations and arc analysis) are detailed and should always remain accurate to what is in code. This on the other hand is a repository of broad ideas and thoughts that may at any time be implemented or unimplemented, under review or decided. Usually I note which of these any given thing is.

You should not edit this section unless specifically asked to do so.

You should never add notes on where or how to enable features in this section. This is for broad thoughts and ideas only. In the following major sections it is fine.

---

### Weather

Weather is our main gimmick. Introduced this gen and is the story theme so leaning into the havoc.

#### Duration

All weather is permanent until replaced. Weather wars are committal - you override theirs or live with it.

- Ability-based weather: permanent. Not implemented.
- Move-based weather: permanent. Not implemented.

#### Design Goals

- Story arc: weather emerges early, becomes oppressive mid-game ("weather tyranny"), Rayquaza is the earned relief.
- Counterplay accessible: Utility Umbrella, Cloud Nine/Air Lock users, Safety Goggles available at reasonable points. [TODO: Place these items]
- Expand availability of weather setters beyond traditional 2-3 (or often just 1 viable) mons. [TODO]
- Expand availability of weather-related abilities (Swift Swim, Chlorophyll, Sand Rush, etc). [TODO]
- Improve weak weather abilities (Castform's Forecast is underwhelming). [TODO]
- Consider modifying existing abilities for weather interplay. [TODO: Brainstorm examples]

#### Trainer Themes

- Rivals have weather identities: Brendan (aggro) uses Sun→Rain, May (stall) uses Sand→Hail. [TODO]
- Rival teams vary based on starter choice? If May gets Roggenrola, a sand team makes thematic sense. [TODO: Design]
- Teams Aqua/Magma themed around Rain/Hail and Sun/Sand respectively. [TODO]

#### Future Ideas

BIG IDEA: Consider having weather per area in the save data. Make weather last "a day" if set by ability? Have it affect berries in different ways. If in battle weather affects the field, can be cool for puzzles/quests/etc as well in the OW?

### Berries

Gen 3 massively expanded the berry system. And I love it besides.

#### Design Goals

- Berry-related abilities distributed wider.
- Good berries (stat-boosting, resist) are rare or quest-locked. Player "collection" grows over game.

#### Berry Farm

- Add a "berry farm" area player can use.
- Seed bank NPC: Bring them 1 berry of X type, and it becomes available as a farm option. [TODO]
- Farm can be partially automated for some cost per berry? Maybe tie to seed bank. [TODO]

#### Future Ideas

- Which berries should be NON-farmable? Stat berries (Liechi, Petaya, etc.) as contest/rare rewards only?
- Weather affecting berry growth rates in the overworld?

### Double Battles

Gen 3 introduced doubles. Emerald expanded on it. We're going further.

#### Philosophy

- Singles remains primary, but doubles are common and meaningful - not a gimmick.
- **Every mon is viable** - need doubles for this.

#### Design Goals

- Themed double teams on trainers - weather cores, Trick Room pairs, ability synergies.
- More route trainers use doubles.
- Double wild encounters.
- Some gym leaders use doubles format.
- Rival doubles battles at key story points.
- Team Aqua/Magma sometimes will have admins/boss run pairs.
- Multi Battles with NPC partners available (need to make sure the partner AI isn't rage inducing tho).
- Ways to practice/learn doubles before competitive content.

### Contests

Gen 3 only again. And they were fun. And they were kinda relevant for feebass lmao.

- Ties well with berries.
- Add better rewards. Make some highly sought competitive items come from this. Maybe even some mons themselves?
- Improve AI one day for "championship" contest or such.

### Secret Bases

Secret bases were a gimmick only gen 3 had and they were pretty cool.

- Make the parts for them far more common.
- Add some use for them. PC/Heal inside?
- Have your rival / other trainers stop by?
- Defend your base! If you are in it for a while, perhaps someone comes along and battles you!

### Side Content

Quests, exploration, and optional content should feel rewarding and meaningful.

- Quest system (Unbound-style) tracks player progress on optional objectives.
- Quest rewards matter, should give unique gameplay options for rewards, but NOT trivialize.
- Time-of-day creates exploration variety and strategic choices.
- Hidden areas and secrets tied to story progression, weather, or time.
- Reward backtracking when new OW access tools become available.
- Add _style_, _mystique_, _wonder_, to the world. Take inspiration from other pokemon media such as anime.

### Mega Evolution

My favorite gimmick and the easiest to balance (maybe) and most fun IMO. No other gen gimmicks (no tera, no Z moves, etc).

- Faithful to ORAS where possible.
- Enhance enemy trainer mega use.
- Add mega stones from later gens.
- Should be collecting stones from the early game.
- Should be using mega evos from gym 4 onward.
- Should have a lot of stones by gym 6-7.

### QOL

I hate grinding. But I like RP. Overall focus on letting the player customize their own experience.

- Don't make EVs/IVs hard to optimize toward the mid/endgame. Should be nearly freely editable (evs) or have a method to get repeat bottlecaps (ivs).
- Overall make them easier to manage earlier than vanilla.
- And matter more than vanilla.
- Thinking a "$100 per EV changed" item avail early on.
- Also make a "free edit EV/IV/nature" item avail in PC for players who just don't want to grind at all?

- Allow move relearn in menu at some point?
- Same for tutor moves / egg moves? May tie to quest?

- Shiny charm available in PC.
- Second one available around gym 3-4.
- Third around 7-8. Fourth for finished dex.

- Dexnav allows search for seen (uncaught) mons. Shows all mons in area regardless of seen status.
- Investigate ways to have it show day/night mons aside from literally having to wait for day/night and recheck it. Color code? Other symbol? Idk.

- Infinite candy item avail (repeatable rare candy).
- No disobey on overlevel.
- No level cap (except 100 lol).

## Locations

### Littleroot

Starters: **Axew** (Dragon/Steel) and **Deino** (Dark/Dragon). A pair, not a trio.

#### Design Philosophy

**Late Growers.** Both lines are pseudo-legendaries. Mid-evo lands around Gym 4-5, final form around Gym 6-7. The starter is a long-term project — the early/mid game is carried by the *team*, not the starter. This is the opposite of the classic "starter snowballs into the ace" pattern; here, wild encounters on Routes 101-104 earn permanent team slots.

**No type triangle.** Both are Dragon-typed. Rival tension shifts from starter matchup to *weather identity* (Brendan aggro sun/rain, May stall sand/hail). Rival takes the other starter (mirror-match rivalry throughout the game).

#### Axew Line (Dragon/Steel)

*Typing rationale*: Haxorus visually reads as armored/steel — axe-face, tusks, plating. Dragon/Steel neutralizes the canonical Ice/Fairy 4x weaknesses and creates a defensive bruiser profile (resists Normal, Flying, Rock, Bug, Steel, Grass, Electric, Psychic, Dragon; immune to Poison). Only weaknesses: Fighting, Ground, Fire. Heavy defensive profile is intentional — Haxorus is meant to carry into L80s content.

*Evolution*: Axew → Fraxure @ L46 → Haxorus @ L68. Steady climb.

*Identity*: Physical attacker. Steady, honorable, reliable. "The hero starter" in vibe, not in route gating.

*Target learnset adjustments*:
- L3-5: **Assurance** — early non-STAB Dark coverage, double power if opponent moved. Thematic vs Deino mirror match.
- L12-15: **Metal Claw** — 50 BP Steel STAB, 10% Atk-up. The Gym 1 (Roxanne) unlock — 2x SE on Rock, addictive chain.
- L18-20: **Dual Chop** — 40 BP × 2, Dragon STAB, Sub/Sturdy breaker. Online for Gym 2.
- Keep Dragon Rage at L10 (40 flat damage scales out well).

*Gym matchup notes*:
- Gym 1 (Roxanne, Rock): **Dominates.** Steel STAB + Rock resistance.
- Gym 2 (Brawly, Fighting): **2x weak.** Must rely on Dual Chop neutral damage + team support (Wingull, Abra, Ralts).

#### Deino Line (Dark/Dragon)

*Typing*: Canonical Dark/Dragon throughout evolution.

*Evolution*: Deino → Zweilous @ L48 → Hydreigon @ L72. Late apotheosis — long middle stage.

*Identity*: Special attacker (Hydreigon). "The anti-hero starter" in vibe. Long climb, massive payoff.

*Early-game problem*: Hustle + limited moveset through L48 is rough. Leaning on minimal learnset buffs rather than ability/stat intervention — the payoff story reads better when the climb is real.

*Target learnset adjustments*:
- L5-6: **Bite** (pulled from L9) — earlier Dark STAB, thematic ("learns by biting").
- L12-15: Headbutt or Assurance for non-STAB coverage.
- L18-20: **Crunch** — natural learn level, keep. *The Gym 2 answer* — 2x SE on Meditite (Psychic).
- L20-22: **Dragon Breath** (pulled from L32) — Dragon STAB online by Gym 2, paralysis chance.

*Gym matchup notes*:
- Gym 1 (Roxanne, Rock): **Neutral-negative.** No SE STAB on Rock. Must lean on team (Wooper, Lotad, Ralts).
- Gym 2 (Brawly, Fighting): **2x weak** but **Crunch is a legit answer vs Meditite** (Psychic 2x SE). Pure Fighting still walls — team support needed.

#### Open Questions

- Does Dragon/Steel Haxorus need a BST nerf to compensate for the defensive profile upgrade? Deferred.
- Does Hydreigon need any special-attacker kit tweaks? Deferred.
- Should starter Deino get an advantage (egg move, hidden ability unlock) to smooth its long early game? Pending playtesting.
- Old starters Pikipek/Machop/Roggenrola need wild/gift placement (see BACKLOG).

### Route 101

Levels 2-3. First route, traversed before pokeballs. Player sees these but can't catch until later.

#### Encounters

| Pokemon | Type | Day % | Night % | Abilities |
|---------|------|-------|---------|-----------|
| Wurmple | Bug | 40% | 40% | Shield Dust / Run Away |
| Zigzagoon | Normal | 30% | 10% | Pickup / Gluttony |
| Skwovet | Normal | 20% | - | Cheek Pouch / Gluttony (HA) |
| Poochyena | Dark | 10% | 30% | Run Away / Quick Feet / Rattled (HA) |
| Spinarak | Bug/Poison | - | 20% | Swarm / Insomnia / Sniper (HA) |

#### Balance Notes

**Wurmple** [PLANNED]

*Vanilla Assessment*: Filler. Cascoon/Silcoon split gives early bug variety but neither evo line is competitively relevant. Beautifly is a poor man's Butterfree. Dustox has okay bulk but bad offensive stats and shallow movepool. Both peak early and become dead weight.

*Target Niche*: Duality theme. Beautifly = ally support. Dustox = enemy debuff. Gen 3 mon, gets extra attention.

*Beautifly Changes*:
- Ability: Add **Triage** (priority healing moves) as Ability 2
- Movepool: Add **Pollen Puff** (level-up ~28) - heals allies or damages enemies, core to Triage identity
- Movepool: Add **Tailwind** (level-up ~35) - speed control, doubles support

*Dustox Changes*:
- TODO

**Zigzagoon**: Pickup utility makes it worth catching for item farming. Linoone is a serviceable HM user. More common during day.

**Skwovet**: Day-exclusive. Cheek Pouch synergizes with berry pillar - heals extra when consuming berries. Greedent is bulky but slow. First berry-focused mon available.

**Poochyena**: More common at night. Mightyena is underwhelming but Intimidate on evolution is useful for doubles/switching. Quick Feet + status orb is a niche strat.

**Spinarak**: Night-exclusive. Bug/Poison is unique typing this early. Ariados is weak but Insomnia has utility vs sleep users. Sniper HA could be interesting with high-crit moves.

### Route 103

Levels 2-4. Rival battle location. First catches happen here after receiving pokeballs.

#### Encounters

| Pokemon | Type | Day % | Night % | Abilities |
|---------|------|-------|---------|-----------|
| Poochyena | Dark | 40% | 40% | Run Away / Quick Feet / Rattled (HA) |
| Wingull | Water/Flying | 30% | 10% | Keen Eye / Hydration / Rain Dish (HA) |
| Ducklett | Water/Flying | 20% | - | Keen Eye / Big Pecks / Hydration (HA) |
| Zigzagoon | Normal | 10% | 30% | Pickup / Gluttony |
| Wooper | Water/Ground | - | 20% | Water Absorb / Damp / Unaware (HA) |

Water/Fishing encounters exist but require Surf/Rods not available at this point.

#### Balance Notes

**Poochyena**: Common both times. See Route 101 notes.

**Wingull**: Key pokemon. Pelipper gets Drizzle - core to weather pillar. Available early but evolution is mid-game. Rain Dish HA for self-sustain in rain. More common during day.

**Ducklett**: Day-exclusive. Water/Flying like Wingull but evolves into Swanna (no Drizzle). Hydration HA cures status in rain - pairs with weather pillar but competes with Pelipper for team slot.

**Zigzagoon**: See Route 101 notes. More common at night here.

**Wooper**: Night-exclusive. Water/Ground is excellent defensive typing. Only Ground-type available pre-Petalburg. **Particularly valuable for Deino players vs Roxanne** — Deino is neutral into Rock and needs team-based answers; Wooper is one of the best available. Unaware HA ignores stat boosts, strong vs setup sweepers. Quagsire is a solid long-term wall.

### Serene Pond

Levels 2-3. Hidden area accessible from Route 101. Requires Cut - player enters from southeast, traverses anti-clockwise through cut trees and grass to reach the pond at the southwest. One-way ledge in the south provides a quick exit. Backtracking reward after Roxanne.

#### Encounters

| Pokemon | Type | Day % | Night % | Abilities |
|---------|------|-------|---------|-----------|
| Wurmple | Bug | 40% | 40% | Shield Dust / Run Away |
| Zigzagoon | Normal | 30% | 10% | Pickup / Gluttony |
| Skwovet | Normal | 20% | - | Cheek Pouch / Gluttony (HA) |
| Poochyena | Dark | 10% | 30% | Run Away / Quick Feet / Rattled (HA) |
| Spinarak | Bug/Poison | - | 20% | Swarm / Insomnia / Sniper (HA) |

#### Fishing (Old Rod+)

| Pokemon | Type | Rate | Abilities |
|---------|------|------|-----------|
| Magikarp (Aevian) | Fairy/Dragon | 100% | Swift Swim / Rattled (HA) |

#### Balance Notes

Land encounters are identical to Route 101. The unique draw is **Aevian Magikarp** - the only location to catch it. This regional variant evolves into Aevian Gyarados (Fire/Dragon), a significant divergence from standard Gyarados.

Since Cut is required, this is a backtracking reward after Roxanne. Players who remember this pond can return for an exclusive catch.

### Oldale Ruins

Levels 3-4. Optional area east of Oldale Town. Home to Berry Thief quest.

#### Encounters

| Pokemon | Type | Day % | Night % | Abilities |
|---------|------|-------|---------|-----------|
| Zigzagoon | Normal | 40% | 40% | Pickup / Gluttony |
| Wurmple | Bug | 40% | - | Shield Dust / Run Away |
| Burmy | Bug | 20% | - | Shed Skin / Overcoat (HA) |
| Spinarak | Bug/Poison | - | 30% | Swarm / Insomnia / Sniper (HA) |
| Poochyena | Dark | - | 20% | Run Away / Quick Feet / Rattled (HA) |
| Phantump | Ghost/Grass | - | 10% | Natural Cure / Frisk / Harvest (HA) |

#### Special: Berry Thief Quest

Phantump available at Lv 5 with **guaranteed Harvest ability** via the Berry Thief quest.

#### Balance Notes

**Zigzagoon/Wurmple**: Filler. See Route 101 notes.

**Burmy**: Day-exclusive. Form changes based on last battle location (Plant/Sandy/Trash Cloak). Evolves into Wormadam (female, type varies by cloak) or Mothim (male, Bug/Flying). Niche but unique mechanic.

**Spinarak**: See Route 101 notes. Higher rate here at night (30% vs 20%).

**Poochyena**: Night only here. See Route 101 notes.

**Phantump**: Night-exclusive, rare (10%). Ghost/Grass is unique typing - immune to Normal/Fighting, resists Water/Grass/Electric/Ground. Wild ones have Natural Cure or Frisk. Quest version with Harvest is far superior. Trevenant evolution is a solid physical attacker.

### Route 102

Levels 3-4. First "full" route after getting Pokeballs. Connects Oldale to Petalburg. Has a small pond (Surf/fishing for backtracking).

#### Day Encounters

| Pokemon | Type | Day % | Abilities |
|---------|------|-------|-----------|
| Poochyena | Dark | 30% | Run Away / Quick Feet / Rattled (HA) |
| Wurmple | Bug | 20% | Shield Dust / Run Away |
| Lotad | Water/Grass | 20% | Swift Swim / Rain Dish / Own Tempo (HA) |
| Zigzagoon | Normal | 16% | Pickup / Gluttony |
| Azurill | Normal/Fairy | 9% | Thick Fat / Huge Power / Sap Sipper (HA) |
| Ralts | Psychic/Fairy | 4% | Synchronize / Trace / Telepathy (HA) |
| Seedot | Grass | 1% | Chlorophyll / Early Bird / Pickpocket (HA) |

#### Night Encounters

| Pokemon | Type | Night % | Abilities |
|---------|------|---------|-----------|
| Poochyena | Dark | 20% | Run Away / Quick Feet / Rattled (HA) |
| Wurmple | Bug | 20% | Shield Dust / Run Away |
| Oddish | Grass/Poison | 20% | Chlorophyll / Run Away (HA) |
| Hoothoot | Normal/Flying | 20% | Insomnia / Keen Eye / Tinted Lens (HA) |
| Zigzagoon | Normal | 15% | Pickup / Gluttony |
| Ralts | Psychic/Fairy | 4% | Synchronize / Trace / Telepathy (HA) |
| Seedot | Grass | 1% | Chlorophyll / Early Bird / Pickpocket (HA) |

#### Balance Notes

**Azurill**: Day-exclusive. Huge Power doubles Attack - Azumarill with this ability is a legitimate threat. Long-term investment for players who recognize the potential. Happiness evolution means early effort pays off.

**Oddish**: Night-exclusive. Grass/Poison provides unique defensive typing this early. Vileplume/Bellossom evolution split offers choice. Chlorophyll ties to weather pillar (sun doubles Speed).

**Hoothoot**: Night-exclusive. Insomnia prevents sleep - useful utility. Tinted Lens HA makes resisted moves hit neutrally, surprisingly powerful on Noctowl. Gives Route 102 a distinct nocturnal identity.

**Lotad**: Day-only now. Swift Swim + Rain Dish ties to weather pillar. Ludicolo is a fun rain sweeper. Counterpart to Oddish at night.

**Ralts**: Kept at 4% - the rare encounter that makes the route memorable. Wally's Ralts connection adds narrative weight.

### Route 104

Levels 6-9. Coastal route north of Petalburg, transitioning into Petalburg Woods. Dual biome (grass north / beach south) treated as a single encounter pool. **First area with a permanent weather cycle** — the tutorial biome for the weather system.

#### Weather Cycle

- **Day: Sun** (permanent, set at battle start, overridable by ability/move weather)
- **Night: Rain** (permanent, same rules)
- Framing: coastal convection — hot dry days, wet nights. Natural Hoenn geography, not Aqua/Magma interference.
- Tutorial: NPC dialogue in Petalburg ("Pack a poncho — always rains after sundown on 104") + player observation in first battle.
- Encounter tables are **not** affected by the cycle — encounters are time-of-day only. Weather and encounters are independent axes.

#### Day Encounters

| Pokemon | Type | Day % | Abilities |
|---------|------|-------|-----------|
| Krabby | Water | 25% | Hyper Cutter / Shell Armor / Sheer Force (HA) |
| Wingull | Water/Flying | 20% | Keen Eye / Hydration / Rain Dish (HA) |
| Dwebble | Bug/Rock | 15% | Sturdy / Shell Armor / Weak Armor (HA) |
| Marill | Water/Fairy | 10% | Thick Fat / Huge Power / Sap Sipper (HA) |
| Taillow | Flying | 10% | Guts / Scrappy (HA) |
| Wurmple | Bug | 10% | Shield Dust / Run Away |
| Poochyena | Dark | 4% | Run Away / Quick Feet / Rattled (HA) |
| Zigzagoon | Normal | 4% | Pickup / Gluttony |
| Pincurchin | Electric | 2% | Lightning Rod / Electric Surge (HA) |

#### Night Encounters

| Pokemon | Type | Night % | Abilities |
|---------|------|---------|-----------|
| Krabby | Water | 30% | Hyper Cutter / Shell Armor / Sheer Force (HA) |
| Dwebble | Bug/Rock | 24% | Sturdy / Shell Armor / Weak Armor (HA) |
| Wingull | Water/Flying | 10% | Keen Eye / Hydration / Rain Dish (HA) |
| Marill | Water/Fairy | 10% | Thick Fat / Huge Power / Sap Sipper (HA) |
| Spinarak | Bug/Poison | 10% | Swarm / Insomnia / Sniper (HA) |
| Poochyena | Dark | 10% | Run Away / Quick Feet / Rattled (HA) |
| Zigzagoon | Normal | 4% | Pickup / Gluttony |
| Pincurchin | Electric | 2% | Lightning Rod / Electric Surge (HA) |

#### Water/Fishing

- **Surf:** Wingull / Pelipper (unchanged from vanilla).
- **Fishing (all rods):** Finneon 100%. Lumineon is the coastal swimmer here; Aevian Magikarp remains Serene Pond exclusive.
- **Corphish/Crawdaunt intentionally absent** from Route 104 (will be placed elsewhere or held for later).

#### Balance Notes

**Density note**: 8-9 mons per day/night pool is above the 5-6 target for single-biome routes. Justified by the dual grass/beach biome character of Route 104. Filler slots (Poochyena/Zigzagoon at 4%) could be pruned if the pool feels crowded in playtesting.

**Krabby**: Flagship beach mon. Kingler has strong Atk and Sheer Force HA boosts Crabhammer/X-Scissor. Early Water access — a key Gym 1 (Roxanne) tool for **Deino players**, who are neutral into Rock and need team-based answers.

**Dwebble**: Bug/Rock — defensively mediocre (weak to Rock/Water/Steel) but **Crustle + Shell Smash** is a legit sweeper. Shell Armor HA blocks crits. Good long-term project.

**Marill**: Huge Power HA doubles Attack. Azumarill + Aqua Jet is a classic priority bruiser. Plays into the Azurill pipeline from Route 102. Water/Fairy gives team coverage against Dark/Dragon threats long-term (including mirror-match starter fights).

**Wingull**: Carries from Route 103. Pelipper Drizzle is core to the weather pillar but evolves mid-game (~L25).

**Taillow**: Vanilla pure Flying. Guts + Flame/Toxic Orb is a classic combo for Swellow. Fast glass cannon.

**Pincurchin**: Rare (2%) pure Electric. **Electric Surge HA** sets Electric Terrain — unique early terrain mechanic access. Pre-Wattson Electric coverage. Commitment catch via DexNav or patience.

**Spinarak / Wurmple / Poochyena / Zigzagoon**: Filler overlap with earlier routes. Keeps Route 104 feeling connected to the rest of early Hoenn.

**No Corphish**: Intentional. Crawdaunt is strong but placing it here alongside Krabby would be two beach-crustacean lines — thematic overlap. Reserved for later placement or intentional cut.

## Gameplay Arcs

### Rival Battle 1 - Route 103.

Only starters available. Rival takes the other starter (mirror match).

| Player | Rival | Notes |
|--------|-------|-------|
| Axew | Deino | Pure Dragon-on-Dragon. Axew's physical bulk + Steel typing advantage vs most early non-STAB moves; Deino's Dragon Rage floors 40 damage. Test: does Axew 2-shot, or does Deino's Dragon Rage + Bite bring it close? |
| Deino | Axew | Same battle inverted. Deino's special bulk is worse but Dragon Rage doesn't scale off Atk. Test: is the flat damage enough to threaten Axew? |

No type advantage either way — both are Dragon. The fight is about stats, abilities (Rivalry vs Hustle), and move access at L5-7.

### Next??


