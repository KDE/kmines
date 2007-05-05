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

#include <QLayout>
#include <QTimer>
#include <QPainter>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QStyleOptionFocusRect>

#include <klocale.h>
#include <knotification.h>
#include <kstandarddirs.h>

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
    : QWidget(parent), _state(Init), _solvingState(Regular), _level(Level::Easy)
{
    borderSize = 0; //Settings::caseSize();
    theme.loadDefault();
}

void Field::readSettings()
{
    if (!theme.load(Settings::theme())) {
        theme.loadDefault();
    }
    svg.load(theme.graphics());
    updatePixmaps();
    if ( inside(_cursor) ) {
        update( toRect(_cursor) );
    }
    if ( Settings::magicReveal() ) setCheating(true);
}

QSize Field::minimumSizeHint() const
{
  return QSize( (_level.width()+2)*20,
               (_level.height()+2)*20);
}

QSize Field::sizeHint() const
{
  return QSize( (_level.width()+0)*Settings::caseSize(),
               (_level.height()+0)*Settings::caseSize());
}

void Field::resizeEvent(QResizeEvent*)
{
    adjustCaseSize(QWidget::size());
    qDebug() << "field resize event";
}
void Field::adjustCaseSize(const QSize & boardsize) 
{
    //calculate our best case size to fit the boardsize passed to us
    qreal aspectratio;
    qreal newcase;

    //actually the boardsize contains the smiley/clock/status bar as well at this point
    // :(
    //take this into account for now, this will change when these elements are migrated a proper status bar 
    //or to in-game elements, according to the theme
    qreal bw = boardsize.width();
    qreal bh = boardsize.height();

    //use fixed size for calculation, adding borders. 
    qreal fullh = (16.0 * (_level.height()+0.0));
    qreal fullw = (16.0 * (_level.width()+0.0));

    if ((fullw/fullh)>(bw/bh)) {
        //space will be left on height, use width as limit
	aspectratio = bw/fullw;
    } else {
	aspectratio = bh/fullh;
    }
    newcase =  aspectratio * 16.0;

qDebug() << "Preferred case size is"<< newcase;
    
    Settings::setCaseSize((int) newcase);
    borderSize = 0;
    updatePixmaps();
    update();
}

void Field::setLevel(const Level &level)
{
    _level = level;
    reset(false);
    updatePixmaps();
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
    if (Settings::magicReveal()) setCheating(true);
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
    //TODO: draw border?
    //drawFrame(&painter);
    if ( _state==Paused ) return;

    Coord min = fromPoint(e->rect().topLeft());
    bound(min);
    Coord max = fromPoint(e->rect().bottomRight());
    bound(max);
    for (short i=min.first; i<=max.first; i++)
        for (short j=min.second; j<=max.second; j++)
            drawCase(painter, Coord(i,j), _pressedCoords.contains(Coord(i,j)));
    _pressedCoords.clear();
}

void Field::changeCase(const Coord &p, CaseState newState)
{
    BaseField::changeCase(p, newState);
    update( toRect(p) );
    if ( isActive() ) emit updateStatus( hasMine(p) );
}

QPoint Field::toPoint(const Coord &p) const
{
    // allow for the gap between the left side of the widget
    // and the left-most case  
    int gap = (rect().width() - sizeHint().width()) / 2;

    QPoint qp;
    qp.setX( p.first*Settings::caseSize() + borderSize + gap );
    qp.setY( p.second*Settings::caseSize() + borderSize );
    return qp;
}

Coord Field::fromPoint(const QPoint &qp) const
{
    // allow for the gap between the left side of the widget
    // and the left-most case
    int gap = (rect().width() - sizeHint().width()) / 2;
    
    double i = (double)(qp.x() - borderSize - gap) / Settings::caseSize();
    double j = (double)(qp.y() - borderSize) / Settings::caseSize();
    return Coord((int)floor(i), (int)floor(j));
}

QRect Field::toRect(const Coord &p) const
{
    return QRect(toPoint(p), QSize( Settings::caseSize(), Settings::caseSize() ));
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
            if(pressed)
                _pressedCoords.append(c);
            update( toRect(c) );
    }
}

void Field::pressClearFunction(const Coord &p, bool pressed)
{
    pressCase(p, pressed);
    CoordList n = coveredNeighbours(p);
    if(pressed)
        _pressedCoords = n;
    QRect rect;
    for (CoordList::const_iterator it=n.begin(); it!=n.end(); ++it)
        rect = rect.unite(toRect(*it));
    update(rect);
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

    KNotification::event(ACTION_DATA[type].event,
                         i18n(ACTION_DATA[type].eventMessage), QPixmap() , this);
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
        update(toRect(old));
        update(toRect(_cursor));
    }
}

void Field::resetAdvised()
{
    if ( !inside(_advisedCoord) ) return;
    Coord tmp = _advisedCoord;
    _advisedCoord = Coord(-1, -1);
    update( toRect(tmp) );
}

void Field::setAdvised(const Coord &c, double proba)
{
    resetAdvised();
    _solvingState = Advised;
    _advisedCoord = c;
    _advisedProba = proba;
    if ( inside(c) ) {
        update( toRect(c) );
    }
}

