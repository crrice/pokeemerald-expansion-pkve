# Gameplay Roadmap: Act 1 (Gyms 1 & 2)

This document outlines the intended player progression, availability of key resources, and design rationale for the first act of the game, culminating in the defeat of the second Gym Leader, Brawly.

**Target level range:** Starter L5 → Gym 1 (Roxanne) L12-15 → Gym 2 (Brawly) L18-24. The expanded level curve (see `memory/level_curve.md`) pulls Act 1 longer than vanilla — more time on Routes 101-104 and in Petalburg Woods is expected.

## Core Design Principles

This roadmap is guided by several core design philosophies:

1.  **Thematic Progression (Weather):** The player should be introduced to the hack's core mechanic—weather—in a structured way. The progression should follow a "Problem -> Tools -> Mastery" arc, where they first experience weather as an obstacle, then acquire the tools to counter it, and finally gain the ability to control it themselves.
2.  **Rival 'Fair Play' Doctrine:** The rival serves as a mirror to the player's journey. They should only use Pokémon that are rare but technically obtainable by the player in the same areas. This reinforces the narrative of two trainers on a parallel adventure. The rival getting "lucky" with a 1% encounter is a perfect way to introduce new mechanics.
3.  **Starter 'Late Growers' Philosophy:** Both starters (Axew and Deino) are pseudo-legendary lines that evolve slowly — mid-evo around Gym 4-5, final form around Gym 6-7. This is deliberate. The early game is *not* carried by a snowballing starter; it's carried by a TEAM. Routes 101-103 are stocked with meaningful wild catches, and Act 1's design rewards players who build out instead of solo-ing. The starter should feel *present and useful* — never deadweight — but the player's relationship with their captured Pokemon should matter just as much.

---

## Segment 1: The Foundation
*(Littleroot Town → Rival Battle 1)*

**Player Journey:**
*   Choose a starter: **Axew** (Dragon/Steel) or **Deino** (Dark/Dragon).
*   Receive the Pokédex and travel to Route 103 for the first rival battle.

| Availability | Details |
| :--- | :--- |
| **Pokémon** | Starter only. |
| **Items** | None. |
| **Mechanics**| Basic battle mechanics. |

**Design Rationale:**
This segment isolates the player with their starter to build a foundational bond and plant the seed of a long partnership — these pseudo-legendary lines won't fully evolve until Gym 6-7, so the starter is a *project*, not a weapon. The rival takes the other starter (player Axew → rival Deino, and vice versa), setting up a mirror-match rivalry that persists throughout the game.

The rival battle is a simple, pure test of this initial experience. Both starters are roughly evenly matched at L5-7 — Axew's better physical bulk vs Deino's Dragon Rage floor damage. No type advantage in either direction since both are Dragon-typed.

---

