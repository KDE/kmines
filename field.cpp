#include "field.h"
#include "field.moc"

#include <math.h>

#include <qbitmap.h>
#include <qlayout.h>
#include <qstyle.h>
#include <qtimer.h>

#include <klocale.h>


//-----------------------------------------------------------------------------
Field::Field(QWidget *parent)
: QFrame(parent, "field"), _level(Level::Easy), random(0), state(Stopped),
  u_mark(false), cursor(false), button(0)
{
	setFrameStyle( QFrame::Box | QFrame::Raised );
	setLineWidth(2);
	setMidLineWidth(2);

	readSettings();
}

void Field::readSettings()
{
	setCaseProperties( AppearanceSettings::readCaseProperties() );
	setUMark( GameSettings::readUMark() );
	setCursor( GameSettings::readKeyboard() );
	for (uint i=0; i<3; i++)
		mb[i] = GameSettings::readMouseBinding((MouseButton)i);
}

void Field::setCaseProperties(const CaseProperties &_cp)
{
	cp = _cp;
	button.resize(cp.size, cp.size);
	updateGeometry();

	QBitmap mask;

	flagPixmap(mask, true);
	flagPixmap(pm_flag, false);
	pm_flag.setMask(mask);

	minePixmap(mask, true, Covered);
	minePixmap(pm_mine, false, Covered);
	pm_mine.setMask(mask);

	minePixmap(mask, true, Exploded);
	minePixmap(pm_exploded, false, Exploded);
	pm_exploded.setMask(mask);

	minePixmap(mask, true, Marked);
	minePixmap(pm_error, false, Marked);
	pm_error.setMask(mask);

	QFont f = font();
	f.setPointSize(cp.size-6);
	f.setBold(true);
	setFont(f);
}

void Field::flagPixmap(QPixmap &pix, bool mask) const
{
	pix.resize(cp.size, cp.size);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 16, 16);
	p.setPen( (mask ? color1 : black) );
	p.drawLine(6, 13, 14, 13);
	p.drawLine(8, 12, 12, 12);
	p.drawLine(9, 11, 11, 11);
	p.drawLine(10, 2, 10, 10);
	if (!mask) p.setPen(cp.colors[FlagColor]);
	p.setBrush( (mask ? color1 : cp.colors[FlagColor]) );
	p.drawRect(4, 3, 6, 5);
}

void Field::minePixmap(QPixmap &pix, bool mask, CaseState type) const
{
	pix.resize(cp.size, cp.size);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 20, 20);

	if ( type==Exploded )
		p.fillRect(2, 2, 16, 16, (mask ? color1 : cp.colors[ExplosionColor]));

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
			pen.setColor(cp.colors[ErrorColor]);
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
	return QSize(2*frameWidth() + _level.width()*cp.size,
				 2*frameWidth() + _level.height()*cp.size);
}

QSizePolicy Field::sizePolicy() const
{
	return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

const KMines::Case &Field::pfield(uint i, uint j) const
{
	return _pfield[i + j*(_level.width()+2)];
}

KMines::Case &Field::pfield(uint i, uint j)
{
	return _pfield[i + j*(_level.width()+2)];
}

uint Field::computeNeighbours(uint i, uint j) const
{
	uint nm = 0;

	if (pfield(i-1,   j).mine) nm++;
	if (pfield(i-1, j+1).mine) nm++;
	if (pfield(i-1, j-1).mine) nm++;
	if (pfield(  i, j+1).mine) nm++;
	if (pfield(  i, j-1).mine) nm++;
	if (pfield(i+1,   j).mine) nm++;
	if (pfield(i+1, j+1).mine) nm++;
	if (pfield(i+1, j-1).mine) nm++;

	return nm;
}

void Field::setLevel(const Level &level)
{
	_level = level;
	restart();
	updateGeometry();
}

void Field::restart()
{
	state = Playing;
	firstReveal = true;
	currentAction = None;
	ic = _level.width()/2;
	jc = _level.height()/2;

	Case tmp;
	tmp.mine = false;
	tmp.state = Covered;
	_pfield.fill(tmp, (_level.width()+2) * (_level.height()+2) );

	// fill hidden borders
	tmp.state = Uncovered;
	for (uint i=0; i<_level.width()+2; i++) {
		pfield(i, 0) = tmp;
		pfield(i, _level.height()+1) = tmp;
	}
	for (uint j=0; j<_level.height()+2; j++) {
		pfield(0, j) = tmp;
		pfield(_level.width()+1, j) = tmp;
	}

	update();
}

void Field::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	drawFrame(&p);

	if ( state==Paused ) return;

	uint imin
        = (uint)QMAX(QMIN(xToI(e->rect().left()), (int)_level.width()), 1);
	uint imax
        = (uint)QMAX(QMIN(xToI(e->rect().right()), (int)_level.width()), 1);
	uint jmin
        = (uint)QMAX(QMIN(yToJ(e->rect().top()), (int)_level.height()), 1);
	uint jmax
        = (uint)QMAX(QMIN(yToJ(e->rect().bottom()), (int)_level.height()), 1);
	for (uint i=imin; i<=imax; i++)
	    for (uint j=jmin; j<=jmax; j++)
            drawCase(p, i, j);
}

