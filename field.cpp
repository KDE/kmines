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

#include "field.h"
#include "field.moc"

#include <math.h>

#include <qbitmap.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qtimer.h>

#include <klocale.h>

#include "solver/solver.h"


using namespace Grid2D;

Field::Field(QWidget *parent)
    : QFrame(parent, "field"), _state(Stopped), _button(0),
      _level(Level::Easy)
{
	setFrameStyle( QFrame::Box | QFrame::Raised );
	setLineWidth(2);
	setMidLineWidth(2);

    readSettings();
}

void Field::readSettings()
{
	_uMark = GameSettings::readUMark();
    _cursorShown = GameSettings::readKeyboard();
	for (uint i=0; i<3; i++)
		_mb[i] = GameSettings::readMouseBinding((MouseButton)i);
    setCaseProperties( AppearanceSettings::readCaseProperties() );
    _completeReveal = GameSettings::readMagicReveal();
    if (_completeReveal) emit setCheating();
}

void Field::setCaseProperties(const CaseProperties &cp)
{
	_cp = cp;
	_button.resize(cp.size, cp.size);

	QBitmap mask;

	flagPixmap(mask, true);
	flagPixmap(_pm_flag, false);
	_pm_flag.setMask(mask);

	minePixmap(mask, true, Covered);
	minePixmap(_pm_mine, false, Covered);
	_pm_mine.setMask(mask);

	minePixmap(mask, true, Exploded);
	minePixmap(_pm_exploded, false, Exploded);
	_pm_exploded.setMask(mask);

	minePixmap(mask, true, Marked);
	minePixmap(_pm_error, false, Marked);
	_pm_error.setMask(mask);

    for (uint i=0; i<5; i++) {
        advisedPixmap(mask, true, i);
        advisedPixmap(_pm_advised[i], false, i);
        _pm_advised[i].setMask(mask);
    }

	QFont f = font();
	f.setPointSize(cp.size-6);
	f.setBold(true);
	setFont(f);

    updateGeometry();
    emit setMood(Smiley::Normal); // #### necessary to correctly resize the
                                  // widget !!!
}

void Field::advisedPixmap(QPixmap &pix, bool mask, uint i) const
{
    pix.resize(_cp.size, _cp.size);
    if (mask) pix.fill(color0);
    QPainter p(&pix);
    p.setWindow(0, 0, 16, 16);
    p.setPen( QPen((mask ? color1 : _cp.numberColors[i]), 2) );
    p.drawRect(3, 3, 11, 11);
}

void Field::flagPixmap(QPixmap &pix, bool mask) const
{
	pix.resize(_cp.size, _cp.size);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 16, 16);
	p.setPen( (mask ? color1 : black) );
	p.drawLine(6, 13, 14, 13);
	p.drawLine(8, 12, 12, 12);
	p.drawLine(9, 11, 11, 11);
	p.drawLine(10, 2, 10, 10);
	if (!mask) p.setPen(black);
	p.setBrush( (mask ? color1 : _cp.colors[FlagColor]) );
	p.drawRect(4, 3, 6, 5);
}

void Field::minePixmap(QPixmap &pix, bool mask, CaseState type) const
{
	pix.resize(_cp.size, _cp.size);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 20, 20);

	if ( type==Exploded )
		p.fillRect(2, 2, 16, 16, (mask ? color1 : _cp.colors[ExplosionColor]));

	QPen pen(mask ? color1 : black, 1);
	p.setPen(pen);
	p.setBrush(mask ? color1 : black);
	p.drawLine(10,3,10,18);
	p.drawLine(3,10,18,10);
	p.drawLine(5, 5, 16, 16);
	p.drawLine(5, 15, 15, 5);
	p.drawEllipse(5, 5, 11, 11);

	p.fillRect(8, 8, 2, 2, (mask ? color1 : white));

	if ( type==Marked ) {
		if (!mask) {
			pen.setColor(_cp.colors[ErrorColor]);
			p.setPen(pen);
		}
		p.drawLine(3, 3, 17, 17);
		p.drawLine(4, 3, 17, 16);
		p.drawLine(3, 4, 16, 17);
		p.drawLine(3, 17, 17, 3);
		p.drawLine(3, 16, 16, 3);
		p.drawLine(4, 17, 17, 4);
	}
}

