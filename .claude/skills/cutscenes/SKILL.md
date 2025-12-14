---
name: cutscenes
description: Scripted cutscene system for NPC/Pokemon movement, emotes, fades, and choreographed sequences. Use when creating story moments, special encounters, or any scripted event sequences.
---

# Cutscenes

Guide for creating scripted cutscenes with object movement, emotes, screen effects, and choreographed sequences.

## Key Files

| File | Purpose |
|------|---------|
| `asm/macros/event.inc` | Script commands (fadescreen, applymovement, etc.) |
| `asm/macros/movement.inc` | Movement action macros |
| `include/constants/event_objects.h` | Object/sprite constants |
| `include/constants/field_weather.h` | Fade mode constants |
| `data/maps/*/scripts.inc` | Map-specific cutscene scripts |
| `data/maps/*/map.json` | Object event definitions |

## Basic Structure

A cutscene typically follows this pattern:

```asm
MyMap_EventScript_Cutscene::
    lockall
    @ ... cutscene actions ...
    releaseall
    end
```

## Screen Fades

Fade the screen to/from black or white:

```asm
fadescreen FADE_TO_BLACK      @ Fade out to black
fadescreen FADE_FROM_BLACK    @ Fade in from black
fadescreen FADE_TO_WHITE      @ Fade out to white
fadescreen FADE_FROM_WHITE    @ Fade in from white
```

Use fades to hide repositioning of objects/player:

```asm
fadescreen FADE_TO_BLACK
setobjectxy OBJ_EVENT_ID_PLAYER, 10, 15
delay 60                       @ Hold black for 1 second (60 frames)
fadescreen FADE_FROM_BLACK
```

## Object Events

### Defining Objects in Porymap/map.json

Objects need to be defined in the map's `map.json`. For Pokemon sprites:

```json
{
  "local_id": "LOCALID_MY_POKEMON",
  "graphics_id": "OBJ_EVENT_GFX_SPECIES(PHANTUMP)",
  "x": 8,
  "y": 7,
  "elevation": 3,
  "movement_type": "MOVEMENT_TYPE_NONE",
  "script": "NULL",
  "flag": "FLAG_TEMP_2"
}
```

**Pokemon sprite format:** `OBJ_EVENT_GFX_SPECIES(SPECIES_NAME)` or numeric value (SPECIES_ID + 16384).

### Showing/Hiding Objects

```asm
@ Hide object on map load (set flag that hides it)
setflag FLAG_TEMP_2

@ Show object during cutscene
clearflag FLAG_TEMP_2
addobject LOCALID_MY_OBJECT

@ Hide object after cutscene
removeobject LOCALID_MY_OBJECT
setflag FLAG_TEMP_2
```

### Positioning Objects

```asm
setobjectxy LOCALID_NPC, 10, 15        @ Move object to coordinates
setobjectxy OBJ_EVENT_ID_PLAYER, 8, 12 @ Move player
```

## Movement

### Applying Movement

```asm
applymovement LOCALID_NPC, MyMap_Movement_WalkToPlayer
waitmovement 0    @ Wait for all movements to complete
```

### Movement Data Format

Movement sequences end with `step_end`:

```asm
MyMap_Movement_Example:
    walk_down
    walk_down
    walk_right
    face_up
    step_end
```

### Movement Actions

**Walking (normal speed):**
- `walk_down`, `walk_up`, `walk_left`, `walk_right`

**Walking (slow):**
- `walk_slow_down`, `walk_slow_up`, `walk_slow_left`, `walk_slow_right`

**Walking (fast):**
- `walk_fast_down`, `walk_fast_up`, `walk_fast_left`, `walk_fast_right`

**Facing (no movement):**
- `face_down`, `face_up`, `face_left`, `face_right`

**Jumping:**
- `jump_down`, `jump_up`, `jump_left`, `jump_right` - Jump 1 tile
- `jump_2_down`, `jump_2_up`, `jump_2_left`, `jump_2_right` - Jump 2 tiles
- `jump_in_place_down`, `jump_in_place_up`, etc. - Jump without moving

**In-place animation:**
- `walk_in_place_down`, `walk_in_place_up`, etc. - Walking animation without moving

**Delays:**
- `delay_1`, `delay_2`, `delay_4`, `delay_8`, `delay_16` - Pause for N frames

## Emotes

Show emotion bubbles above objects:

