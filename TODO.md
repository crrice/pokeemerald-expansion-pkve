
# TODO

---

- Find a better name.

---

I have settled on using the flying/fighting/rock trio as the starter types.
Therefore a lot is needed to really enable that.

Because for a starter trio they should have:
Symmetric type matchups in all directions. Only the traditional trio and the above satisfy this.
Three-stage pokemon (evolves twice).
No evolution gimmicks (level up evo).
Similar evo levels.
Have a mega-evolution (well, either all have it or none).

For fighting type, we can only choose from the Mankey, Machop, and Timbur lines, as only these are three-stage.
However both the Timbur and Machop lines use a trade for the final evo.
And then Primeape evolving however the fuck it wants (which ofc is use rage fist 20x).

For rock, the variety of stage 2 is quite large:
Geodude, classic, but has evo gimmick.
Rhyhorn, awesome but Rhyperior has evo gimmick and also is just meh (was better as a stage 2 only imo, hot take).
Roggenrola, which remains as pure rock.
Rolycoly, which picks up fire type when it evolves.
Larvitar, which gets dark on the final evo but has ground subtype till then. But also everyone loves Tyranitar.
Aron, which I personally love but I want non gen-3 pkmn for starters.

And for flying the motherlode of options:
Pidgey, has a mega-evo and is classic AF.
Zubat, which I love and everyone I know loves and would be super cool.
Fletchling, great pokemon and it gets fire type!
Pikipek, toucannon is epic and will get pure flying...

So given all that, the todos are:

- Change the starter trio! Lets use...: (Pikipek) > (Machop) > (Roggenrola)
**DONE**

This gives a final trio of (TalonFlame (Flying)) > (Machamp (Fighting)) > (Roggenrola (Rock))
Is good because I like the pure types. Other notes:

So with this combo, the other tasks are:

- Change Machop to evolve at level 18, and again at 36 (instead of trade).
**DONE**
- Change Roggenrola to evolve at level 16, and again at 34 (instead of trade).
**DONE**

Pikipek already evolves at levels 14 and 28 which I like.

Since most flying types also are normal type, it kinda ruins the advantage
gimmick of the starters. And cause I think it would be cool anyway:
- Change normal/flying pkmn types to be pure flying where appropriate (which is most of them lmao rofl),

Also I understand that roost removes the flying type from a pkmn for that turn which I think
is just kind of odd so I think this change fits thematically while still keeping a type on that turn:
- Change effect of 'roost' so when used it applies effect of 'gravity' for 1 turn instead of removing flying type.
NOTES: L12054, remove this code to disable the type removal effect of roost. But then how to apply the effect of smack down instead?
Basically search for `roostActive` and you will find all the things for that but there is still the question of how apply other effect.
Something something battleStruct thingy IDKTHBWTF.

Anyway I guess it should handle type-less pkmn just fine but like isn't it kind of a buff for roost?
Like yeah I heal and nothing is super effective this turn. Nah. That move don't need no buffs.
And I just like the idea of like, if a bird is roosting it's grounded in a way so ground moves should hit it.

And as some final touches:
- Give Roggenrola line some buffs.
- Give Pikipek line some buffs.
- Give Machop line some buffs.

They should feel like starters... Generally speaking: early powerhouses that remain potent yet taper off toward the endgame.
Take into account their evo levels and midgame balance but make them overall feel good to use throughout a run. Yet no so good
that you feel like you can't box them. Take a look at their abilities and stats and make it have a decent strat for the whole
run, not something that kicks in (too) late.

Ability notes:
Machop (Guts, No Guard) default abilities (look good).
       (Steadfast) hidden (looks not so good, but what do I know).
Roggenrola (Sturdy, No Guard > Sand Stream) default abilities (look good).
           (Sand Force) hidden (looks iffy, maybe weak, but what do I know).
Pikipek (Keen Eye, Skill Link) default abilities (Keen Eye maybe needs replaced, Skill Link OP for sure).
        (Pickup) hidden (fuck no).

And their base stats are gunna need changed too.
Yay! Many things for much later (aka probably never).

Okay, so noting that since the starter trio changed we have to change the starter the rival uses, so:
- Redo rival teams (brendan/may)

I think it would be fun if the two rivals had differing battle styles.
Brendan should be a hyper-aggro glass cannon sweeper edgelord.
And May should favor bulkier mons.
- Separate rivals into two battle styles.

Alright so I changed the evolution level and method for the new starters but it occurs to me that their
balance will be pretty fucked so:
- Review starter line configs.
  - Learnset.
  - BST.
  - Abilities.

Because this is Emerald and I also believe it would be fun AF, lets try:
- Change weather to its OG. The weather tyranny returns, it has no automatic turn limit.

Might have to change it back later but lets give it a go.

## Littleroot Town

Here we will use this town as a training ground, I guess. So, lets make these changes:

- Add a tree.
- Add a sign.
- Change the assistants dialog when he says the prof isn't there at the start.

And more advanced:

- Make a small new area.
  - Link it to littleroot town behind a cut tree.
  - Add a small grass patch with an encounter.
  - Add a small pond with a fishing encounter (but no surf, is peaceful here).

And finally:

- Make that fishing encounter Aevian Magikarp.

And for that matter lets add the Aevian variants that are seabound, so Magikarp and:

- Add Aevian Feebas
- Add Aevian Shellos (Both forms? They do a live/dying reef theme)

They're both kind of reef themed maybe something can be done with that. Throw in that ghost type corsola for fun too.
We need a polluted reef environment or something cool like that.
Right outside slateport or mauville maybe.

## Route 101

- Add a trainer with a top percentage zigzagoon (and actually give it cracked IVs).
