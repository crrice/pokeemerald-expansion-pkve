#include "global.h"
#include "test/battle.h"
#include "item.h"

// Wimpy Brace (custom item, Oldale gift): the holder gains no EVs. It has no
// hold effect of its own; MonGainEVs checks the item ID.

TEST("Verdant: Wimpy Brace holder gains no EVs")
{
    struct Pokemon withBrace, withoutBrace;
    enum Item item = ITEM_WIMPY_BRACE;

    ASSUME(gSpeciesInfo[SPECIES_CATERPIE].evYield_HP == 1);
    CreateMon(&withoutBrace, SPECIES_WOBBUFFET, 5, 0, OTID_STRUCT_PRESET(0));
    CreateMon(&withBrace, SPECIES_WOBBUFFET, 5, 0, OTID_STRUCT_PRESET(0));
    SetMonData(&withBrace, MON_DATA_HELD_ITEM, &item);
    MonGainEVs(&withoutBrace, SPECIES_CATERPIE);
    MonGainEVs(&withBrace, SPECIES_CATERPIE);
    EXPECT_EQ(GetMonData(&withoutBrace, MON_DATA_HP_EV), 1);
    EXPECT_EQ(GetMonData(&withBrace, MON_DATA_HP_EV), 0);
}

TEST("Verdant: Wimpy Brace blocks every stat's EVs")
{
    struct Pokemon mon;
    enum Item item = ITEM_WIMPY_BRACE;

    ASSUME(gSpeciesInfo[SPECIES_HAXORUS].evYield_Attack > 0);
    ASSUME(gSpeciesInfo[SPECIES_BLISSEY].evYield_HP > 0);
    ASSUME(gSpeciesInfo[SPECIES_DEOXYS_NORMAL].evYield_Speed > 0);
    CreateMon(&mon, SPECIES_WOBBUFFET, 5, 0, OTID_STRUCT_PRESET(0));
    SetMonData(&mon, MON_DATA_HELD_ITEM, &item);
    MonGainEVs(&mon, SPECIES_HAXORUS);
    MonGainEVs(&mon, SPECIES_BLISSEY);
    MonGainEVs(&mon, SPECIES_DEOXYS_NORMAL);
    EXPECT_EQ(GetMonEVCount(&mon), 0);
}

TEST("Verdant: Wimpy Brace has no hold effect and keeps its item data")
{
    EXPECT_EQ(GetItemHoldEffect(ITEM_WIMPY_BRACE), HOLD_EFFECT_NONE);
    EXPECT_EQ(GetItemPocket(ITEM_WIMPY_BRACE), POCKET_ITEMS);
    EXPECT_EQ(GetItemPrice(ITEM_WIMPY_BRACE), 3000);
    EXPECT_EQ(GetItemFlingPower(ITEM_WIMPY_BRACE), 60);
}

WILD_BATTLE_TEST("Verdant: Wimpy Brace holder gains no EVs from a battle")
{
    enum Item item = ITEM_NONE;
    u32 expectedEVs = 0;

    PARAMETRIZE { item = ITEM_NONE;        expectedEVs = 1; }
    PARAMETRIZE { item = ITEM_WIMPY_BRACE; expectedEVs = 0; }

    GIVEN {
        ASSUME(gSpeciesInfo[SPECIES_CATERPIE].evYield_HP == 1);
        PLAYER(SPECIES_WOBBUFFET) { Item(item); }
        OPPONENT(SPECIES_CATERPIE) { HP(1); }
    } WHEN {
        TURN { MOVE(player, MOVE_SCRATCH); }
    } SCENE {
        MESSAGE("The wild Caterpie fainted!");
    } THEN {
        EXPECT_EQ(GetMonData(&gParties[B_TRAINER_PLAYER][0], MON_DATA_HP_EV), expectedEVs);
    }
}
