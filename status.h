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
	void setUMark(bool b)  { field->setUMark(b); }
	void setCursor(bool b) { field->setCursor(b); }
	
 public slots:
	void restartGame();
	void changeCase(uint i, uint j);
	void update(bool);
	void endGame(int);
	void showHighScores() { highScores(0); }
	void pauseGame() { field->pause(); }
	void print();
	void options();

	void moveUp()     { field->up(); }
	void moveDown()   { field->down(); }
	void moveLeft()   { field->left(); }
	void moveRight()  { field->right(); }
	void reveal()     { field->reveal(); }
	void mark()       { field->mark(); }
	void autoReveal() { field->autoReveal(); }

 private:
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
