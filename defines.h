#ifndef DEFINES_H
#define DEFINES_H

void initRandomWithTime();
int randomInt(int min, int max);

/* Strings for the configuration file */
extern const char *OP_GRP;
extern const char *OP_UMARK;
extern const char *OP_MENU;
extern const char *OP_LEVEL;
extern const char *HS_GRP[3];
extern const char *HS_NAME;
extern const char *HS_MIN;
extern const char *HS_SEC;

/* States of a case (unsigned int) */
#define NOTHING    0
#define MINE       1
#define COVERED    2
#define UNCERTAIN  4
#define MARKED     8
#define UNCOVERED  16
#define EXPLODED   32
#define ERROR      64                        

/* States of the smiley */
enum SmileyState { OK = 0, STRESS, HAPPY, UNHAPPY };

/* Layout dimensions */
#define CASE_W   20
#define CASE_H   20

#define BORDER   10
#define SEP      10

#define SMILEY_W 29
#define SMILEY_H 29

#define MIN_W    12
#define MIN_H    12

/* Predefined levels */
const unsigned int MODES[3][3] = { /* (width, height, mines) */
		{8,   8, 10}, // Easy
		{16, 16, 40}, // Normal
		{30, 16, 99}  // Expert
};

#endif // DEFINES_H
