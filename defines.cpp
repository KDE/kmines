#include "defines.h"
#include <stdlib.h>
#include <time.h>
#include <config.h>

const char *HS_GRP[3] = { "Easy level", "Normal level", "Expert level" };

void initRandomWithTime()
{
	srandom(time(0));
}	

int randomInt(int min, int max)
{
	return min + (int)( (max-min+1.0)*random() / (RANDOM_MAX+1.0) );
}
