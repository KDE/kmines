#ifndef KMINES_D_H
#define KMINES_D_H

#include <qstring.h>
#include <klocale.h>
#include <kapp.h>

/* Version name and date */
#define NAME "kmines"
#define SNAME "kmines 0.6.10 alpha"
#define SDATE "21/01/98"

/* Strings for the configuration file */
#define OP_GRP "Options"
#define OP_UMARK_KEY "? mark"
const QString HS_GRP[3] = { 
	"Easy level", "Normal level", "Expert level"
};
#define HS_NAME_KEY "Name"
#define HS_MIN_KEY "Min"
#define HS_SEC_KEY "Sec"
#define OP_MENUBAR_VIS "menubar visible"

/* Strings for key management */
#define K_KMINES "kmines"
#define K_CUSTOM "custom"
#define K_HS "highscores"
#define K_OP "options"

/* States of a case */
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

#define STATUS_H 60
#define FRAME_W   9
#define LABEL_H  25

#define LCD_W    90
#define LCD_H    40
#define SMILEY_W 29
#define SMILEY_H 29

#define MIN_W    12
#define MIN_H    12

#define LABEL_WC 10
#define LABEL_HC 10

/* Predefined levels */
#define EASY   0
#define NORMAL 1
#define EXPERT 2 

/* (width, height, mines) */
const int MODES[3][3] =
{
	  {8, 8, 10},
	  {16, 16, 40},
	  {30, 16, 99}
};

/* Useful macros */
#define ADD_LABEL( str, x, y, w, h) \
    { label = new QLabel(str, this); \
      label->setGeometry(x, y, w, h); }
 
#endif
