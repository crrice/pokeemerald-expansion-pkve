// Custom time-of-day event handlers
// This file handles game events that need to respond to time changes

#include "global.h"
#include "main.h"
#include "event_data.h"
#include "berry.h"
#include "constants/berry.h"
#include "constants/quests.h"
#include "constants/maps.h"
#include "constants/items.h"
#include "constants/rtc.h"
#include "overworld.h"
#include "quests.h"

// Forward declarations
static void UpdateBerryThiefTree(void);

// Helper to check quest state
static bool8 IsQuestComplete(u8 questId)
{
    return QuestMenu_GetSetQuestState(questId, FLAG_GET_COMPLETED);
}

static bool8 IsQuestReward(u8 questId)
{
    return QuestMenu_GetSetQuestState(questId, FLAG_GET_REWARD);
}

static bool8 IsQuestActive(u8 questId)
{
    return QuestMenu_GetSetQuestState(questId, FLAG_GET_ACTIVE);
}

// Called from UpdateTimeOfDay when the time period changes
void OnTimeOfDayChange(u8 previousTime, u8 newTime)
{
    // Only process if we're on a relevant map (Oldale Ruins area uses MAPSEC_ROUTE_102)
    if (gMapHeader.regionMapSectionId == MAPSEC_ROUTE_102)
    {
        UpdateBerryThiefTree();
    }
}

// Update the Berry Thief quest tree based on quest state and time
static void UpdateBerryThiefTree(void)
{
    struct BerryTree *tree = GetBerryTreeInfo(BERRY_TREE_OLDALE_RUINS_1);

    // Quest complete - normal behavior, don't interfere
    if (IsQuestComplete(QUEST_BERRY_THIEF))
        return;

    // Quest in reward state - tree should have berries ready
    if (IsQuestReward(QUEST_BERRY_THIEF))
    {
        // Only set if not already in berry state (avoid resetting if player is mid-pickup)
        if (tree->stage != BERRY_STAGE_BERRIES)
        {
            PlantBerryTree(BERRY_TREE_OLDALE_RUINS_1, ITEM_TO_BERRY(ITEM_WATMEL_BERRY), BERRY_STAGE_BERRIES, FALSE);
        }
        return;
    }

    // Quest active - check time of day
    if (IsQuestActive(QUEST_BERRY_THIEF))
    {
        if (gTimeOfDay == TIME_NIGHT)
        {
            // Night: tree appears full (thief is there)
            PlantBerryTree(BERRY_TREE_OLDALE_RUINS_1, ITEM_TO_BERRY(ITEM_WATMEL_BERRY), BERRY_STAGE_BERRIES, FALSE);
        }
        else
        {
            // Day: tree stuck at flowering (berries not ready)
            PlantBerryTree(BERRY_TREE_OLDALE_RUINS_1, ITEM_TO_BERRY(ITEM_WATMEL_BERRY), BERRY_STAGE_FLOWERING, FALSE);
        }
        return;
    }

    // Quest locked (not started) - tree always stuck at flowering
    PlantBerryTree(BERRY_TREE_OLDALE_RUINS_1, ITEM_TO_BERRY(ITEM_WATMEL_BERRY), BERRY_STAGE_FLOWERING, FALSE);
}

// Called on map load to set initial tree state
void InitBerryThiefTreeState(void)
{
    UpdateBerryThiefTree();
}
