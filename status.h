#ifndef STATUS_H
#define STATUS_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlcdnumber.h>

#include "field.h"

class DigitalClock;

class Status : public QWidget
{
  Q_OBJECT
	
 public :
	Status(QWidget *parent=0, const char *name=0);

	bool newGame(uint i);
	void changeUMark(bool b) { field->changeUMark(b); }
	
 public slots:
	void restartGame();
	void changeCase(uint i, uint j);
	void update(bool);
	void updateSmiley(int);
	void endGame(int);
	void showHighScores() { highScores(0); }
	void pauseGame() { field->pause(); }
	void print();
	void setMsg(const QString &s) { mesg->setText(s); }
  
 signals:
	void message(const QString &);
	void quit();
	
 private:
	enum GameType { Easy = 0, Normal, Expert, Custom };
	
	QPixmap  s_ok, s_happy, s_ohno, s_stress;
	uint     uncovered, marked, uncertain;  /* number of cases in each state */
	
	Field    *field;
	GameType _type;
	
	QPushButton  *smiley;
	QLCDNumber   *left;
	QLabel       *mesg;
	DigitalClock *dg;
  
	void createSmileyPixmap(QPixmap *, QPainter *);
	void exmesg(const QString &);
	void highScores(const Score *);
	void initGame();
};

#endif // STATUS_H