```asm
MyMap_Movement_Surprised:
    emote_exclamation_mark    @ !
    delay_16
    delay_16
    step_end

MyMap_Movement_Confused:
    emote_question_mark       @ ?
    delay_16
    delay_16
    step_end
```

**Available emotes:**
- `emote_exclamation_mark` - !
- `emote_question_mark` - ?
- `emote_heart` - Heart
- `emote_double_exclamation_mark` - !!
- `emote_x` - X

## Player Control

### Hiding Follower Pokemon

```asm
hidefollower 0    @ Hide follower instantly (0 = don't wait)
hidefollower 1    @ Hide follower and wait for animation
```

The follower automatically reappears after battles.

### Moving the Player

```asm
setobjectxy OBJ_EVENT_ID_PLAYER, 8, 12
applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceUp
waitmovement 0
```

## Timing and Delays

```asm
delay 30     @ Wait 30 frames (0.5 seconds at 60fps)
delay 60     @ Wait 1 second
```

In movement data, use `delay_16` (approx 0.27 seconds) and chain multiple for longer pauses.

## Complete Example

From the Berry Thief quest - Phantump emerges and gets startled:

```asm
@ Local ID reference
.set LOCALID_PHANTUMP, LOCALID_OLDALE_RUINS_PHANTUMP

OldaleRuins_EventScript_PhantumpCutscene:
    lockall
    msgbox Text_WaitByTree, MSGBOX_YESNO
    goto_if_eq VAR_RESULT, NO, OldaleRuins_EventScript_Cancel
    closemessage

    @ Setup: fade, reposition, spawn
    fadescreen FADE_TO_BLACK
    hidefollower 0
    setobjectxy OBJ_EVENT_ID_PLAYER, 8, 12
    applymovement OBJ_EVENT_ID_PLAYER, Common_Movement_FaceUp
    clearflag FLAG_TEMP_2
    addobject LOCALID_PHANTUMP
    delay 60
    fadescreen FADE_FROM_BLACK

    @ Phantump emerges
    delay 20
    applymovement LOCALID_PHANTUMP, Movement_PhantumpEmerge
    waitmovement 0

    @ Walks to tree, notices something
    applymovement LOCALID_PHANTUMP, Movement_PhantumpToTree
    waitmovement 0
    delay 30
    applymovement LOCALID_PHANTUMP, Movement_PhantumpNotice
    waitmovement 0

    @ Peeks around, spots player
    applymovement LOCALID_PHANTUMP, Movement_PhantumpPeek
    waitmovement 0
    delay 40    @ Comedic pause
    applymovement LOCALID_PHANTUMP, Movement_PhantumpStartled
    waitmovement 0

    @ Battle
    msgbox Text_StartledAttack, MSGBOX_DEFAULT
    closemessage
    createmon 1, 0, SPECIES_PHANTUMP, 5, abilityNum=2
    dowildbattle

    @ Cleanup
    removeobject LOCALID_PHANTUMP
    setflag FLAG_TEMP_2
    releaseall
    end

@ Movement data
Movement_PhantumpEmerge:
    jump_in_place_down
    delay_16
    jump_in_place_down
    delay_16
    step_end

Movement_PhantumpToTree:
    walk_slow_down
    walk_slow_down
    walk_slow_down
    step_end

Movement_PhantumpNotice:
    emote_question_mark
    delay_16
    delay_16
    delay_16
    step_end

Movement_PhantumpPeek:
    walk_slow_left
    face_down
    delay_16
    walk_slow_right
    walk_slow_right
    walk_slow_down
    walk_slow_down
    face_left
    step_end

Movement_PhantumpStartled:
    emote_exclamation_mark
    delay_16
    delay_16
    delay_16
    step_end
```

## Tips

- **Lock first:** Always `lockall` at start to prevent player input during cutscene.
- **Fade for repositioning:** Use screen fades to hide sudden position changes.
- **Comedic timing:** Add `delay` commands between emotes and reactions.
- **Slow movement:** Use `walk_slow_*` for more dramatic/deliberate movement.
- **Clean up:** Always `removeobject` and reset flags after temporary objects.
- **Test positions:** The player interacts from different angles - account for this or reposition them.

## Common Issues

### Object visible when it shouldn't be
- Ensure the flag hiding the object is set on map load
- Add `setflag FLAG_TEMP_X` to the map's `OnLoad` script

### Movement looks wrong
- Check coordinates - Y increases going down
- Verify elevation matches other objects on the map

### Player can move during cutscene
- Make sure `lockall` is called before any cutscene actions
- Use `releaseall` only at the very end
