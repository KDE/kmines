#ifndef KMINES_D_H
#define KMINES_D_H

#include <qstring.h>

/* Version name and date */
const char SNAME[] = "kmines 0.6.5 alpha";
const char SDATE[] = "06/05/97";

/* configuration file */
const QString OP_GRP = "Options";
const QString OP_UMARK_KEY = "? mark";
const QString HS_GRP[3] = { 
	"Easy level", "Normal level", "Expert level"
};
const QString HS_NAME_KEY = "Name";
const QString HS_MIN_KEY = "Min";
const QString HS_SEC_KEY = "Sec";
const QString OP_MENUBAR_VIS = "menubar visible";

/* states of a case */
#define NOTHING    0
#define MINE       1
#define COVERED    2
#define UNCERTAIN  4
#define MARKED     8
#define UNCOVERED  16
#define EXPLODED   32
#define ERROR      64                        

/* states of the smiley */
#define OK      0
#define STRESS  1
#define HAPPY   2
#define UNHAPPY 3

/* layout */
#define CASE_W   20

#define STATUS_H 60
#define FRAME_W   9

#define LCD_W    90
#define LCD_H    40
#define SMILEY_W 29
#define SMILEY_H 29

#define MIN_W    12
#define MIN_H    12

#define LABEL_H  20
#define LABEL_W  43

#define LABEL_WC 10
#define LABEL_HC 10

/* predefined levels */
#define EASY   0
#define NORMAL 1
#define EXPERT 2 

/* (width, height, mines) */
const  int MODES[3][3] =
{
	  {8, 8, 10},
	  {16, 16, 40},
	  {30, 16, 99}
};

#define ADD_LABEL( str, x, y, w, h) \
    { label = new QLabel(str, this); \
      label->setGeometry(x, y, w, h); }
 

#endif
