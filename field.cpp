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

#include "field.h"
#include "field.moc"

#include <math.h>

#include <qlayout.h>
#include <qtimer.h>
#include <qpainter.h>

#include <klocale.h>
#include <knotifyclient.h>

#include "settings.h"
#include "solver/solver.h"
#include "dialogs.h"


using namespace KGrid2D;

const Field::ActionData Field::ACTION_DATA[Nb_Actions] = {
  { "Reveal",         "reveal",          I18N_NOOP("Case revealed")       },
  { "AutoReveal",     "autoreveal",      I18N_NOOP("Case autorevealed")   },
  { "SetFlag",        "mark",            I18N_NOOP("Flag set")            },
  { "UnsetFlag",      "unmark",          I18N_NOOP("Flag unset")          },
  { "SetUncertain",   "set_uncertain",   I18N_NOOP("Question mark set")   },
  { "UnsetUncertain", "unset_uncertain", I18N_NOOP("Question mark unset") }
};

Field::Field(QWidget *parent)
    : FieldFrame(parent), _state(Init), _solvingState(Regular), _level(Level::Easy)
{}

void Field::readSettings()
{
    if ( inside(_cursor) ) {
        QPainter p(this);
        drawCase(p, _cursor);
    }
    if ( Settings::magicReveal() ) emit setCheating();
}

QSize Field::sizeHint() const
{
  return QSize(2*frameWidth() + _level.width()*Settings::caseSize(),
               2*frameWidth() + _level.height()*Settings::caseSize());
}

void Field::setLevel(const Level &level)
{
    _level = level;
    reset(false);
    adjustSize();
}

void Field::setReplayField(const QString &field)
{
    setState(Replaying);
    initReplay(field);
}

void Field::setState(GameState state)
{
    Q_ASSERT( state!=GameOver );
    emit gameStateChanged(state);
    _state = state;
}

void Field::reset(bool init)
{
    BaseField::reset(_level.width(), _level.height(), _level.nbMines());
    if ( init || _state==Init ) setState(Init);
    else setState(Stopped);
    if (Settings::magicReveal()) emit setCheating();
    _currentAction = Settings::EnumMouseAction::None;
    _reveal = false;
    _cursor.first = _level.width()/2;
    _cursor.second = _level.height()/2;
    _advisedCoord = Coord(-1, -1);
    update();
}

void Field::paintEvent(QPaintEvent *e)
{
	QPainter painter(this);
	drawFrame(&painter);
	if ( _state==Paused ) return;

    Coord min = fromPoint(e->rect().topLeft());
    bound(min);
    Coord max = fromPoint(e->rect().bottomRight());
    bound(max);
	for (short i=min.first; i<=max.first; i++)
	    for (short j=min.second; j<=max.second; j++)
            drawCase(painter, Coord(i,j));
}

void Field::changeCase(const Coord &p, CaseState newState)
{
    BaseField::changeCase(p, newState);
    QPainter painter(this);
	drawCase(painter, p);
	if ( isActive() ) emit updateStatus( hasMine(p) );
}

QPoint Field::toPoint(const Coord &p) const
{
    QPoint qp;
    qp.setX( p.first*Settings::caseSize() + frameWidth() );
    qp.setY( p.second*Settings::caseSize() + frameWidth() );
    return qp;
}

Coord Field::fromPoint(const QPoint &qp) const
{
	double i = (double)(qp.x() - frameWidth()) / Settings::caseSize();
    double j = (double)(qp.y() - frameWidth()) / Settings::caseSize();
    return Coord((int)floor(i), (int)floor(j));
}

int Field::mapMouseButton(QMouseEvent *e) const
{
	switch (e->button()) {
	case Qt::LeftButton:  return Settings::mouseAction(Settings::EnumButton::left);
	case Qt::MidButton:   return Settings::mouseAction(Settings::EnumButton::mid);
	case Qt::RightButton: return Settings::mouseAction(Settings::EnumButton::right);
	default:              return Settings::EnumMouseAction::ToggleFlag;
	}
}

void Field::revealActions(bool press)
{
    if ( _reveal==press ) return; // avoid flicker
    _reveal = press;

	switch (_currentAction) {
	case Reveal:
		pressCase(_cursor, press);
		break;
	case AutoReveal:
		pressClearFunction(_cursor, press);
		break;
	default:
		break;
	}
}

