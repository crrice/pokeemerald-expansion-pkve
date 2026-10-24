#include "global.h"
#include "test/test.h"
#include "event_data.h"
#include "pokemon.h"
#include "string_util.h"
#include "text.h"
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
TEST("Verdant: QoL Stat Editor edits natures, IVs and abilities from the start")
{
    EXPECT(StatEditor_CanEditNatures(ITEM_QOL_STAT_EDITOR));
    EXPECT(StatEditor_CanEditIVs(ITEM_QOL_STAT_EDITOR));
    EXPECT(StatEditor_CanEditAbilities(ITEM_QOL_STAT_EDITOR));
}

TEST("Verdant: Stat Editor starts with EVs, then gains natures, IVs, then abilities")
{
    EXPECT(!StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(!StatEditor_CanEditIVs(ITEM_STAT_EDITOR));
    EXPECT(!StatEditor_CanEditAbilities(ITEM_STAT_EDITOR));

    FlagSet(FLAG_STAT_EDITOR_NATURE_UNLOCK);
    EXPECT(StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(!StatEditor_CanEditIVs(ITEM_STAT_EDITOR));
    EXPECT(!StatEditor_CanEditAbilities(ITEM_STAT_EDITOR));

    FlagSet(FLAG_STAT_EDITOR_IV_UNLOCK);
    EXPECT(StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(StatEditor_CanEditIVs(ITEM_STAT_EDITOR));
    EXPECT(!StatEditor_CanEditAbilities(ITEM_STAT_EDITOR));

    FlagSet(FLAG_STAT_EDITOR_ABILITY_UNLOCK);
    EXPECT(StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(StatEditor_CanEditIVs(ITEM_STAT_EDITOR));
    EXPECT(StatEditor_CanEditAbilities(ITEM_STAT_EDITOR));
}

TEST("Verdant: Stat Editor's IV upgrade includes natures")
{
    FlagSet(FLAG_STAT_EDITOR_IV_UNLOCK);
    EXPECT(StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(StatEditor_CanEditIVs(ITEM_STAT_EDITOR));
    EXPECT(!StatEditor_CanEditAbilities(ITEM_STAT_EDITOR));
}

TEST("Verdant: Stat Editor's ability upgrade includes natures and IVs")
{
    FlagSet(FLAG_STAT_EDITOR_ABILITY_UNLOCK);
    EXPECT(StatEditor_CanEditNatures(ITEM_STAT_EDITOR));
    EXPECT(StatEditor_CanEditIVs(ITEM_STAT_EDITOR));
    EXPECT(StatEditor_CanEditAbilities(ITEM_STAT_EDITOR));
}

// Ability edits move between the species' ability slots, hidden included, like
// an Ability Capsule or Patch: the personality stays.
TEST("Verdant: Stat Editor cycles through every ability, hidden included")
{
    struct Pokemon mon;
    u32 personality, abilityNum = 0;

    ASSUME(GetSpeciesAbility(SPECIES_ZIGZAGOON, 0) != GetSpeciesAbility(SPECIES_ZIGZAGOON, 1));
    ASSUME(GetSpeciesAbility(SPECIES_ZIGZAGOON, 1) != GetSpeciesAbility(SPECIES_ZIGZAGOON, 2));
    ASSUME(GetSpeciesAbility(SPECIES_ZIGZAGOON, 0) != GetSpeciesAbility(SPECIES_ZIGZAGOON, 2));
    ASSUME(GetSpeciesAbility(SPECIES_ZIGZAGOON, 1) != ABILITY_NONE);
    CreateMon(&mon, SPECIES_ZIGZAGOON, 5, 0, OTID_STRUCT_PRESET(0));
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &abilityNum);
    personality = GetMonData(&mon, MON_DATA_PERSONALITY);

    EXPECT(StatEditor_CycleMonAbility(&mon, TRUE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 1);
    EXPECT(StatEditor_CycleMonAbility(&mon, TRUE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 2);
    EXPECT(StatEditor_CycleMonAbility(&mon, TRUE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 0);
    EXPECT(StatEditor_CycleMonAbility(&mon, FALSE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 2);
    EXPECT_EQ(GetMonData(&mon, MON_DATA_PERSONALITY), personality);
}

TEST("Verdant: Stat Editor skips empty ability slots")
{
    struct Pokemon mon;
    u32 abilityNum = 0;

    ASSUME(GetSpeciesAbility(SPECIES_WOBBUFFET, 1) == ABILITY_NONE);
    ASSUME(GetSpeciesAbility(SPECIES_WOBBUFFET, 2) != ABILITY_NONE);
    CreateMon(&mon, SPECIES_WOBBUFFET, 5, 0, OTID_STRUCT_PRESET(0));
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &abilityNum);

    EXPECT(StatEditor_CycleMonAbility(&mon, TRUE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 2);
    EXPECT(StatEditor_CycleMonAbility(&mon, TRUE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 0);
}

TEST("Verdant: Stat Editor can't change the ability of a species with only one")
{
    struct Pokemon mon;
    u32 abilityNum = 0;

    ASSUME(GetSpeciesAbility(SPECIES_UNOWN, 1) == ABILITY_NONE);
    ASSUME(GetSpeciesAbility(SPECIES_UNOWN, 2) == ABILITY_NONE);
    CreateMon(&mon, SPECIES_UNOWN, 5, 0, OTID_STRUCT_PRESET(0));
    SetMonData(&mon, MON_DATA_ABILITY_NUM, &abilityNum);

    EXPECT(!StatEditor_CycleMonAbility(&mon, TRUE));
    EXPECT(!StatEditor_CycleMonAbility(&mon, FALSE));
    EXPECT_EQ(GetMonData(&mon, MON_DATA_ABILITY_NUM), 0);
}

// Long ability names drop to a narrower font, and the few still too wide are cut
// short, so none runs under the box's right arrow.
TEST("Verdant: Every ability name fits the Stat Editor's ability box")
{
    u32 i, fontId;
    enum Ability ability = ABILITY_NONE;
    u8 name[ABILITY_NAME_LENGTH + 1];

    for (i = 1; i < ABILITIES_COUNT; i++)
    {
        PARAMETRIZE_LABEL("%S", gAbilitiesInfo[i].name) { ability = i; }
    }
    StringCopy(name, gAbilitiesInfo[ability].name);
    fontId = StatEditor_FitBoxText(name);
    EXPECT_LE(GetStringWidth(fontId, name, 0), STAT_EDITOR_BOX_TEXT_WIDTH);
    EXPECT_EQ(StringCompareN(name, gAbilitiesInfo[ability].name, StringLength(name)), 0); // only cut short, never changed
}