void Field::changeCaseState(uint i, uint j, CaseState new_st)
{
	emit changeCase(pfield(i, j).state, -1);
	pfield(i, j).state = new_st;
	emit changeCase(new_st, 1);
    QPainter p(this);
	drawCase(p, i, j);
	if ( state==Playing ) emit updateStatus(pfield(i, j).mine);
}

int Field::iToX(uint i) const
{
	return (i-1)*cp.size + frameWidth();
}

int Field::jToY(uint j) const
{
	return (j-1)*cp.size + frameWidth();
}

int Field::xToI(int x) const
{
	double d = (double)(x - frameWidth()) / cp.size;
	return (int)floor(d) + 1;
}

int Field::yToJ(int y) const
{
	double d = (double)(y - frameWidth()) / cp.size;
	return (int)floor(d) + 1;
}

void Field::uncover(uint i, uint j)
{
	if ( pfield(i, j).state!=Covered ) return;
	uint nbs = computeNeighbours(i, j);

	if (!nbs) {
		changeCaseState(i, j, Uncovered);
		uncover(i-1, j+1);
		uncover(i-1,   j);
		uncover(i-1, j-1);
		uncover(  i, j+1);
		uncover(  i, j-1);
		uncover(i+1, j+1);
		uncover(i+1,   j);
		uncover(i+1, j-1);
	} else changeCaseState(i, j, Uncovered);
}

bool Field::inside(int i, int j) const
{
	return ( i>=1 && i<=(int)_level.width() && j>=1
             && j<=(int)_level.height());
}

KMines::MouseAction Field::mapMouseButton(QMouseEvent *e) const
{
	switch (e->button()) {
	case LeftButton:  return mb[KMines::Left];
	case MidButton:   return mb[KMines::Mid];
	case RightButton: return mb[KMines::Right];
	default:          return Mark;
	}
}

bool Field::revealActions(bool press)
{
	switch (currentAction) {
	case Reveal:
		pressCase(ic, jc, press);
		return true;
	case AutoReveal:
		pressClearFunction(ic, jc, press);
		return true;
	default:
		return false;
	}
}

void Field::mousePressEvent(QMouseEvent *e)
{
	if ( state!=Playing || currentAction!=None ) return;

	setMood(Smiley::Stressed);
	currentAction = mapMouseButton(e);
	if ( !placeCursor(xToI(e->pos().x()), yToJ(e->pos().y())) ) return;

	if ( !revealActions(true) )
		switch (currentAction) {
		case Mark:
			mark();
			break;
		case UMark:
			umark();
			break;
		default:
			break;
		}
}

void Field::mouseReleaseEvent(QMouseEvent *e)
{
	if ( state!=Playing ) return;

	MouseAction ma = mapMouseButton(e);
	if ( ma!=currentAction ) return;
	setMood(Smiley::Normal);
	revealActions(false);
	currentAction = None;

	if ( !placeCursor(xToI(e->pos().x()), yToJ(e->pos().y())) ) return;

	switch (ma) {
	case Reveal:
		reveal();
		break;
	case AutoReveal:
		autoReveal();
		break;
	default:
		break;
	}
}