## Segment 2: Building a Team
*(Post-Rival 1 → Wally's Tutorial)*

**Player Journey:**
*   Receive Poké Balls.
*   Travel through Routes 102 & 101 to Petalburg City.

| Availability | Details |
| :--- | :--- |
| **Pokémon** | Encounters on Routes 101-103 become catchable (Wooper, Wingull, Spinarak, etc.). |
| **Items** | Poké Balls, Potions. |
| **Mechanics**| Day/Night system becomes strategically relevant for encounters. |

**Design Rationale:**
Under the Late Growers philosophy, this segment is where the player's *real* team starts forming. Unlike hacks where the starter snowballs, here the starter is mid-game potential that needs to be *supported* now.
*   **Axew players** have a huge advantage into the first gym (Steel STAB + Rock resistance) but are 2x-weak to Fighting. They need Psychic/Flying/Fairy coverage for Gym 2 → catching Ralts, Wingull, or later an Abra is meaningful.
*   **Deino players** are neutral into Gym 1 (Rock hits Dark/Dragon 1x) and struggle against Rock-types without help. Wooper (Water/Ground) at night is a standout pick — also neutral-ish into Gym 2.
*   **Both** players should learn the Day/Night system in this segment; encounter variance per time makes Routes 101-103 feel bigger than vanilla.

---

## Segment 3: The Divergence (Tunnel Before Gym)
*(Post-Wally → Rusturf Tunnel → Roxanne or Wattson)*

**Player Journey (Hero intent):**
*   Complete Wally's tutorial in Petalburg.
*   Traverse Petalburg Woods. Two Aqua grunts ambush a Devon researcher; one fights the player as a delay, the other flees with the stolen goods.
*   Arrive in Rustboro. Gym is closed — *"Leader Roxanne is assisting the Devon Corporation."* NPCs and Devon Corp staff point east, toward Rusturf Tunnel.
*   Enter Rusturf Tunnel. Cutscene with Roxanne at a freshly-smashed east wall — the grunt is visibly fleeing through the breach. Roxanne: *"Damn — they broke through?! If the goods are already on the other side, we may have lost them..."* She heads back to Rustboro.
*   **The choice** (presented geographically, not by dialogue):
    *   **Double back** through the tunnel. A previously-unnoticed side passage reveals the grunt — the broken rocks were a diversion. Fight him, recover the goods.
    *   **Pursue east** through the broken rocks. L15-18 wild gauntlet. Emerge in Verdanturf.
*   **Hero path:** return goods to Devon → receive **Cut HM** → Roxanne returns to her gym → challenge her as Gym 1.

| Availability | Details |
| :--- | :--- |
| **Pokémon** | Petalburg Woods encounters (Shroomish, Cottonee, etc.). Rusturf Tunnel interior on both paths. |
| **Items** | Exp. Share (Devon employee, pre-tunnel). Cut HM (post-goods-return). |
| **Mechanics**| First divergence point. Gym-before-tunnel sequence broken. Cut HM available earlier than vanilla (pre-Gym-1 on hero). |

**Design Rationale:**
This segment restructures the traditional Emerald opening to front-load the Hero/Anti-hero choice. The misdirection is both literal (the grunt broke the rocks to bait pursuit) and meta (seasoned Emerald players expect gym-first and will feel the wrongness). The player who rushes east "relentlessly pursues" — anti-hero's defining trait. The player who stops and thinks retraces — hero's trait. The choice is embodied in movement, never prompted.

*   **Gym closure mechanism:** Rustboro gym is gated by tunnel-scene completion. Once Roxanne has delivered her line at the broken rocks, the flag flips; on the hero path she returns to her gym after goods are confirmed returned to Devon.
*   **Anti-hero diversion:** The player who pursues east emerges in Verdanturf. Wattson is their first challenge — same team as hero's Gym 3, no scaling. The true commitment (the "Rubicon") is not tunnel emergence, but **beating Wattson**. Until that badge is earned, player can retreat through the tunnel and pick up the hero path via the side passage.
*   **Rival dropped from this segment.** The tunnel scene carries the weight; a rival battle here would dilute the Rubicon moment. The rival's weather identity is introduced later (Segment 4 rework or beyond).
*   **Starters into Roxanne (if reached on hero path):**
    *   **Axew:** Dragon/Steel resists Rock and hits Rock 2x SE with Metal Claw (learnt ~L12-15). Dominant matchup — Axew players steamroll.
    *   **Deino:** Dark/Dragon is neutral into Rock. Bite/Crunch are 1x, Dragon Rage floors 40. Deino leans on team (Wooper, Lotad, Ralts) and wins through composition.
*   Asymmetry remains intentional: Axew players get the "my starter is a monster" moment; Deino players get the "I built a team" moment.

---

## Segment 4: Acquiring Tools & Counters
*(Post-Roxanne → Brawly)*

**Player Journey:**
*   Earn the Stone Badge and receive the Cut HM.
*   Travel to Dewford Town and explore Granite Cave.

| Availability | Details |
| :--- | :--- |
| **Pokémon** | **Oldale Ruins (Cut Area):** Rare **Psyduck** with **Cloud Nine** ability. <br> **Serene Pond (Cut Area):** **Lotad** (Rain Dish), **Poliwag** (Swift Swim). <br> **Granite Cave:** **Abra**, **Makuhita**. |
| **Items** | **Utility Umbrella** (Gift from Prof. Birch after beating the weather-using rival). |
| **Mechanics**| Rewarding exploration with direct counters to the game's core challenge. |

**Design Rationale:**
This segment is about empowering the player. After being shown the "problem" of weather, they are now given the "tools" to solve it.
*   **Rewarding Exploration:** The first direct weather counter (**Cloud Nine Psyduck**) and a key strategic item (**Utility Umbrella**) are deliberately placed as rewards for story progression and exploration (using Cut). This teaches the player to engage with the world to solve their problems.
*   **Foreshadowing:** The player can now find their own weather-synergy Pokémon (Lotad, Poliwag), foreshadowing that they will one day be able to control weather themselves.
*   **The Universal Skill Check:** Brawly's Fighting-type Gym is where the Late Growers philosophy pays off. **Both starters are 2x-weak to Fighting** — unlike a trio hack where one starter dominates, here Brawly tests *team-building*, not starter-choice.
    *   **Axew players**, who steamrolled Roxanne, now have to think. Dual Chop (~L18-20) is neutral damage. A caught Wingull (Air Slash SE) or Ralts (Psybeam SE vs Meditite) earns its keep. This is where "easy mode" meets its ceiling and the team matters.
    *   **Deino players**, who had to build a team for Roxanne, find that same team carries here too. **Crunch at L20** is 2x SE on Meditite (Psychic), a genuine answer. The hardship of Gym 1 pays forward — Deino players are ahead of the curve in team-building.
    *   **Makuhita and pure Fighting mons** wall both starters. Team coverage (Abra, Ralts, Wingull) isn't optional — it's the point.
