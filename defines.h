#ifndef DEFINES_H
#define DEFINES_H

void initRandomWithTime();
int randomInt(int min, int max);

/* Strings for the configuration file */
#define OP_GRP         "Options"
#define OP_UMARK_KEY   "? mark"
extern const char *HS_GRP[3];
#define HS_NAME_KEY    "Name"
#define HS_MIN_KEY     "Min"
#define HS_SEC_KEY     "Sec"
#define OP_MENUBAR_VIS "menubar visible"

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
#define OK      0
#define STRESS  1
#define HAPPY   2
#define UNHAPPY 3

/* Layout dimensions */
#define CASE_W   20
#define CASE_H   20

#define BORDER   10
#define SEP      10

//#define STAT_H   60
//#define FRAME_W   9
//#define LABEL_H  25

//#define LCD_W    90
//#define LCD_H    40
#define SMILEY_W 29
#define SMILEY_H 29

#define MIN_W    12
#define MIN_H    12

//#define LABEL_WC 10
//#define LABEL_HC 10

/* Predefined levels */
const unsigned int MODES[3][3] = { /* (width, height, mines) */
		{8,   8, 10}, // Easy
		{16, 16, 40}, // Normal
		{30, 16, 99}  // Expert
};

#endif // DEFINES_H
