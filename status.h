#ifndef STATUS_H
#define STATUS_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbt.h>
#include <qlabel.h>
#include <qlcdnum.h>


class Field;
class DigitalClock;

/* status widget */
class KStatus : public QWidget
{
  Q_OBJECT
	
 public :
	KStatus( QWidget *parent=0, const char *name=0 );
	
 public slots:
	void restartGame();
	void newGame(uint, uint, uint);
	void changeCase(uint, uint);
	void update(bool);
	void updateSmiley(int);
	void endGame(int);
	void getNumbers(uint *, uint *, uint *);
	void showHighScores();
	void pauseGame() { pause(); };
	void print();
  
 signals:
	/* field signals */
	void newField(uint, uint, uint);
	void stopField();
  
	/* update status signals */
	void exleft(const char *);
	void freezeTimer();
	void zeroTimer();
	void startTimer();
	void getTime(int *, int*);
	void pause();
	
	void quit();
  
 private:
	QPixmap *s_ok, *s_stress, *s_happy, *s_ohno;
  
	uint nb_width, nb_height, nb_mines; /* mines field */
	uint uncovered, marked, uncertain;  /* number of cases in each state */
	
	QFrame *frame;
	Field  *field;
	
	QPushButton  *smiley;
	QLCDNumber   *left;
	QLabel       *mesg;
	DigitalClock *dg;
  
	void adjustSize();
	void createSmileyPixmap(QPixmap *, QPainter *);
	void exmesg(const char *);
	int  setHighScore(int, int, int);
};

#endif // STATUS_H
