/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef STATUS_H
#define STATUS_H

#include <qdom.h>

#include "field.h"


class Smiley;
class LCD;
class DigitalClock;
class Solver;
class QWidgetStack;

class Status : public QWidget, public KMines
{
 Q_OBJECT
 public :
	Status(QWidget *parent);

	const Level &currentLevel() const { return field->level(); }
    bool isPaused() const             { return field->isPaused(); }
    void settingsChanged();

 signals:
    void pause();
	void gameStateChangedSignal(KMines::GameState);

 public slots:
    void newGame(int type);
	void restartGame();
	void update(bool);
	void pauseGame()     { field->pause(); }

	void moveUp()        { field->moveCursor(Grid2D::SquareBase::Up); }
	void moveDown()      { field->moveCursor(Grid2D::SquareBase::Down); }
	void moveLeft()      { field->moveCursor(Grid2D::SquareBase::Left); }
	void moveRight()     { field->moveCursor(Grid2D::SquareBase::Right); }
    void moveLeftEdge()  { field->moveToEdge(Grid2D::SquareBase::Left); }
    void moveRightEdge() { field->moveToEdge(Grid2D::SquareBase::Right); }
    void moveTop()       { field->moveToEdge(Grid2D::SquareBase::Up); }
    void moveBottom()    { field->moveToEdge(Grid2D::SquareBase::Down); }
	void reveal()        { field->doReveal(); }
	void mark()          { field->doMark(); }
	void autoReveal()    { field->keyboardAutoReveal(); }

    void advise();
    void solve();
    void solveRate();
    void addAction(const Grid2D::Coord &, Field::ActionType type);

 private slots:
    void gameStateChanged(GameState, bool won);
    void smileyClicked();
    void solvingDone(bool success);

 private:
	Field        *field;
    QWidget      *_fieldContainer, *_resumeContainer;
    QWidgetStack *_stack;

	Smiley       *smiley;
	LCD          *left;
	DigitalClock *dg;
    Solver       *_solver;

    QDomDocument  _log;
    QDomElement   _logRoot, _logList;

    void setGameOver(bool won);
    void setStopped();
};

#endif // STATUS_H
