#ifndef STATUS_H
#define STATUS_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include "field.h"

class DigitalClock;
class LCDNumber;

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
	void endGame(int);
	void showHighScores() { highScores(0); }
	void pauseGame() { field->pause(); }
	void print();
	
 private:
	enum GameType { Easy = 0, Normal, Expert, Custom };
	
	uint     uncovered, marked, uncertain;  /* number of cases in each state */
	Field    *field;
	GameType _type;
	
	Smiley       *smiley;
	LCDNumber    *left;
	QLabel       *message;
	DigitalClock *dg;
  
	void exmesg(const QString &);
	void highScores(const Score *);
	void initGame();
};

#endif // STATUS_H
