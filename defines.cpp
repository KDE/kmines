#include "defines.h"

#include <klocale.h>


const LevelData LEVELS[NbLevels] = {
	{8,   8, 10, Easy,   "easy",   "8x8x10",   I18N_NOOP("Easy")   },
	{16, 16, 40, Normal, "normal", "16x16x40", I18N_NOOP("Normal") },
	{30, 16, 99, Expert, "expert", "30x16x99", I18N_NOOP("Expert") }
};
