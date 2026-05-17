---
name: vanilla-walkthrough
description: Look up events, trainers, items, or progression details from vanilla Pokemon Emerald. Use when answering any question about what happens in the unmodified game (e.g. "what happens after gym X", "when do you meet Y", "what's in route Z in vanilla"). Useful as a baseline reference when comparing to this hack's divergences.
---

# Vanilla Emerald Walkthrough Lookup

Reference: `docs/VANILLA_WALKTHROUGH.txt` — a full text walkthrough of unmodified Pokemon Emerald by Matt Johnson (GameFAQs, 2006).

## Navigation system (use this — don't read the whole file)

The walkthrough uses 5-character bracketed codes (e.g. `[pekoh]`, `[mfrbc]`, `[87/97]`) to jump between sections. The Table of Contents sits at the top of the file (roughly lines 12–170) and lists every section with its code.

**Workflow:**

1. **Read the TOC first** to find sections that match the question:
   ```
   Read docs/VANILLA_WALKTHROUGH.txt with limit ~170 lines
   ```
2. **Grep for the code(s)** to get line numbers — each code appears twice (once in the TOC, once at the section header):
   ```
   grep -n "pekoh\|mfrbc" docs/VANILLA_WALKTHROUGH.txt
   ```
3. **Read only the relevant section** using `offset` and `limit`. Sections are typically 30–200 lines.

This keeps context usage minimal — never read the whole file (it is thousands of lines).

## Section structure

The TOC is organized by badge / story milestone:

- **Section 3** — Intro through leaving Littleroot
- **Section 4** — Gym 1 (Roxanne)
- **Section 5** — Gym 2 (Brawly) — includes Rusturf Tunnel, Dewford
- **Section 6** — Gym 3 (Wattson) — Slateport, Mauville
- **Section 7** — Gym 4 (Flannery) — Fallarbor, Meteor Falls, Lavaridge
- **Section 8** — Gym 5 (Norman) — Mirage Tower, Petalburg
- **Section 9** — Gym 6 (Winona) — Weather Institute, Fortree
- **Section 10** — Surf routes
- **Section 11** — Gym 7 (Tate & Liza? check TOC)
- Later sections cover gym 8, Elite Four, postgame

Rival fights, item lists, and per-route Pokemon tables are embedded inline within each section.

## When to use

- Any question about vanilla story beats, NPCs, items, or trainer rosters.
- Comparing what the hack changes vs. the baseline (cross-reference with `DIVERGENCE_THEORY.md`, `ROADMAP.md`, `BALANCE.md`).
- Sanity-checking whether a feature being added/changed exists in vanilla.

## When NOT to use

- Questions about *this hack's* changes — those live in the project design docs (`DIVERGENCE_THEORY.md`, `BALANCE.md`, etc.) and the actual code/data files.
- Mechanics questions (damage formulas, ability behavior) — check the code or upstream pokeemerald-expansion docs.
