#ifndef STATUS_H
#define STATUS_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlcdnumber.h>

#include "field.h"
#include "defines.h"

class DigitalClock;
class LCDNumber;

class Status : public QWidget
{
  Q_OBJECT
	
 public :
	Status(QWidget *parent=0, const char *name=0);

	bool newGame(GameType &);

 signals:
	void keyboardEnabled(bool);
	void gameStateChanged(GameState);
	
 public slots:
	void restartGame();
	void changeCase(uint i, uint j);
	void update(bool);
	void endGame(int);
	void showHighScores() { highScores(0); }
	void pauseGame() { field->pause(); }
	void print();
	void preferences();
	void setGameState(GameState);

	void moveUp()     { field->up(); }
	void moveDown()   { field->down(); }
	void moveLeft()   { field->left(); }
	void moveRight()  { field->right(); }
	void reveal()     { field->reveal(); }
	void mark()       { field->mark(); }
	void autoReveal() { field->keyboardAutoReveal(); }

 private:
	uint     uncovered, marked, uncertain;  /* number of cases in each state */
	Field    *field;
	
	Smiley       *smiley;
	LCDNumber    *left;
	QLabel       *message;
	DigitalClock *dg;

	void exmesg(const QString &);
	void highScores(const Score *);
	void initGame();
};

#endif // STATUS_H
