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

#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "defines.h"
#include "solver/bfield.h"
#include "dialogs.h"


using namespace KMines;

//-----------------------------------------------------------------------------
class Field : public QFrame, public BaseField
{
 Q_OBJECT
 public:
	Field(QWidget *parent);

	QSize sizeHint() const;
	QSizePolicy sizePolicy() const;

    void setLevel(const Level &level);
    const Level &level() const { return _level; }

	void reset();

    bool isActive() const { return _state==Playing || _state==Stopped; }
	bool isPaused() const { return _state==Paused; }
	void pause();
	void gameOver() { _state = GameOver; }

    void moveCursor(Neighbour);
    void moveToEdge(Neighbour);
	void doReveal() { doReveal(_cursor); }
	void doMark()   { doMark(_cursor); }
	void doUmark()  { doUmark(_cursor); }
	void keyboardAutoReveal();

	void readSettings();
	void setCaseSize(uint s);
	uint caseSize() const { return _cp.size; }

    void setAdvised(const Grid2D::Coord &c, double proba);

 signals:
	void updateStatus(bool);
	void gameStateChanged(GameState, bool won);
    void incActions();
    void setMood(Smiley::Mood);
    void setCheating();

 protected:
	void paintEvent(QPaintEvent *);
	void mousePressEvent(QMouseEvent *);
	void mouseReleaseEvent(QMouseEvent *);
	void mouseMoveEvent(QMouseEvent *);

 private slots:
	void keyboardAutoRevealSlot();

 private:
	GameState      _state;
	bool           _cursorShown, _reveal, _completeReveal;
    Grid2D::Coord  _cursor, _advised;
    double         _advisedProba;
	MouseAction    _mb[3];
	MouseAction    _currentAction;
	CaseProperties _cp;
	QPixmap        _pm_flag, _pm_mine, _pm_exploded, _pm_error;
    QPixmap        _pm_advised[5];
	QPushButton    _button;
    Level          _level;

	void minePixmap(QPixmap &, bool mask, CaseState) const;
	void pressCase(const Grid2D::Coord &, bool);
	void pressClearFunction(const Grid2D::Coord &, bool);
	void placeCursor(const Grid2D::Coord &);
	void flagPixmap(QPixmap &, bool mask) const;
    void advisedPixmap(QPixmap &, bool mask, uint i) const;
	void revealActions(bool press);

    void doAutoReveal(const Grid2D::Coord &);
    void doReveal(const Grid2D::Coord &);
    void doMark(const Grid2D::Coord &);
    void doUmark(const Grid2D::Coord &);
    void changeCase(const Grid2D::Coord &, CaseState newState);

    QPoint toPoint(const Grid2D::Coord &) const;
    Grid2D::Coord fromPoint(const QPoint &) const;

	void drawCase(QPainter &, const Grid2D::Coord &) const;
	void drawBox(QPainter &, const Grid2D::Coord &, bool, const QPixmap * = 0,
                 const QString &text = QString::null,
                 const QColor &textColor = black) const;
    void drawItem(QPainter &, const QPoint &, const QPixmap *,
                  const QString &text = QString::null,
                  const QColor &textColor = black) const;

	void setCaseProperties(const CaseProperties &);
	MouseAction mapMouseButton(QMouseEvent *) const;
    void resetAdvised();
    void setState(GameState);
};

#endif // FIELD_H
