
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

Notes on starter balance. Integrate from TODO.md / discussion.

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

**Wooper**: Night-exclusive. Water/Ground is excellent defensive typing. Only Ground-type available pre-Petalburg - critical for Pikipek players vs Roxanne. Unaware HA ignores stat boosts, strong vs setup sweepers. Quagsire is a solid wall.

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

## Gameplay Arcs

### Rival Battle 1 - Route 103.

Only starters available.

Player -- Rival

Roggen vs Machop: No problems.
Machop vs Pikipek: TODO, test.
Pikipek vs Roggen: TODO, test.

### Next??


