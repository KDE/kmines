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

void initRandomWithTime()
{
	srandom(time(0));
}	

int randomInt(int min, int max)
{
	return min + (int)( (max-min+1.0)*random() / (RANDOM_MAX+1.0) );
}