void Field::drawCase(QPainter &painter, const Coord &c, bool pressed) 
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
        type = QuestionPixmap;
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
            if (nbMines) {
              switch (nbMines) {
                case 1:
                  type = Num1Pixmap;
                  break;
                case 2:
                  type = Num2Pixmap;
                  break;
                case 3:
                  type = Num3Pixmap;
                  break;
                case 4:
                  type = Num4Pixmap;
                  break;
                case 5:
                  type = Num5Pixmap;
                  break;
                case 6:
                  type = Num6Pixmap;
                  break;
                case 7:
                  type = Num7Pixmap;
                  break;
                case 8:
                  type = Num8Pixmap;
                  break;
              }
            }
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

void Field::updatePixmaps()
{
    QPixmap mask;
    for (uint i=0; i<Nb_Pixmap_Types; i++) {
        drawPixmap(_pixmaps[i], (PixmapType)i, true);
    }
    for (uint i=0; i<Nb_Advised; i++) {
        drawAdvised(_advised[i], i, true);;
    }

    QFont f = font();
    f.setPointSize(qMax(1, Settings::caseSize()-6));
    f.setBold(true);
    setFont(f);
}

void Field::initPixmap(QPixmap &pix, bool mask) 
{
    pix = QPixmap( Settings::caseSize(), Settings::caseSize() );
    if (mask) pix.fill(Qt::color0);
}

void Field::drawPixmap(QPixmap &pix, PixmapType type, bool mask) 
{
    initPixmap(pix, mask);

    QImage qiRend(QSize(pix.width(), pix.height()),QImage::Format_ARGB32_Premultiplied);
    qiRend.fill(0);
    QPainter p(&qiRend);

    if ( type==FlagPixmap ) {
        svg.render(&p, "flag");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==QuestionPixmap ) {
        svg.render(&p, "question");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==Num1Pixmap ) {
        svg.render(&p, "arabicOne");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==Num2Pixmap ) {
        svg.render(&p, "arabicTwo");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==Num3Pixmap ) {
        svg.render(&p, "arabicThree");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==Num4Pixmap ) {
        svg.render(&p, "arabicFour");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==Num5Pixmap ) {
        svg.render(&p, "arabicFive");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==Num6Pixmap ) {
        svg.render(&p, "arabicSix");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==Num7Pixmap ) {
        svg.render(&p, "arabicSeven");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    if ( type==Num8Pixmap ) {
        svg.render(&p, "arabicEight");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    //If exploding...;
    if ( type==ExplodedPixmap ) {
        svg.render(&p, "explosion");;
    }

    //Now render mine graphic
    svg.render(&p, "mine");

    //Finally render the error marker
    if ( type==ErrorPixmap ) {
      svg.render(&p, "error");
    }
    pix = QPixmap::fromImage(qiRend);
}

void Field::drawAdvised(QPixmap &pix, uint i, bool mask) 
{
    initPixmap(pix, mask);
    QImage qiRend(QSize(pix.width(), pix.height()),QImage::Format_ARGB32_Premultiplied);
    qiRend.fill(0);
    QPainter p(&qiRend);
    /*QPainter p(&pix);
    p.setWindow(0, 0, 16, 16);
    p.setPen( QPen(mask ? Qt::color1 : Settings::mineColor(i), 2) );
    p.drawRect(3, 3, 11, 11);*/
    svg.render(&p, "hint");
    pix = QPixmap::fromImage(qiRend);
}

void Field::drawBox(QPainter &painter, const QPoint &p,
                      bool pressed, PixmapType type, const QString &text,
                      uint nbMines, int advised,
                      bool hasFocus) 
{
    //Use SVG theme instead
    /*qDrawShadePanel(&painter, p.x(), p.y(), _button.width(), _button.height(),
                    palette(),  pressed, 2,
                    &palette().brush(QPalette::Background));

    if (hasFocus) {
        painter.translate(p.x(), p.y());
        QStyleOptionFocusRect option;
        option.init(this);
        option.fontMetrics = painter.fontMetrics();
        option.state = QStyle::State_Enabled;
        QRect fbr = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &option, &_button);
        option.rect = fbr;
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter);
        painter.resetMatrix();
    }*/

    QRect r(p, QSize(Settings::caseSize(),Settings::caseSize()));
    if (pressed) {
        svg.render(&painter, "cell_down", r);
    } else {
        svg.render(&painter, "cell_up", r);
    }
    const QPixmap *pixmap = (type==NoPixmap ? 0 : &_pixmaps[type]);
    /*QColor color = (nbMines==0 ? Qt::black : Settings::mineColor(nbMines-1));
    QPalette pal;
    pal.setColor( QPalette::WindowText, color );
    style()->drawItemText(&painter, r, Qt::AlignCenter, pal, true, text, QPalette::WindowText);*/
    if (pixmap)
      style()->drawItemPixmap(&painter, r, Qt::AlignCenter, *pixmap);
    if ( advised!=-1 )
        style()->drawItemPixmap(&painter, r, Qt::AlignCenter, _advised[advised]);
}
