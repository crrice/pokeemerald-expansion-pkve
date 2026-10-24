#ifndef GUARD_UI_STAT_EDITOR_H
#define GUARD_UI_STAT_EDITOR_H

#include "main.h"
#include "constants/items.h"

struct Pokemon;

bool32 StatEditor_CanEditNatures(enum Item item);
bool32 StatEditor_CanEditIVs(enum Item item);
bool32 StatEditor_CanEditAbilities(enum Item item);
void StatEditor_Init(MainCallback callback);
void StatEditor_SetMonNature(struct Pokemon *mon, u8 nature);
bool32 StatEditor_CycleMonAbility(struct Pokemon *mon, bool32 next);

#endif // GUARD_UI_STAT_EDITOR_H