void Field::mousePressEvent(QMouseEvent *e)
{
    if ( !isActive() || (_currentAction!=Settings::EnumMouseAction::None) ) return;

    emit setMood(Stressed);
    _currentAction = mapMouseButton(e);

    Coord p = fromPoint(e->pos());
    if ( !inside(p) ) return;
    placeCursor(p);
    revealActions(true);
}

void Field::mouseReleaseEvent(QMouseEvent *e)
{
    if ( !isActive() ) return;

    int tmp = _currentAction;
    emit setMood(Normal);
    revealActions(false);
    int ma = mapMouseButton(e);
    _currentAction = Settings::EnumMouseAction::None;
    if ( ma!=tmp ) return;

    Coord p = fromPoint(e->pos());
    if ( !inside(p) ) return;
    placeCursor(p);

    switch (ma) {
    case Settings::EnumMouseAction::ToggleFlag:          doMark(p); break;
    case Settings::EnumMouseAction::ToggleUncertainFlag: doUmark(p); break;
    case Settings::EnumMouseAction::Reveal:              doReveal(p); break;
    case Settings::EnumMouseAction::AutoReveal:          doAutoReveal(p); break;
    default: break;
    }
}

void Field::mouseMoveEvent(QMouseEvent *e)
{
	if ( !isActive() ) return;

    Coord p = fromPoint(e->pos());
    if ( p==_cursor ) return; // avoid flicker

	revealActions(false);
    if ( !inside(p) ) return;
    placeCursor(p);
	revealActions(true);
}

void Field::pressCase(const Coord &c, bool pressed)
{
	if ( state(c)==Covered ) {
        QPainter painter(this);
        drawCase(painter, c, pressed);
    }
}

void Field::pressClearFunction(const Coord &p, bool pressed)
{
    pressCase(p, pressed);
    CoordList n = coveredNeighbours(p);
    QPainter painter(this);
    for (CoordList::const_iterator it=n.begin(); it!=n.end(); ++it)
        drawCase(painter, *it, pressed);
}

void Field::keyboardAutoReveal()
{
	_cursor_back = _cursor;
	pressClearFunction(_cursor_back, true);
	QTimer::singleShot(50, this, SLOT(keyboardAutoRevealSlot()));
}

void Field::keyboardAutoRevealSlot()
{
	pressClearFunction(_cursor_back, false);
	doAutoReveal(_cursor_back);
}

void Field::doAutoReveal(const Coord &c)
{
	if ( !isActive() ) return;
    if ( state(c)!=Uncovered ) return;
    emit addAction(c, AutoReveal);
    resetAdvised();
    doAction(AutoReveal, c, Settings::magicReveal());
}

void Field::pause()
{
    switch (_state) {
    case Paused:  setState(Playing); break;
    case Playing: setState(Paused); break;
    default: return;
    }
    update();
}

void Field::moveCursor(Neighbour n)
{
    Coord c = neighbour(_cursor, n);
    if ( inside(c) ) placeCursor(c);
}

void Field::moveToEdge(Neighbour n)
{
    Coord c = toEdge(_cursor, n);
    if ( inside(c) ) placeCursor(c);
}

bool Field::doReveal(const Coord &c, CoordList *autorevealed,
                     bool *caseUncovered)
{
	if ( !isActive() ) return true;
    if ( state(c)!=Covered ) return true;
    if ( firstReveal() ) setState(Playing);
    CaseState state =
        doAction(Reveal, c, Settings::magicReveal(), autorevealed, caseUncovered);
    emit addAction(c, Reveal);
    return ( state!=Error );
}

void Field::doMark(const Coord &c)
{
	if ( !isActive() ) return;
    ActionType action;
    CaseState oldState = state(c);
    switch (oldState) {
	case Covered:   action = SetFlag; break;
	case Marked:    action = (Settings::uncertainMark() ? SetUncertain : UnsetFlag); break;
	case Uncertain:	action = UnsetUncertain; break;
	default:        return;
	}
    CaseState newState = doAction(action, c, Settings::magicReveal());
    addMarkAction(c, newState, oldState);
}

