#ifndef STATUS_H
#define STATUS_H

#include <qwidget.h>
#include <qframe.h>
#include <qpushbutton.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qwidgetstack.h>

#include "field.h"
#include "defines.h"


class DigitalClock;
class LCDNumber;

class Status : public QWidget, public KMines
{
 Q_OBJECT
 public :
	Status(QWidget *parent=0, const char *name=0);

	const Level &currentLevel() const { return field->level(); }
    bool isPaused() const             { return field->isPaused(); }
    void settingsChanged();

 signals:
	void gameStateChanged(GameState);

 public slots:
    void newGame(int type);
	void restartGame();
	void changeCase(CaseState, int inc);
	void update(bool);
	void gameLost()       { _endGame(false); }
	void pauseGame()      { field->pause(); }
	void print();

	void moveUp()     { field->up(); }
	void moveDown()   { field->down(); }
	void moveLeft()   { field->left(); }
	void moveRight()  { field->right(); }
	void reveal()     { field->reveal(); }
	void mark()       { field->mark(); }
	void autoReveal() { field->keyboardAutoReveal(); }

 private slots:
    void gameStateChangedSlot(GameState);
    void smileyClicked();

 private:
	uint uncovered, marked, uncertain;  /* number of cases in each state */

	Field        *field;
    QWidget      *_fieldContainer, *_resumeContainer;
    QWidgetStack *_stack;

	Smiley       *smiley;
	LCDNumber    *left;
	DigitalClock *dg;

	void exmesg(const QString &);
	void initGame();
	void _endGame(bool win);
};

#endif // STATUS_H
