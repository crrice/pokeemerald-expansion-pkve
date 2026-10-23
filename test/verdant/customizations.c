#include "global.h"
#include "test/test.h"
#include "item.h"
#include "item_use.h"
#include "starter_choose.h"
#include "string_util.h"
#include "wild_encounter.h"
#include "constants/items.h"
#include "constants/map_groups.h"
#include "constants/maps.h"

// Pins our edits to upstream data, so a merge that silently drops one fails here
// instead of in a manual audit (merge hygiene item 15). The design behind the
// values lives in the design docs; this file only holds what the game ships.

#define NOT_LEARNED 0xFF

// Level of the first level-up entry for move, or NOT_LEARNED.
static u32 GetLevelUpMoveLevel(enum Species species, enum Move move)
{
    const struct LevelUpMove *learnset = GetSpeciesLevelUpLearnset(species);
    for (u32 i = 0; learnset[i].move != LEVEL_UP_MOVE_END; i++)
    {
        if (learnset[i].move == move)
            return learnset[i].level;
    }
    return NOT_LEARNED;
}

// Level of the EVO_LEVEL evolution into target, or 0 if there is none.
static u32 GetEvolutionLevel(enum Species species, enum Species target)
{
    const struct Evolution *evolutions = GetSpeciesEvolutions(species);
    if (evolutions == NULL)
        return 0;
    for (u32 i = 0; evolutions[i].method != EVOLUTIONS_END; i++)
    {
        if (evolutions[i].method == EVO_LEVEL && evolutions[i].targetSpecies == target)
            return evolutions[i].param;
    }
    return 0;
}

static bool32 FormTableHas(enum Species species, enum Species form)
{
    const u16 *table = GetSpeciesFormTable(species);
    if (table == NULL)
        return FALSE;
    for (u32 i = 0; table[i] != FORM_SPECIES_END; i++)
    {
        if (table[i] == form)
            return TRUE;
    }
    return FALSE;
}

static const struct WildPokemonHeader *FindWildMonHeader(u32 mapGroup, u32 mapNum)
{
    for (u32 i = 0; gWildMonHeaders[i].mapGroup != MAP_GROUP(MAP_UNDEFINED); i++)
    {
        if (gWildMonHeaders[i].mapGroup == mapGroup && gWildMonHeaders[i].mapNum == mapNum)
            return &gWildMonHeaders[i];
    }
    return NULL;
}

TEST("Verdant: the starters are Axew and Deino")
{
    EXPECT_EQ(GetStarterPokemon(0), SPECIES_AXEW);  // VAR_STARTER_MON 0 (TREECKO slot)
    EXPECT_EQ(GetStarterPokemon(1), SPECIES_DEINO); // VAR_STARTER_MON 1 (TORCHIC slot)
}

TEST("Verdant: level-up learnset edits")
{
    enum Species species = SPECIES_NONE;
    enum Move move = MOVE_NONE;
    u32 level = 0;

    PARAMETRIZE { species = SPECIES_AXEW;       move = MOVE_METAL_CLAW;     level = 6; }
    PARAMETRIZE { species = SPECIES_AXEW;       move = MOVE_FALSE_SWIPE;    level = 9; }
    PARAMETRIZE { species = SPECIES_AXEW;       move = MOVE_ASSURANCE;      level = 12; }
    PARAMETRIZE { species = SPECIES_FRAXURE;    move = MOVE_METAL_CLAW;     level = 1; }
    PARAMETRIZE { species = SPECIES_FRAXURE;    move = MOVE_ASSURANCE;      level = 12; }
    PARAMETRIZE { species = SPECIES_HAXORUS;    move = MOVE_METAL_CLAW;     level = 1; }
    PARAMETRIZE { species = SPECIES_HAXORUS;    move = MOVE_ASSURANCE;      level = 12; }
    PARAMETRIZE { species = SPECIES_DEINO;      move = MOVE_BITE;           level = 6; }
    PARAMETRIZE { species = SPECIES_DEINO;      move = MOVE_DRAGON_BREATH;  level = 16; }
    PARAMETRIZE { species = SPECIES_MACHOP;     move = MOVE_POUND;          level = 1; }
    PARAMETRIZE { species = SPECIES_MACHOP;     move = MOVE_LOW_KICK;       level = 6; }
    PARAMETRIZE { species = SPECIES_ROGGENROLA; move = MOVE_ROCK_THROW;     level = 7; }

    ASSUME(P_LVL_UP_LEARNSETS >= GEN_9); // our edits live in level_up_learnsets/gen_9.h
    EXPECT_EQ(GetLevelUpMoveLevel(species, move), level);
}

TEST("Verdant: the Axew line is Dragon/Steel")
{
    enum Species species = SPECIES_NONE;

    PARAMETRIZE { species = SPECIES_AXEW; }
    PARAMETRIZE { species = SPECIES_FRAXURE; }
    PARAMETRIZE { species = SPECIES_HAXORUS; }

    EXPECT_EQ(GetSpeciesType(species, 0), TYPE_DRAGON);
    EXPECT_EQ(GetSpeciesType(species, 1), TYPE_STEEL);
}

