#include "defines.h"
#include <stdlib.h>
#include <time.h>
#include <config.h>

const char *OP_GRP    = "Options";
const char *OP_UMARK  = "? mark";
const char *OP_MENU   = "menubar visible";
const char *OP_LEVEL  = "Level";
const char *HS_NAME   = "Name";
const char *HS_MIN    = "Min";
const char *HS_SEC    = "Sec";
const char *HS_GRP[3] = { "Easy level", "Normal level", "Expert level" };

const Level LEVELS[3] = {
	{8,   8, 10}, // Easy
	{16, 16, 40}, // Normal
	{30, 16, 99}  // Expert
};

