#ifndef GUARD_UI_STAT_EDITOR_H
#define GUARD_UI_STAT_EDITOR_H

#include "main.h"

struct Pokemon;

void StatEditor_Init(MainCallback callback);
void StatEditor_SetMonNature(struct Pokemon *mon, u8 nature);

#endif // GUARD_UI_STAT_EDITOR_H