TEST("Verdant: evolution level edits")
{
    enum Species species = SPECIES_NONE, target = SPECIES_NONE;
    u32 level = 0;

    PARAMETRIZE { species = SPECIES_AXEW;       target = SPECIES_FRAXURE;   level = 46; }
    PARAMETRIZE { species = SPECIES_FRAXURE;    target = SPECIES_HAXORUS;   level = 68; }
    PARAMETRIZE { species = SPECIES_DEINO;      target = SPECIES_ZWEILOUS;  level = 48; }
    PARAMETRIZE { species = SPECIES_ZWEILOUS;   target = SPECIES_HYDREIGON; level = 72; }
    PARAMETRIZE { species = SPECIES_MACHOP;     target = SPECIES_MACHOKE;   level = 18; }
    PARAMETRIZE { species = SPECIES_MACHOKE;    target = SPECIES_MACHAMP;   level = 36; }
    PARAMETRIZE { species = SPECIES_ROGGENROLA; target = SPECIES_BOLDORE;   level = 16; }
    PARAMETRIZE { species = SPECIES_BOLDORE;    target = SPECIES_GIGALITH;  level = 34; }
    PARAMETRIZE { species = SPECIES_MAGIKARP_AEVIAN; target = SPECIES_GYARADOS_AEVIAN; level = 20; }

    EXPECT_EQ(GetEvolutionLevel(species, target), level);
}

TEST("Verdant: Aevian Magikarp and Gyarados are forms of their species")
{
    EXPECT(FormTableHas(SPECIES_MAGIKARP, SPECIES_MAGIKARP_AEVIAN));
    EXPECT(FormTableHas(SPECIES_MAGIKARP_AEVIAN, SPECIES_MAGIKARP));
    EXPECT(FormTableHas(SPECIES_GYARADOS, SPECIES_GYARADOS_AEVIAN));
    EXPECT(FormTableHas(SPECIES_GYARADOS_AEVIAN, SPECIES_GYARADOS));
    EXPECT_EQ(GetSpeciesType(SPECIES_MAGIKARP_AEVIAN, 0), TYPE_FIRE);
    EXPECT_EQ(GetSpeciesType(SPECIES_MAGIKARP_AEVIAN, 1), TYPE_FIRE);
    EXPECT_EQ(GetSpeciesType(SPECIES_GYARADOS_AEVIAN, 0), TYPE_FIRE);
    EXPECT_EQ(GetSpeciesType(SPECIES_GYARADOS_AEVIAN, 1), TYPE_DRAGON);
}

TEST("Verdant: QoL Stat Editor item data")
{
    EXPECT_EQ(StringCompare(GetItemName(ITEM_QOL_STAT_EDITOR), COMPOUND_STRING("QoL Stat Editor")), 0);
    EXPECT_EQ(GetItemPocket(ITEM_QOL_STAT_EDITOR), POCKET_KEY_ITEMS);
    EXPECT_EQ(GetItemImportance(ITEM_QOL_STAT_EDITOR), 1);
    EXPECT_EQ(GetItemType(ITEM_QOL_STAT_EDITOR), ITEM_USE_PARTY_MENU);
    EXPECT(GetItemFieldFunc(ITEM_QOL_STAT_EDITOR) == ItemUseOutOfBattle_StatEditor);
}

TEST("Verdant: Exp. Share All item data")
{
    EXPECT_EQ(GetItemPocket(ITEM_EXP_SHARE_ALL), POCKET_KEY_ITEMS);
    EXPECT_EQ(GetItemType(ITEM_EXP_SHARE_ALL), ITEM_USE_FIELD);
    EXPECT(GetItemFieldFunc(ITEM_EXP_SHARE_ALL) == ItemUseOutOfBattle_ExpShareAll);
}

TEST("Verdant: Infinite Candy item data")
{
    EXPECT_EQ(GetItemPocket(ITEM_INFINITE_CANDY), POCKET_ITEMS);
    EXPECT_EQ(GetItemType(ITEM_INFINITE_CANDY), ITEM_USE_PARTY_MENU);
    EXPECT(GetItemFieldFunc(ITEM_INFINITE_CANDY) == ItemUseOutOfBattle_RareCandy);
    EXPECT(GetItemEffect(ITEM_INFINITE_CANDY) == GetItemEffect(ITEM_RARE_CANDY));
    EXPECT_EQ(GetItemConsumability(ITEM_INFINITE_CANDY), FALSE);
}

// Wimpy Brace: test/verdant/wimpy_brace.c.

TEST("Verdant: time-of-day encounter tables")
{
    u32 map = MAP_UNDEFINED;
    const struct WildPokemonHeader *header;

    PARAMETRIZE { map = MAP_ROUTE101; }
    PARAMETRIZE { map = MAP_ROUTE102; }
    PARAMETRIZE { map = MAP_ROUTE103; }
    PARAMETRIZE { map = MAP_SERENE_POND; }
    PARAMETRIZE { map = MAP_OLDALE_RUINS; }

    ASSUME(OW_TIME_OF_DAY_ENCOUNTERS == TRUE);
    header = FindWildMonHeader(MAP_GROUP(map), MAP_NUM(map));
    EXPECT(header != NULL);
    EXPECT(header->encounterTypes[TIME_MORNING].landMonsInfo != NULL);
    EXPECT(header->encounterTypes[TIME_NIGHT].landMonsInfo != NULL);
    EXPECT(header->encounterTypes[TIME_MORNING].landMonsInfo != header->encounterTypes[TIME_NIGHT].landMonsInfo);
}