void Field::doUmark(const Coord &c)
{
	if ( !isActive() ) return;
    ActionType action;
    CaseState oldState = state(c);
    switch (oldState) {
	case Covered:
	case Marked:    action = SetUncertain; break;
	case Uncertain: action = UnsetUncertain; break;
	default:        return;
	}
    CaseState newState = doAction(action, c, Settings::magicReveal());
    addMarkAction(c, newState, oldState);
}


KMines::CaseState Field::doAction(ActionType type, const Coord &c,
                                  bool complete, CoordList *autorevealed,
                                  bool *caseUncovered)
{
    resetAdvised();
    CaseState state = Error;
    if ( _solvingState==Solved ) complete = false;

    KNotifyClient::event(winId(), ACTION_DATA[type].event,
                         i18n(ACTION_DATA[type].eventMessage));
    switch (type) {
    case Reveal:
        if ( !reveal(c, autorevealed, caseUncovered) )
            emit gameStateChanged(GameOver);
        else {
            state = Uncovered;
            if (complete) completeReveal();
        }
        break;
    case AutoReveal:
        if ( !autoReveal(c, caseUncovered) )
            emit gameStateChanged(GameOver);
        else {
            state = Uncovered;
            if (complete) completeReveal();
        }
        break;
    case SetFlag:
        state = Marked;
        if (complete) completeReveal();
        break;
    case UnsetFlag:
    case UnsetUncertain:
        state = Covered;
        break;
    case SetUncertain:
        state = Uncertain;
        break;
    case Nb_Actions:
        Q_ASSERT(false);
        break;
    }

    if ( state!=Error ) changeCase(c, state);
    return state;
}

void Field::addMarkAction(const Coord &c, CaseState newS, CaseState oldS)
{
    switch (newS) {
    case Marked:    emit addAction(c, SetFlag); return;
    case Uncertain: emit addAction(c, SetUncertain); return;
    default: break;
    }
    switch (oldS) {
    case Marked:    emit addAction(c, UnsetFlag); return;
    case Uncertain: emit addAction(c, UnsetUncertain); return;
    default: break;
    }
}

void Field::placeCursor(const Coord &p)
{
    if ( !isActive() ) return;

    Q_ASSERT( inside(p) );
    Coord old = _cursor;
    _cursor = p;
    if ( Settings::keyboardGame() ) {
        QPainter painter(this);
        drawCase(painter, old);
        drawCase(painter, _cursor);
    }
}

void Field::resetAdvised()
{
    if ( !inside(_advisedCoord) ) return;
    QPainter p(this);
    Coord tmp = _advisedCoord;
    _advisedCoord = Coord(-1, -1);
    drawCase(p, tmp);
}

void Field::setAdvised(const Coord &c, double proba)
{
    resetAdvised();
	_solvingState = Advised;
	_advisedCoord = c;
    _advisedProba = proba;
    if ( inside(c) ) {
        QPainter p(this);
        drawCase(p, c);
    }
}

void Field::drawCase(QPainter &painter, const Coord &c, bool pressed) const
{
	Q_ASSERT( inside(c) );

    QString text;
    uint nbMines = 0;
    PixmapType type = NoPixmap;

	switch ( state(c) ) {
    case Covered:
        break;
	case Marked:
        type = FlagPixmap;
        pressed = false;
        break;
	case Error:
        type = ErrorPixmap;
        pressed = true;
        break;
	case Uncertain:
        text = '?';
        pressed = false;
        break;
	case Exploded:
        type = ExplodedPixmap;
        pressed = true;
        break;
	case Uncovered:
        pressed = true;
        if ( hasMine(c) ) type = MinePixmap;
        else {
            nbMines = nbMinesAround(c);
            if (nbMines) text.setNum(nbMines);
        }
	}

    int i = -1;
    if ( c==_advisedCoord ) {
        if ( _advisedProba==1 ) i = 0;
        else if ( _advisedProba>0.75 ) i = 1;
        else if ( _advisedProba>0.5  ) i = 2;
        else if ( _advisedProba>0.25 ) i = 3;
        else i = 4;
    }

    bool hasFocus = ( Settings::keyboardGame() && (c==_cursor) );
    drawBox(painter, toPoint(c), pressed, type, text, nbMines, i, hasFocus);
}
