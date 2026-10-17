#ifndef GUARD_CONFIG_VERDANT_H
#define GUARD_CONFIG_VERDANT_H

// Verdant Emerald's config choices, all in one place.
//
// Upstream's config/*.h files stay as upstream ships them, so their comment and
// default changes merge cleanly. This header is included at the end of
// constants/global.h, after every config/*.h and before config/test.h (test
// builds keep test.h's values), and overrides their values.
//
// Each option is checked first: an upstream rename or removal becomes a compile
// error here instead of a define that silently does nothing.
//
// Exceptions, for files that read a config header without constants/global.h:
// - OW_TIME_OF_DAY_ENCOUNTERS is overridden at the bottom of config/overworld.h:
//   tools/wild_encounters/wild_encounters_to_header.py reads that file as text.
// - P_GEN_9_POKEMON, P_GIGANTAMAX_FORMS and P_TERA_FORMS stay edited in
//   config/species_enabled.h: data/sound_data.s includes config/pokemon.h (and with
//   it species_enabled.h) directly, and its cry tables are gated on them.

#include "constants/flags.h"
#include "constants/vars.h"

#if !defined(B_TRAINERS_KNOCK_OFF_ITEMS)
#error "config/battle.h: B_TRAINERS_KNOCK_OFF_ITEMS was renamed or removed upstream"
#endif
#if !defined(DEXNAV_ENABLED) || !defined(DN_FLAG_SEARCHING) || !defined(DN_FLAG_DEXNAV_GET) || !defined(DN_FLAG_DETECTOR_MODE) || !defined(DN_VAR_SPECIES) || !defined(DN_VAR_STEP_COUNTER)
#error "config/dexnav.h: a DexNav option was renamed or removed upstream"
#endif
#if !defined(I_EXP_SHARE_FLAG)
#error "config/item.h: I_EXP_SHARE_FLAG was renamed or removed upstream"
#endif
#if !defined(OW_FOLLOWERS_ENABLED) || !defined(B_FLAG_FOLLOWERS_DISABLED) || !defined(OW_USE_FAKE_RTC) || !defined(OW_ALTERED_TIME_RATIO)
#error "config/overworld.h: an option was renamed or removed upstream"
#endif
#if !defined(P_LVL_UP_LEARNSETS) || !defined(P_SHEDINJA_BALL) || !defined(P_SHUCKLE_BERRY_JUICE) || !defined(P_CAN_FORGET_HIDDEN_MOVE) || !defined(P_SHOW_DYNAMIC_TYPES)
#error "config/pokemon.h: an option was renamed or removed upstream"
#endif
#if !defined(P_SUMMARY_SCREEN_IV_EV_INFO) || !defined(P_SUMMARY_SCREEN_IV_EV_VALUES)
#error "config/summary_screen.h: an option was renamed or removed upstream"
#endif

// config/battle.h
#undef B_TRAINERS_KNOCK_OFF_ITEMS
#define B_TRAINERS_KNOCK_OFF_ITEMS      FALSE

// config/dexnav.h
#undef DEXNAV_ENABLED
#define DEXNAV_ENABLED                  TRUE
#undef DN_FLAG_SEARCHING
#define DN_FLAG_SEARCHING               FLAG_UNUSED_0x020
#undef DN_FLAG_DEXNAV_GET
#define DN_FLAG_DEXNAV_GET              FLAG_UNUSED_0x023
#undef DN_FLAG_DETECTOR_MODE
#define DN_FLAG_DETECTOR_MODE           FLAG_UNUSED_0x022
#undef DN_VAR_SPECIES
#define DN_VAR_SPECIES                  VAR_UNUSED_0x404E
#undef DN_VAR_STEP_COUNTER
#define DN_VAR_STEP_COUNTER             VAR_UNUSED_0x4083

// config/item.h
#undef I_EXP_SHARE_FLAG
#define I_EXP_SHARE_FLAG                FLAG_UNUSED_0x021

// config/overworld.h
#undef OW_FOLLOWERS_ENABLED
#define OW_FOLLOWERS_ENABLED            TRUE
#undef B_FLAG_FOLLOWERS_DISABLED
#define B_FLAG_FOLLOWERS_DISABLED       FLAG_UNUSED_0x02A  // not 0x020: DexNav clears DN_FLAG_SEARCHING on every map transition
#undef OW_USE_FAKE_RTC
#define OW_USE_FAKE_RTC                 TRUE
#undef OW_ALTERED_TIME_RATIO
#define OW_ALTERED_TIME_RATIO           GEN_8_PLA

// config/pokemon.h
#undef P_LVL_UP_LEARNSETS
#define P_LVL_UP_LEARNSETS              GEN_9  // our learnset edits live in level_up_learnsets/gen_9.h
#undef P_SHEDINJA_BALL
#define P_SHEDINJA_BALL                 GEN_3
#undef P_SHUCKLE_BERRY_JUICE
#define P_SHUCKLE_BERRY_JUICE           GEN_2
#undef P_CAN_FORGET_HIDDEN_MOVE
#define P_CAN_FORGET_HIDDEN_MOVE        TRUE
#undef P_SHOW_DYNAMIC_TYPES
#define P_SHOW_DYNAMIC_TYPES            TRUE

// config/summary_screen.h
#undef P_SUMMARY_SCREEN_IV_EV_INFO
#define P_SUMMARY_SCREEN_IV_EV_INFO     TRUE
#undef P_SUMMARY_SCREEN_IV_EV_VALUES
#define P_SUMMARY_SCREEN_IV_EV_VALUES   TRUE

#endif // GUARD_CONFIG_VERDANT_H
