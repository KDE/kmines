#ifndef KMINES_S_H
#define KMINES_S_H

#include "field.h"
#include "dialogs.h"

#include <kconfig.h>

/* status widget */
class KStatus : public QWidget
{
  Q_OBJECT
	
 public :
	KStatus( QWidget *parent=0, const char *name=0 );
	
 public slots:
	void restartGame();
	void newGame(int, int, int);
	void changeCase(int, int);
	void update();
	void updateSmiley(int);
	void endGame(int);
	void getNumbers(int *, int *, int *);
	void options();
	void showHighScores();
	void pauseGame();
  
 signals:
	/* field signals */
	void newField(int, int, int);
	void stopField();
  
	/* update status signals */
	void exleft(const char *);
	void freezeTimer();
	void zeroTimer();
	void startTimer();
	void getTime(int *, int*);
	void setUMark(int);
	void pause();
  
 private:
	QPixmap *s_ok, *s_stress, *s_happy, *s_ohno;
  
	int nb_width, nb_height, nb_mines; /* mines field */
	int uncovered, marked, uncertain;  /* number of cases in each state */

//	QFile *file;
	KConfig *kconf;
	bool isConfigWritable;
	
	QFrame *frame;
	Field  *field;
	
	QPushButton  *smiley;
	QLCDNumber   *left;
	QLabel       *mesg;
	DigitalClock *dg;
  
	void adjustSize();
	void createSmileyPixmap(QPixmap *, QPainter *);
	void exmesg(const char *);
	void setHighScore(int, int, int);
	void errorShow(QString);
};

#endif
