---
name: time-of-day
description: Time-of-day system for day/night encounters, time-based events, and RTC configuration. Use when implementing time-sensitive features or debugging time-related behavior.
---

# Time of Day System

Guide for using the day/night cycle, time-based encounters, and time-triggered events.

## Key Files

| File | Purpose |
|------|---------|
| `include/config/overworld.h` | Time system configuration |
| `include/constants/rtc.h` | Time period constants (TIME_MORNING, etc.) |
| `src/overworld.c` | Time update logic, `gTimeOfDay` variable |
| `src/data/wild_encounters.json` | Time-based encounter tables |
| `src/time_events_custom.c` | Custom time-change callbacks (project-specific) |

## Time Periods

Four time periods defined in `include/constants/rtc.h`:

| Constant | Value | Hours |
|----------|-------|-------|
| `TIME_MORNING` | 0 | 6:00 - 11:59 |
| `TIME_DAY` | 1 | 12:00 - 16:59 |
| `TIME_EVENING` | 2 | 17:00 - 19:59 |
| `TIME_NIGHT` | 3 | 20:00 - 5:59 |

## Configuration

In `include/config/overworld.h`:

```c
// Enable time-based wild encounters
#define OW_TIME_OF_DAY_ENCOUNTERS    TRUE

// Use fake RTC (not synced to real time)
#define OW_USE_FAKE_RTC              TRUE

// Time speed multiplier (GEN_8_PLA = 60x, GEN_9 = 20x)
#define OW_ALTERED_TIME_RATIO        GEN_8_PLA
```

## Checking Time in Scripts

Use `gettimeofday` to get current time period in `VAR_0x8000`:

```asm
MyScript::
    gettimeofday
    goto_if_eq VAR_0x8000, TIME_NIGHT, MyScript_Night
    goto_if_eq VAR_0x8000, TIME_MORNING, MyScript_Morning
    @ Default (day/evening)
    msgbox Text_Daytime, MSGBOX_DEFAULT
    end

MyScript_Night:
    msgbox Text_Nighttime, MSGBOX_DEFAULT
    end

MyScript_Morning:
    msgbox Text_Morning, MSGBOX_DEFAULT
    end
```

**Important:** Use `gettimeofday` (puts TIME_* constant 0-3 in `VAR_0x8000`), NOT `gettime` (which puts hour/min/sec in VAR_0x8000/8001/8002).

## Checking Time in C Code

The global variable `gTimeOfDay` holds the current time period:

```c
#include "overworld.h"
#include "constants/rtc.h"

void MyFunction(void)
{
    if (gTimeOfDay == TIME_NIGHT)
    {
        // Night-specific behavior
    }
}
```

## Time-Based Encounters

### Enabling

Set `OW_TIME_OF_DAY_ENCOUNTERS` to `TRUE` in config.

### Encounter Table Format

In `src/data/wild_encounters.json`, create separate entries for each time period using suffixes:

```json
{
  "map": "MAP_ROUTE101",
  "base_label": "gRoute101_Morning",
  "land_mons": {
    "encounter_rate": 20,
    "mons": [...]
  }
},
{
  "map": "MAP_ROUTE101",
  "base_label": "gRoute101_Day",
  "land_mons": { ... }
},
{
  "map": "MAP_ROUTE101",
  "base_label": "gRoute101_Evening",
  "land_mons": { ... }
},
{
  "map": "MAP_ROUTE101",
  "base_label": "gRoute101_Night",
  "land_mons": { ... }
}
```

The suffixes `_Morning`, `_Day`, `_Evening`, `_Night` are required for the system to find the correct table.

### Sharing Tables

If Morning/Day/Evening share the same encounters, you can omit separate entries - the system falls back. Only define the variants that differ.

### Encounter Slot Rates

The 12 encounter slots have fixed rates that cannot be changed per-route:

```
Slots 0-1:  20% each (40% total)
Slots 2-3:  10% each (20% total)
Slots 4-5:  10% each (20% total)
Slots 6-7:   5% each (10% total)
Slots 8-9:   4% each (8% total)
Slots 10-11: 1% each (2% total)
```

To achieve a desired distribution like 40/30/20/10, place species in multiple slots.

## Time Change Callbacks

To run code when the time period changes, use the callback system in `src/time_events_custom.c`:

```c
#include "global.h"
#include "overworld.h"
#include "constants/rtc.h"
#include "constants/maps.h"

void OnTimeOfDayChange(u8 previousTime, u8 newTime)
{
    // Only process on relevant maps
    if (gMapHeader.regionMapSectionId == MAPSEC_ROUTE_102)
    {
        // Update something based on time change
        UpdateMyFeature();
    }
}
```

This is called from `UpdateTimeOfDay()` in `src/overworld.c` whenever `gTimeOfDay` changes.

### Example: Time-Based Object State

From the Berry Thief quest - tree changes appearance based on time:

```c
static void UpdateBerryThiefTree(void)
{
    if (IsQuestComplete(QUEST_BERRY_THIEF))
        return;  // Normal behavior after quest

    if (IsQuestActive(QUEST_BERRY_THIEF))
    {
        if (gTimeOfDay == TIME_NIGHT)
        {
            // Night: tree appears full (thief is there)
            PlantBerryTree(TREE_ID, BERRY_ID, BERRY_STAGE_BERRIES, FALSE);
        }
        else
        {
            // Day: tree stuck at flowering
            PlantBerryTree(TREE_ID, BERRY_ID, BERRY_STAGE_FLOWERING, FALSE);
        }
    }
}

void OnTimeOfDayChange(u8 previousTime, u8 newTime)
{
    if (gMapHeader.regionMapSectionId == MAPSEC_ROUTE_102)
        UpdateBerryThiefTree();
}
```

## Map Load Initialization

To set initial state when entering a map, call from the map's `OnLoad` script:

```asm
MyMap_MapScripts::
    map_script MAP_SCRIPT_ON_LOAD, MyMap_OnLoad
    .byte 0

MyMap_OnLoad:
    callnative InitMyTimeBasedFeature
    end
```

```c
// In C code
void InitMyTimeBasedFeature(void)
{
    // Set up state based on current gTimeOfDay
}
```

## Testing Tips

### Fast Time for Testing

Set fake RTC with high speed multiplier:

```c
#define OW_USE_FAKE_RTC         TRUE
#define OW_ALTERED_TIME_RATIO   GEN_8_PLA  // 60x speed
```

At 60x, one real minute = one in-game hour.

### Debug Menu

Use the debug menu (Hold R + Press START) to check/set flags and variables for testing time-dependent quest states.

### Time Period Transitions

With fake RTC at 60x:
- Morning→Day: ~6 real minutes after game start
- Full day cycle: ~24 real minutes

## Common Issues

### Script checks time incorrectly
- Use `gettimeofday` not `gettime`
- Check `VAR_0x8000` not `VAR_0x8002`

### Encounters not changing with time
- Verify `OW_TIME_OF_DAY_ENCOUNTERS` is `TRUE`
- Check table labels have correct suffixes (`_Morning`, `_Day`, etc.)
- Ensure `base_label` matches the pattern

### Time-based C code not triggering
- Verify `OnTimeOfDayChange` is being called (add debug output)
- Check map section ID matches where you expect it to run
- Make sure the callback is declared in header and called from `UpdateTimeOfDay()`
