#ifndef STATUS_H
#define STATUS_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlcdnumber.h>


class Field;
class DigitalClock;

/* status widget */
class KMinesStatus : public QWidget
{
  Q_OBJECT
	
 public :
	KMinesStatus( QWidget *parent=0, const char *name=0 );
	
 public slots:
	void restartGame();
	void newGame(uint width, uint height, uint nbMines);
	void changeCase(uint i, uint j);
	void update(bool);
	void updateSmiley(int);
	void endGame(int);
	void getNumbers(uint &width, uint &height, uint &nbMines);
	void showHighScores();
	void pauseGame() { pause(); };
	void print();
  
 signals:
	/* field signals */
	void newField(uint width, uint height, uint nbMines);
	void stopField();
  
	/* update status signals */
	void exleft(const QString &);
	void freezeTimer();
	void zeroTimer();
	void startTimer();
	void getTime(int &sec, int &min);
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
	void exmesg(const QString &str);
	int  setHighScore(int, int, int);
};

#endif // STATUS_H