QSize Field::sizeHint() const
{
	return QSize(2*frameWidth() + _level.width()*_cp.size,
				 2*frameWidth() + _level.height()*_cp.size);
}

QSizePolicy Field::sizePolicy() const
{
	return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void Field::setLevel(const Level &level)
{
    _level = level;
    updateGeometry();
    reset();
}

void Field::setState(GameState state)
{
    _state = state;
    emit gameStateChanged(state, false);
}

void Field::reset()
{
    BaseField::reset(_level.width(), _level.height(), _level.nbMines());
    setState(Stopped);
    if (_completeReveal) emit setCheating();
	_currentAction = None;
    _reveal = false;
    _cursor.first = _level.width()/2;
    _cursor.second = _level.height()/2;
    _advised = Coord(-1, -1);
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
    qp.setX( p.first*_cp.size + frameWidth() );
    qp.setY( p.second*_cp.size + frameWidth() );
    return qp;
}

Coord Field::fromPoint(const QPoint &qp) const
{
	double i = (double)(qp.x() - frameWidth()) / _cp.size;
    double j = (double)(qp.y() - frameWidth()) / _cp.size;
    return Coord((int)floor(i), (int)floor(j));
}

MouseAction Field::mapMouseButton(QMouseEvent *e) const
{
	switch (e->button()) {
	case LeftButton:  return _mb[KMines::LeftButton];
	case MidButton:   return _mb[KMines::MidButton];
	case RightButton: return _mb[KMines::RightButton];
	default:          return Mark;
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
	if ( !isActive() || _currentAction!=None ) return;

    emit setMood(Smiley::Stressed);
	_currentAction = mapMouseButton(e);

    Coord p = fromPoint(e->pos());
    if ( !inside(p) ) return;
    placeCursor(p);
	revealActions(true);
}

void Field::mouseReleaseEvent(QMouseEvent *e)
{
	if ( !isActive() ) return;

    MouseAction tmp = _currentAction;
    emit setMood(Smiley::Normal);
	revealActions(false);
    MouseAction ma = mapMouseButton(e);
    _currentAction = None;
    if ( ma!=tmp ) return;

    Coord p = fromPoint(e->pos());
    if ( !inside(p) ) return;
    placeCursor(p);

	switch (ma) {
    case Mark:       doMark(p); break;
    case UMark:      doUmark(p); break;
	case Reveal:     doReveal(p); break;
	case AutoReveal: doAutoReveal(p); break;
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
        drawBox(painter, c, pressed);
    }
}

void Field::pressClearFunction(const Coord &p, bool pressed)
{
    pressCase(p, pressed);
    CoordSet n;
    neighbours(p, n);
    for (CoordSet::iterator it=n.begin(); it!=n.end(); ++it)
        pressCase(*it, pressed);
}

void Field::keyboardAutoReveal()
{
	pressClearFunction(_cursor, true);
	QTimer::singleShot(50, this, SLOT(keyboardAutoRevealSlot()));
}

void Field::keyboardAutoRevealSlot()
{
	pressClearFunction(_cursor, false);
	doAutoReveal(_cursor);
}

void Field::doAutoReveal(const Coord &p)
{
	if ( !isActive() ) return;

    if ( state(p)==Uncovered ) emit incActions();
    resetAdvised();
    bool ok = autoReveal(p, 0);
    if ( !ok ) setState(GameOver);
    else if (_completeReveal) completeReveal();
}

void Field::pause()
{
    switch (_state) {
    case Paused:  setState(Playing); break;
    case Playing: setState(Paused);  break;
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

void Field::doReveal(const Coord &p)
{
	if ( !isActive() ) return;
    emit incActions();
    resetAdvised();

    if ( firstReveal() ) setState(Playing);
    bool ok = reveal(p, 0, 0);
    if (!ok) setState(GameOver);
    else if (_completeReveal) completeReveal();
}

void Field::doMark(const Coord &p)
{
	if ( !isActive() ) return;
    emit incActions();
    resetAdvised();
    mark(p);
}

void Field::doUmark(const Coord &p)
{
	if ( !isActive() ) return;
    emit incActions();
    resetAdvised();
    umark(p);
}

void Field::placeCursor(const Coord &p)
{
	if ( !isActive() ) return;

    Q_ASSERT( inside(p) );
    Coord old = _cursor;
    _cursor = p;
	if ( _cursorShown ) {
        QPainter painter(this);
		drawCase(painter, old);
		drawCase(painter, _cursor);
	}
}

void Field::resetAdvised()
{
    if ( !inside(_advised) ) return;
    QPainter p(this);
    Coord tmp = _advised;
    _advised = Coord(-1, -1);
    drawCase(p, tmp);
}

void Field::setAdvised(const Coord &c, double proba)
{
    resetAdvised();

    _advised = c;
    _advisedProba = proba;
    if ( inside(_advised) ) {
        QPainter p(this);
        drawCase(p, c);
    }
}

// draw methods
void Field::drawItem(QPainter &painter, const QPoint &p,
                     const QPixmap *pixmap, const QString &text,
                     const QColor &textColor) const
{
    QRect r(p, _button.size());
    style().drawItem(&painter, r, AlignCenter, colorGroup(), true, pixmap,
                    text, -1, &textColor);
}

void Field::drawBox(QPainter &painter, const Coord &c, bool pressed,
                    const QPixmap *pixmap, const QString &text,
                    const QColor &textColor) const
{
    QPoint p = toPoint(c);
	painter.translate(p.x(), p.y());

    QStyle::SFlags flags = QStyle::Style_Enabled;
    bool hasFocus = ( _cursorShown && c==_cursor );
    if (hasFocus) flags |= QStyle::Style_HasFocus;
     if (pressed) {
        flags |= QStyle::Style_Sunken;
        flags |= QStyle::Style_Down;
    } else {
        flags |= QStyle::Style_Raised;
        flags |= QStyle::Style_Up;
    }
    style().drawPrimitive(QStyle::PE_ButtonCommand, &painter, _button.rect(),
                          colorGroup(), flags);
    if (hasFocus) {
        QRect fbr = style().subRect(QStyle::SR_PushButtonFocusRect, &_button);
        style().drawPrimitive(QStyle::PE_FocusRect, &painter, fbr,
                              colorGroup(), flags);
    }

	painter.resetXForm();
    drawItem(painter, p, pixmap, text, textColor);
}

void Field::drawCase(QPainter &painter, const Coord &c) const
{
	Q_ASSERT( inside(c) );

	switch ( state(c) ) {
    case Covered:
        drawBox(painter, c, false);
        break;
	case Marked:
        drawBox(painter, c, false, &_pm_flag);
        break;
	case Error:
        drawBox(painter, c, true, &_pm_error);
        break;
	case Uncertain:
        drawBox(painter, c, false, 0, "?", black);
        break;
	case Exploded:
        drawBox(painter, c, true, &_pm_exploded);
        break;
	case Uncovered:
        if ( hasMine(c) ) drawBox(painter, c, true, &_pm_mine);
        else {
            uint n = nbMinesAround(c);
            QString nb;
            if (n) nb.setNum(n);
                drawBox(painter, c, true, 0, nb,
                        n ? _cp.numberColors[n-1] : black);
        }
	}

    if ( c==_advised ) {
        uint i = 4;
        if ( _advisedProba==1 ) i = 0;
        else if ( _advisedProba>0.75 ) i = 1;
        else if ( _advisedProba>0.5  ) i = 2;
        else if ( _advisedProba>0.25 ) i = 3;
        drawItem(painter, toPoint(c), &_pm_advised[i]);
    }
}
