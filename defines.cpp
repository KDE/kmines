#include "defines.h"

#include <klocale.h>


const KMines::LevelData KMines::LEVELS[KMines::NbLevels] = {
	{8,   8, 10, KMines::Easy,   "easy",   "8x8x10",   I18N_NOOP("Easy")   },
	{16, 16, 40, KMines::Normal, "normal", "16x16x40", I18N_NOOP("Normal") },
	{30, 16, 99, KMines::Expert, "expert", "30x16x99", I18N_NOOP("Expert") }
};
