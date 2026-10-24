#include "global.h"
#include "test/test.h"
#include "event_data.h"
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

// What each editor item can edit (BALANCE.md QOL). Save blocks start cleared,
// so no unlock flag is set until a test sets it.
TEST("Verdant: QoL Stat Editor edits natures and IVs from the start")
{
    EXPECT(StatEditor_CanEditNatures(ITEM_QOL_STAT_EDITOR));
    EXPECT(StatEditor_CanEditIVs(ITEM_QOL_STAT_EDITOR));
}

TEST("Verdant: Stat Editor starts with EVs, then gains natures, then IVs")
{
    EXPECT(!StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(!StatEditor_CanEditIVs(ITEM_STAT_EDITOR));

    FlagSet(FLAG_STAT_EDITOR_NATURE_UNLOCK);
    EXPECT(StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(!StatEditor_CanEditIVs(ITEM_STAT_EDITOR));

    FlagSet(FLAG_STAT_EDITOR_IV_UNLOCK);
    EXPECT(StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(StatEditor_CanEditIVs(ITEM_STAT_EDITOR));
}

TEST("Verdant: Stat Editor's IV upgrade includes natures")
{
    FlagSet(FLAG_STAT_EDITOR_IV_UNLOCK);
    EXPECT(StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(StatEditor_CanEditIVs(ITEM_STAT_EDITOR));
}
