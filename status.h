/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef STATUS_H
#define STATUS_H

#include <qdom.h>

#include "field.h"


class Smiley;
class KGameLCD;
class DigitalClock;
class Solver;
class QWidgetStack;
class QTimer;

class Status : public QWidget, public KMines
{
 Q_OBJECT
 public :
	Status(QWidget *parent);

	const Level &currentLevel() const { return field->level(); }
    bool isPlaying() const            { return field->gameState()==Playing; }
    void settingsChanged();

    bool checkBlackMark();

 signals:
    void pause();
	void gameStateChangedSignal(KMines::GameState);

 public slots:
    void newGame(int type);
	void restartGame();
	void update(bool);
	void pauseGame()     { field->pause(); }

	void moveUp()        { field->moveCursor(KGrid2D::SquareBase::Up); }
	void moveDown()      { field->moveCursor(KGrid2D::SquareBase::Down); }
	void moveLeft()      { field->moveCursor(KGrid2D::SquareBase::Left); }
	void moveRight()     { field->moveCursor(KGrid2D::SquareBase::Right); }
    void moveLeftEdge()  { field->moveToEdge(KGrid2D::SquareBase::Left); }
    void moveRightEdge() { field->moveToEdge(KGrid2D::SquareBase::Right); }
    void moveTop()       { field->moveToEdge(KGrid2D::SquareBase::Up); }
    void moveBottom()    { field->moveToEdge(KGrid2D::SquareBase::Down); }
	void reveal()        { field->doReveal(); }
	void mark()          { field->doMark(); }
	void autoReveal()    { field->keyboardAutoReveal(); }

    void advise();
    void solve();
    void solveRate();
    void addAction(const KGrid2D::Coord &, Field::ActionType type);

    void viewLog();
    void replayLog();
    void saveLog();
    void loadLog();

 private slots:
    void gameStateChangedSlot(GameState state)
        { gameStateChanged(state, false); }
    void smileyClicked();
    void solvingDone(bool success);
    void replayStep();

 private:
	Field        *field;
    QWidget      *_fieldContainer, *_resumeContainer;
    QWidgetStack *_stack;

	Smiley       *smiley;
	KGameLCD     *left;
	DigitalClock *dg;
    Solver       *_solver;
    bool          _advised, _solved;

    QDomDocument  _log;
    QDomElement   _logRoot, _logList;
    QDomNodeList  _actions;
    uint          _index;
    bool          _completeReveal;
    Level         _oldLevel;
    QTimer       *_timer;

    void setGameOver(bool won);
    void setStopped();
    void setPlaying();
    void newGame(const Level &);
    void gameStateChanged(GameState, bool won);
    static bool checkLog(const QDomDocument &);
};

#endif // STATUS_H