void Field::mouseMoveEvent(QMouseEvent *e)
{
	if ( state!=Playing ) return;

	int i = xToI(e->pos().x());
	int j = yToJ(e->pos().y());
	if ( i==(int)ic && j==(int)jc ) return; // avoid flicker

	revealActions(false);
	if ( !placeCursor(i, j) ) return;
	revealActions(true);
}

void Field::showMines()
{
	for(uint i=1; i<=_level.width(); i++)
		for(uint j=1; j<=_level.height(); j++) {
		    if ( !pfield(i, j).mine ) continue;
			if ( pfield(i, j).state!=Exploded && pfield(i, j).state!=Marked )
				changeCaseState(i, j, Uncovered);
		}
}

void Field::pressCase(uint i, uint j, bool pressed)
{
	if ( pfield(i, j).state==Covered ) {
        QPainter p(this);
        drawBox(p, i, j, pressed);
    }
}

void Field::pressClearFunction(uint i, uint j, bool pressed)
{
	pressCase(i-1, j+1, pressed);
	pressCase(i-1,   j, pressed);
	pressCase(i-1, j-1, pressed);
	pressCase(  i, j+1, pressed);
	pressCase(  i,   j, pressed);
	pressCase(  i, j-1, pressed);
	pressCase(i+1, j-1, pressed);
	pressCase(i+1,   j, pressed);
	pressCase(i+1, j+1, pressed);
}

void Field::keyboardAutoReveal()
{
	pressClearFunction(ic, jc, true);
	QTimer::singleShot(50, this, SLOT(keyboardAutoRevealSlot()));
}

void Field::keyboardAutoRevealSlot()
{
	pressClearFunction(ic, jc, false);
	autoReveal();
}

void Field::autoReveal()
{
	if ( state!=Playing ) return;
    if ( pfield(ic, jc).state!=Uncovered ) return;
    emit incActions();

	// number of mines around the case
	uint nm = computeNeighbours(ic, jc);
	if ( pfield(ic-1,   jc).state==Marked ) nm--;
	if ( pfield(ic-1, jc+1).state==Marked ) nm--;
	if ( pfield(ic-1, jc-1).state==Marked ) nm--;
	if ( pfield(  ic, jc+1).state==Marked ) nm--;
	if ( pfield(  ic, jc-1).state==Marked ) nm--;
	if ( pfield(ic+1,   jc).state==Marked ) nm--;
	if ( pfield(ic+1, jc+1).state==Marked ) nm--;
	if ( pfield(ic+1, jc-1).state==Marked ) nm--;

	if (!nm) { // number of surrounding mines == number of marks :)
		uncoverCase(ic+1, jc+1);
		uncoverCase(ic+1,   jc);
		uncoverCase(ic+1, jc-1);
		uncoverCase(  ic, jc+1);
		uncoverCase(  ic, jc-1);
		uncoverCase(ic-1, jc+1);
		uncoverCase(ic-1,   jc);
		uncoverCase(ic-1, jc-1);
	}
}

void Field::uncoverCase(uint i, uint j)
{
	if ( pfield(i, j).state!=Covered ) return;

    if ( !pfield(i, j).mine ) {
        uncover(i, j);
        return;
    }

    // explosion
    changeCaseState(i, j, Exploded);
    // if this is not the first explosion, the game is already stopped
    if ( state==Stopped ) return;

    // find all errors
    for (uint ii=1; ii<_level.width()+1; ii++)
        for (uint jj=1; jj<_level.height()+1; jj++)
            if ( pfield(ii, jj).state==Marked && !pfield(ii, jj).mine )
                changeCaseState(ii, jj, Error);

    emit gameLost();
}

void Field::pause()
{
	if ( firstReveal || state==Stopped ) return;

	if ( state==Paused ) resume(); // if already paused : resume game
	else {
		emit stopTimer();
		state = Paused;
        emit setMood(Smiley::Sleeping);
		emit gameStateChanged(Paused);
		update();
	}
}

void Field::resume()
{
	state = Playing;
    emit setMood(Smiley::Normal);
	emit gameStateChanged(Playing);
	emit startTimer();
	update();
}

void Field::up()
{
	placeCursor(ic, jc-1);
}

