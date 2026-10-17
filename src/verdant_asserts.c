#include "global.h"
#include "constants/hold_effects.h"

// Build-time checks on the IDs and limits our customizations rely on. One home
// for them; the IDs themselves are listed in the save-system skill's ID registry
// (.claude/skills/save-system/SKILL.md). A failure names the broken rule.

// Upstream documents that HOLD_EFFECT_COUNT stays at or below 127 (constants/hold_effects.h).
// We add no hold effects of our own, so this only trips if upstream grows past its own limit.
STATIC_ASSERT(HOLD_EFFECT_COUNT <= 127, HoldEffectCountExceedsUpstreamLimit);

// Config flags and vars we assign in config/verdant.h: each one must be a real
// saved flag (not a temp flag, which is cleared on every map change) and no two
// may share an ID (B_FLAG_FOLLOWERS_DISABLED once sat on DN_FLAG_SEARCHING, which
// DexNav clears on every map transition). config/test.h moves config flags to
// TESTING_FLAG_* IDs outside the saved range in test builds, so the check is for
// the game build only.
#if !TESTING

#define ASSERT_SAVED_FLAG(flag) STATIC_ASSERT((flag) > TEMP_FLAGS_END && (flag) < FLAGS_COUNT, flag##_IsNotASavedFlag)
#define ASSERT_DISTINCT(a, b) STATIC_ASSERT((a) != (b), a##_SharesAnIdWith_##b)

ASSERT_SAVED_FLAG(DN_FLAG_SEARCHING);
ASSERT_SAVED_FLAG(DN_FLAG_DEXNAV_GET);
ASSERT_SAVED_FLAG(DN_FLAG_DETECTOR_MODE);
ASSERT_SAVED_FLAG(I_EXP_SHARE_FLAG);
ASSERT_SAVED_FLAG(B_FLAG_FOLLOWERS_DISABLED);

ASSERT_DISTINCT(DN_FLAG_SEARCHING, DN_FLAG_DEXNAV_GET);
ASSERT_DISTINCT(DN_FLAG_SEARCHING, DN_FLAG_DETECTOR_MODE);
ASSERT_DISTINCT(DN_FLAG_SEARCHING, I_EXP_SHARE_FLAG);
ASSERT_DISTINCT(DN_FLAG_SEARCHING, B_FLAG_FOLLOWERS_DISABLED);
ASSERT_DISTINCT(DN_FLAG_DEXNAV_GET, DN_FLAG_DETECTOR_MODE);
ASSERT_DISTINCT(DN_FLAG_DEXNAV_GET, I_EXP_SHARE_FLAG);
ASSERT_DISTINCT(DN_FLAG_DEXNAV_GET, B_FLAG_FOLLOWERS_DISABLED);
ASSERT_DISTINCT(DN_FLAG_DETECTOR_MODE, I_EXP_SHARE_FLAG);
ASSERT_DISTINCT(DN_FLAG_DETECTOR_MODE, B_FLAG_FOLLOWERS_DISABLED);
ASSERT_DISTINCT(I_EXP_SHARE_FLAG, B_FLAG_FOLLOWERS_DISABLED);

STATIC_ASSERT(DN_VAR_SPECIES >= VARS_START && DN_VAR_SPECIES <= VARS_END, DN_VAR_SPECIES_IsNotASavedVar);
STATIC_ASSERT(DN_VAR_STEP_COUNTER >= VARS_START && DN_VAR_STEP_COUNTER <= VARS_END, DN_VAR_STEP_COUNTER_IsNotASavedVar);
ASSERT_DISTINCT(DN_VAR_SPECIES, DN_VAR_STEP_COUNTER);

// Weather moves and abilities can replace area weather (BALANCE.md, Weather > Duration).
// Upstream's Gen 9 default blocks them while overworld weather is present.
STATIC_ASSERT(B_OVERWORLD_WEATHER_OVERRIDE < GEN_9, VerdantWeatherIsOverridable);

#endif // !TESTING
