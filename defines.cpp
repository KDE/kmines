#include "defines.h"

const char *OP_GRP       = "Options";
const char *OP_UMARK     = "? mark";
const char *OP_MENUBAR   = "menubar visible";
const char *OP_LEVEL     = "Level";
const char *OP_CASE_SIZE = "case size";
const char *OP_KEYBOARD  = "keyboard game";

const char *HS_NAME   = "Name";
const char *HS_MIN    = "Min";
const char *HS_SEC    = "Sec";
const char *HS_GRP[3] = { "Easy level", "Normal level", "Expert level" };

const uint CASE_SIZE     = 20;
const uint MIN_CASE_SIZE = 20;
const uint MAX_CASE_SIZE = 100;

const Level LEVELS[NbLevels-1] = {
	{8,   8, 10}, // Easy
	{16, 16, 40}, // Normal
	{30, 16, 99}  // Expert
};