void Field::down()
{
	placeCursor(ic, jc+1);
}

void Field::left()
{
	placeCursor(ic-1, jc);
}

void Field::right()
{
	placeCursor(ic+1, jc);
}

void Field::reveal()
{
	if ( state!=Playing ) return;
    emit incActions();
	if (firstReveal) {
		// set mines positions on field ; must avoid the first
		// clicked case
		for(uint k=0; k<_level.nbMines(); k++) {
			uint i, j;
			do {
				i = random.getLong(_level.width());
				j = random.getLong(_level.height());
			} while ( pfield(i+1, j+1).mine
					  || ((i+1)==(uint)ic && (j+1)==(uint)jc) );

			pfield(i+1, j+1).mine = true;
		}
		emit startTimer();
		firstReveal = false;
		emit gameStateChanged(Playing);
	}

	uncoverCase(ic, jc);
}

void Field::mark()
{
	if ( state!=Playing ) return;
    emit incActions();
	switch (pfield(ic, jc).state) {
	case Covered:   changeCaseState(ic, jc, Marked); break;
	case Marked:    changeCaseState(ic, jc, (u_mark ? Uncertain : Covered));
		        break;
	case Uncertain:	changeCaseState(ic, jc, Covered); break;
	default:        break;
	}
}

void Field::umark()
{
	if ( state!=Playing ) return;
    emit incActions();
	switch (pfield(ic, jc).state) {
	case Covered:
	case Marked:    changeCaseState(ic, jc, Uncertain); break;
	case Uncertain: changeCaseState(ic, jc, Covered); break;
	default:        break;
	}
}

bool Field::placeCursor(int i, int j)
{
	if ( state!=Playing || !inside(i, j) ) return false;
	uint oldIc = ic;
	uint oldJc = jc;
	ic = (uint)i;
	jc = (uint)j;
	if (cursor) {
        QPainter p(this);
		drawCase(p, oldIc, oldJc);
		drawCase(p, ic, jc);
	}
	return true;
}

void Field::setCursor(bool show)
{
	cursor = show;
	if ( state==Playing ) {
        QPainter p(this);
        drawCase(p, ic, jc);
    }
}

// draw methods
void Field::drawBox(QPainter &p, uint i, uint j, bool pressed,
                    const QPixmap *pixmap, const QString &text,
                    const QColor &textColor)
{
	int x = iToX(i);
	int y = jToY(j);

	p.translate(x, y);

	QStyle::SFlags flags = QStyle::Style_Enabled;
	if (pressed) flags |= QStyle::Style_Down;
	else flags |= QStyle::Style_Raised;
	if ( cursor && i==ic && j==jc ) flags |= QStyle::Style_HasFocus;

	style().drawControl(QStyle::CE_PushButton, &p, &button, button.rect(),
                            colorGroup(), flags);
	style().drawControl(QStyle::CE_PushButtonLabel, &p, &button,
                            style().subRect(QStyle::SR_PushButtonFocusRect, &button),
                            colorGroup(), flags);

	// we need to draw directly because the pushbutton control clips too much
	// text and pixmap ...
	p.resetXForm();
	QRect r(x, y, cp.size, cp.size);
	style().drawItem(&p, r, AlignCenter, colorGroup(), true, pixmap,
                         text, -1, &textColor);
}

void Field::drawCase(QPainter &p, uint i, uint j)
{
	Q_ASSERT( inside(i, j) );
	switch (pfield(i, j).state) {
		case Covered:
			drawBox(p, i, j, false);
			break;
	case Marked:
			drawBox(p, i, j, false, &pm_flag);
			break;
	case Error:
			drawBox(p, i, j, true, &pm_error);
			break;
	case Uncertain:
			drawBox(p, i, j, false, 0, "?", black);
			break;
	case Exploded:
			drawBox(p, i, j, true, &pm_exploded);
			break;
	case Uncovered:
        	if ( pfield(i, j).mine ) drawBox(p, i, j, true, &pm_mine);
        	else {
                uint n = computeNeighbours(i, j);
                QString nb;
                if (n) nb.setNum(n);
                drawBox(p, i, j, true, 0, nb,
                        n ? cp.numberColors[n-1] : black);
        	}
	}
}
