#include "global.h"
#include "test/test.h"
#include "pokemon.h"
#include "ui_stat_editor.h"

// The stat editor changes a nature the way a Mint does (BALANCE.md QOL): the
// nature that sets stats changes; the personality and what it decides stay.
TEST("Verdant: Stat Editor sets the nature like a Mint")
{
    struct Pokemon mon;
    u32 personality, gender, shiny, abilityNum, adamantAtk;

    CreateMonWithIVs(&mon, SPECIES_AXEW, 50, NATURE_MODEST, OTID_STRUCT_PRESET(0), MAX_PER_STAT_IVS); // personality % 25 is the nature
    personality = GetMonData(&mon, MON_DATA_PERSONALITY);
    gender = GetMonGender(&mon);
    shiny = IsMonShiny(&mon);
    abilityNum = GetMonData(&mon, MON_DATA_ABILITY_NUM);

    StatEditor_SetMonNature(&mon, NATURE_ADAMANT);
    adamantAtk = GetMonData(&mon, MON_DATA_ATK);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_HIDDEN_NATURE), NATURE_ADAMANT);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_PERSONALITY), personality);
    EXPECT_EQ(GetNature(&mon), NATURE_MODEST); // the original nature, which Synchronize passes on
    EXPECT_EQ(GetMonGender(&mon), gender);
    EXPECT_EQ(IsMonShiny(&mon), shiny);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), abilityNum);

    StatEditor_SetMonNature(&mon, NATURE_MODEST);
    EXPECT_GT(adamantAtk, GetMonData(&mon, MON_DATA_ATK)); // stats follow the edited nature
}
