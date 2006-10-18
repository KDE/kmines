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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef FIELD_H
#define FIELD_H

#include "solver/bfield.h"
#include "frame.h"


//-----------------------------------------------------------------------------
class Field : public FieldFrame, public BaseField
{
 Q_OBJECT
 public:
    enum ActionType { Reveal = 0, AutoReveal, SetFlag, UnsetFlag, SetUncertain,
                      UnsetUncertain, Nb_Actions };
    struct ActionData {
      const char *name, *event, *eventMessage;
    };
    static const ActionData ACTION_DATA[Nb_Actions];

 public:
    Field(QWidget *parent);

    virtual QSize sizeHint() const;

    void setLevel(const Level &level);
    void setReplayField(const QString &field);
    const Level &level() const { return _level; }
    void reset(bool init);

    GameState gameState() const { return _state; }
    bool isActive() const { return _state!=Paused && _state!=GameOver; }
    void pause();
    void setGameOver() { _state = GameOver; }

    void moveCursor(Neighbour);
    void moveToEdge(Neighbour);
    void doReveal() { doReveal(_cursor); }
    void doMark()   { doMark(_cursor); }
    void doUmark()  { doUmark(_cursor); }
    void keyboardAutoReveal();
    CaseState doAction(ActionType type, const KGrid2D::Coord &c,
                     bool completeReveal, KGrid2D::CoordList *autorevealed = 0,
                     bool *caseUncovered = 0);

	void readSettings();

    void setAdvised(const KGrid2D::Coord &c, double proba);
	void setSolvingState(SolvingState state) { _solvingState = state; }
	SolvingState solvingState() const { return _solvingState; }
	
 signals:
	void updateStatus(bool);
	void gameStateChanged(GameState);
    void setMood(Mood);
    void setCheating();
    void addAction(const KGrid2D::Coord &, Field::ActionType);

 protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);

 private slots:
	void keyboardAutoRevealSlot();

 private:
	GameState   _state;
	bool              _reveal;
	SolvingState _solvingState;
	KGrid2D::Coord  _cursor, _cursor_back, _advisedCoord;
	double        _advisedProba;
	int               _currentAction;
	Level          _level;

	void pressCase(const KGrid2D::Coord &, bool);
	void pressClearFunction(const KGrid2D::Coord &, bool);
	void placeCursor(const KGrid2D::Coord &);
	void revealActions(bool press);

    void doAutoReveal(const KGrid2D::Coord &);
    bool doReveal(const KGrid2D::Coord &, KGrid2D::CoordList *autorevealed = 0,
                  bool *caseUncovered = 0);
    void doMark(const KGrid2D::Coord &);
    void doUmark(const KGrid2D::Coord &);
    void changeCase(const KGrid2D::Coord &, CaseState newState);
    void addMarkAction(const KGrid2D::Coord &, CaseState newS, CaseState oldS);

    QPoint toPoint(const KGrid2D::Coord &) const;
    KGrid2D::Coord fromPoint(const QPoint &) const;

    void drawCase(QPainter &, const KGrid2D::Coord &,
                  bool forcePressed = false) const;

    int mapMouseButton(QMouseEvent *) const;
    void resetAdvised();
    void setState(GameState);
};

#endif // FIELD_H
