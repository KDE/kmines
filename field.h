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
    static const char *ACTION_NAMES[Nb_Actions];

 public:
	Field(QWidget *parent);

	QSize sizeHint() const;
	QSizePolicy sizePolicy() const;

    void setLevel(const Level &level);
    void setReplayField(const QString &field);
    const Level &level() const { return _level; }
	void reset(bool init);

    GameState gameState() const { return _state; }
    bool isActive() const { return _state!=Paused && _state!=GameOver; }
	void pause();
	void setGameOver() { _state = GameOver; }
    bool hasCompleteReveal() const { return _completeReveal; }

    void moveCursor(Neighbour);
    void moveToEdge(Neighbour);
	void doReveal() { doReveal(_cursor); }
	void doMark()   { doMark(_cursor); }
	void doUmark()  { doUmark(_cursor); }
	void keyboardAutoReveal();
    CaseState doAction(ActionType type, const Grid2D::Coord &c,
                       bool completeReveal, Grid2D::CoordSet *autorevealed = 0,
                       bool *caseUncovered = 0);

	void readSettings();

    void setAdvised(const Grid2D::Coord &c, double proba);

 signals:
	void updateStatus(bool);
	void gameStateChanged(GameState);
    void setMood(Mood);
    void setCheating();
    void addAction(const Grid2D::Coord &, Field::ActionType);

 protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);

 private slots:
	void keyboardAutoRevealSlot();

 private:
	GameState      _state;
	bool           _cursorShown, _reveal, _completeReveal, _umark;
    Grid2D::Coord  _cursor, _advised;
    double         _advisedProba;
	MouseAction    _mb[3];
	MouseAction    _currentAction;
    Level          _level;

	void pressCase(const Grid2D::Coord &, bool);
	void pressClearFunction(const Grid2D::Coord &, bool);
	void placeCursor(const Grid2D::Coord &);
	void revealActions(bool press);

    void doAutoReveal(const Grid2D::Coord &);
    bool doReveal(const Grid2D::Coord &, Grid2D::CoordSet *autorevealed = 0,
                  bool *caseUncovered = 0);
    void doMark(const Grid2D::Coord &);
    void doUmark(const Grid2D::Coord &);
    void changeCase(const Grid2D::Coord &, CaseState newState);
    void addMarkAction(const Grid2D::Coord &, CaseState newS, CaseState oldS);

    QPoint toPoint(const Grid2D::Coord &) const;
    Grid2D::Coord fromPoint(const QPoint &) const;

	void drawCase(QPainter &, const Grid2D::Coord &,
                  bool forcePressed = false) const;

	MouseAction mapMouseButton(QMouseEvent *) const;
    void resetAdvised();
    void setState(GameState);
};

#endif // FIELD_H
