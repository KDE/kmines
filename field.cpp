#include "field.h"

#include <qdrawutil.h>
#include <qlayout.h>
#include <qbitmap.h>

#include <klocale.h>

Field::Field(QWidget *parent, const char *name)
: QFrame(parent, name), lev(LEVELS[0]), random(0), state(Stopped),
  u_mark(false), cursor(false), _reveal(false), _autoreveal(false)
{
	setFrameStyle( QFrame::Box | QFrame::Raised );
	setLineWidth(2);
	setMidLineWidth(2);

	QVBoxLayout *top = new QVBoxLayout(this, 0);
	top->addStretch(1);
	pb = new QPushButton(i18n("Press to resume"), this);
	pb->hide();
	top->addWidget(pb, 0, AlignCenter);
	connect(pb, SIGNAL(clicked()), this, SLOT(resume()));
	top->addStretch(1);

	QFont f = font();
	f.setBold(true);
	setFont(f);
	
	readSettings();
}

void Field::readSettings()
{
	setCaseProperties(OptionDialog::readCaseProperties());
	setUMark(OptionDialog::readUMark());
	setCursor(OptionDialog::readKeyboard());
	for (uint i=0; i<3; i++)
		mb[i] = OptionDialog::readMouseBinding((MouseButton)i);
}

void Field::setCaseProperties(const CaseProperties &_cp)
{
	cp = _cp;
	
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

	cursorPixmap(mask, true);
	cursorPixmap(pm_cursor, false);
	pm_cursor.setMask(mask);

	QFont f = font();
	f.setPointSize(cp.size-6);
	setFont(f);

	updateGeometry();
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
	if (!mask) p.setPen(cp.flagColor);
	p.setBrush( (mask ? color1 : cp.flagColor) );
	p.drawRect(4, 3, 6, 5);
}

void Field::cursorPixmap(QPixmap &pix, bool mask) const
{
	pix.resize(cp.size, cp.size);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 20, 20);
	p.setPen( (mask ? color1 : black) );
	p.drawRect(2, 2, 16, 16);
}

void Field::minePixmap(QPixmap &pix, bool mask, CaseState type) const
{
	pix.resize(cp.size, cp.size);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 20, 20);

	if ( type==Exploded )
		p.fillRect(2, 2, 16, 16, (mask ? color1 : cp.explosionColor));

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
			pen.setColor(cp.errorColor);
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
	return QSize(2*frameWidth() + lev.width*cp.size,
				 2*frameWidth() + lev.height*cp.size);
}

QSizePolicy Field::sizePolicy() const
{
	return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

const Case &Field::pfield(uint i, uint j) const
{
	return _pfield[i + j*(lev.width+2)];
}

Case &Field::pfield(uint i, uint j)
{
	return _pfield[i + j*(lev.width+2)];
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

void Field::setLevel(const Level &l)
{
	lev = l;
	restart(false);
	updateGeometry();
}

void Field::restart(bool repaint)
{
	/* if game is paused : resume before restart */
	if ( state==Paused ) {
		resume();
		emit freezeTimer();
	}

	state = Playing;
	first_click = true;

	_pfield.resize( (lev.width+2) * (lev.height+2) );
	
	QPainter *p;
	if (repaint) p = new QPainter(this);
	for (uint i=0; i<lev.width+2; i++)
		for (uint j=0; j<lev.height+2; j++) {
			Case tmp;
			tmp.mine  = false;
			tmp.state = (i==0 || i==lev.width+1 || j==0 || j==lev.height+1
						 ? Uncovered : Covered);
			if ( pfield(i, j).mine==false && pfield(i, j).state==tmp.state )
				continue;
			pfield(i, j) = tmp;
			if (repaint && tmp.state==Covered) drawCase(i, j, p);
		}

	if (repaint) drawCursor(false, p);
	ic = lev.width/2;
	jc = lev.height/2;
	if ( repaint && cursor ) drawCursor(true, p);
}

void Field::paintEvent(QPaintEvent *e)
{
	if ( state==Paused ) return;		

	QPainter p(this);
	drawFrame(&p);
	uint imin = (uint)QMAX(QMIN(xToI(e->rect().left()), (int)lev.width), 1);
	uint imax = (uint)QMAX(QMIN(xToI(e->rect().right()), (int)lev.width), 1);
	uint jmin = (uint)QMAX(QMIN(yToJ(e->rect().top()), (int)lev.height), 1);
	uint jmax = (uint)QMAX(QMIN(yToJ(e->rect().bottom()), (int)lev.height), 1);
	for (uint i=imin; i<=imax; i++)
	    for (uint j=jmin; j<=jmax; j++) drawCase(i, j, &p);
}

void Field::changeCaseState(uint i, uint j, CaseState new_st)
{
	emit changeCase(pfield(i, j).state, -1);
	pfield(i, j).state = new_st;
	emit changeCase(new_st, 1);
	drawCase(i, j);
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
	// the cast is necessary when x-frameWidth() is negative ( ?? )
	return (int)((double)(x - frameWidth())/cp.size) + 1;
}

int Field::yToJ(int y) const
{
	return (int)((double)(y - frameWidth())/cp.size) + 1;
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
	return ( i>=1 && i<=(int)lev.width && j>=1 && j<=(int)lev.height);
}

MouseAction Field::mapMouseButton(QMouseEvent *e) const
{
	switch (e->button()) {
	case LeftButton:  return mb[Left];
	case MidButton:   return mb[Mid];
	case RightButton: return mb[Right];
	default:          return Mark;
	}
}

void Field::mousePressEvent(QMouseEvent *e)
{
	if ( state!=Playing ) return;
	setMood(Smiley::Stressed);
	bool inside = placeCursor(xToI(e->pos().x()), yToJ(e->pos().y()));

	switch ( mapMouseButton(e) ) {
	case Reveal:
		_reveal = true;
		if (inside) pressCase(ic, jc, false);
		break;
	case Mark:
		if (inside) mark();
		break;
	case UMark:
		if (inside) umark();
		break;
	case AutoReveal:
		_autoreveal = true;
		if (inside) pressClearFunction(ic, jc, false);
		break;
	}
}

void Field::mouseReleaseEvent(QMouseEvent *e)
{
	if ( state!=Playing ) return;
	setMood(Smiley::Normal);

	if ( inside(ic, jc) )
		if (_autoreveal) pressClearFunction(ic, jc, true);
	_reveal = false;
	_autoreveal = false;
	if ( !placeCursor(xToI(e->pos().x()), yToJ(e->pos().y())) ) return;

	switch ( mapMouseButton(e) ) {
	case Reveal:
		reveal();
		break;
	case Mark:
	case UMark:
		break;
	case AutoReveal:
		autoReveal();
		break;
	}
}

void Field::mouseMoveEvent(QMouseEvent *e)
{
	if ( state!=Playing ) return;

	if ( inside(ic, jc) ) {
		if (_reveal) pressCase(ic, jc, true);
		if (_autoreveal) pressClearFunction(ic, jc, true);
	}

	if ( !placeCursor(xToI(e->pos().x()), yToJ(e->pos().y())) ) return;

	if (_reveal) pressCase(ic, jc, false);
	else if (_autoreveal) pressClearFunction(ic, jc, false);
}

void Field::showMines()
{
	for(uint i=1; i<=lev.width; i++)
		for(uint j=1; j<=lev.height; j++) {
		    if ( !pfield(i, j).mine ) continue;
			if ( pfield(i, j).state!=Exploded && pfield(i, j).state!=Marked )
				changeCaseState(i, j, Uncovered);
		}
}

void Field::pressCase(uint i, uint j, bool pressed, QPainter *p)
{
	if ( pfield(i, j).state==Covered ) drawBox(iToX(i), jToY(j), pressed, p);
}

void Field::pressClearFunction(uint i, uint j, bool pressed)
{
	QPainter p(this);
	pressCase(i-1, j+1, pressed, &p);
	pressCase(i-1,   j, pressed, &p);
	pressCase(i-1, j-1, pressed, &p);
	pressCase(  i, j+1, pressed, &p);
	pressCase(  i,   j, pressed, &p);
	pressCase(  i, j-1, pressed, &p);
	pressCase(i+1, j-1, pressed, &p);
	pressCase(i+1,   j, pressed, &p);
	pressCase(i+1, j+1, pressed, &p);
}

#define M_OR_U(i, j) ( pfield(i, j).state==Marked \
					   || pfield(i, j).state==Uncertain )

void Field::keyboardAutoReveal()
{
	pressClearFunction(ic, jc, false);
	QTimer::singleShot(50, this, SLOT(keyboardAutoRevealSlot()));
}

void Field::keyboardAutoRevealSlot()
{
	pressClearFunction(ic, jc, true);
	drawCursor(true);
	autoReveal();
}

void Field::autoReveal()
{
	if ( state!=Playing ) return;
	switch (pfield(ic, jc).state) {
	case Covered:
	case Marked:
	case Uncertain: return;
	default:        break;
	}
	
	/* number of mines around the case */
	uint nm = computeNeighbours(ic, jc);
	if M_OR_U(ic-1,   jc) nm--;
	if M_OR_U(ic-1, jc+1) nm--;
	if M_OR_U(ic-1, jc-1) nm--;
	if M_OR_U(  ic, jc+1) nm--;
	if M_OR_U(  ic, jc-1) nm--;
	if M_OR_U(ic+1,   jc) nm--;
	if M_OR_U(ic+1, jc+1) nm--;
	if M_OR_U(ic+1, jc-1) nm--;
	
	if (!nm) { /* the number of surrounding mines is equal */
		       /* to the number of marks :) */
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
	if ( pfield(i, j).state==Covered ) {
		if ( pfield(i, j).mine ) changeCaseState(i, j, Uncovered);
		else uncover(i, j);
	}

	/* to enable multiple explosions ... */
	if ( pfield(i, j).mine && pfield(i, j).state==Uncovered ) {
		changeCaseState(i, j, Exploded);
		_endGame();
	}
}

void Field::_endGame()
{
	if ( state==Stopped ) return;
	/* find all errors */
	for (uint ii=1; ii<lev.width+1; ii++)
		for (uint jj=1; jj<lev.width+1; jj++)
			if ( pfield(ii, jj).state==Marked && !pfield(ii, jj).mine )
				changeCaseState(ii, jj, Error);
	emit endGame();
}

void Field::pause()
{
	if (first_click) return;
	
	/* if already in pause : resume game */
	if ( state==Paused ) resume();
	else {
		emit freezeTimer();
		eraseField();
		pb->show();
		pb->setFocus();
		state = Paused;
		emit gameStateChanged(Paused);
	}
}

void Field::resume()
{
	state = Playing;
	emit gameStateChanged(Playing);
	pb->hide();
	emit startTimer();
	update();
}

void Field::up()
{
	placeCursor(ic, jc-1, true);
}

void Field::down()
{
	placeCursor(ic, jc+1, true);
}

void Field::left()
{
	placeCursor(ic-1, jc, true);
}

void Field::right()
{
	placeCursor(ic+1, jc, true);
}

void Field::reveal()
{
	if ( state!=Playing ) return;
	if ( first_click ) {
		// set mines positions on field ; must avoid the first
		// clicked case
		for(uint k=0; k<lev.nbMines; k++) {
			uint i, j;
			do {
				i = random.getLong(lev.width);
				j = random.getLong(lev.height);
			} while ( pfield(i+1, j+1).mine
					  || ((i+1)==(uint)ic && (j+1)==(uint)jc) );
			
			pfield(i+1, j+1).mine = true;
		}
		emit startTimer();
		first_click = false;
		emit gameStateChanged(Playing);
	}
	
	uncoverCase(ic, jc);
}

void Field::mark()
{
	if ( state!=Playing ) return;
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
	switch (pfield(ic, jc).state) {
	case Covered:
	case Marked:    changeCaseState(ic, jc, Uncertain); break;
	case Uncertain: changeCaseState(ic, jc, Covered); break;
	default:        break;
	}
}

bool Field::placeCursor(int i, int j, bool check)
{
	if ( check && (state!=Playing || !inside(i, j)) ) return false;
	if ( cursor && inside(ic, jc) ) drawCursor(false);
	ic = i;
	jc = j;
	if ( !inside(i, j) ) return false;
	if (cursor) drawCursor(true);
	return true;
}

void Field::setCursor(bool show)
{
	if ( state==Playing && inside(ic, jc) ) {
		if ( cursor && !show ) drawCursor(false);
		if ( !cursor && show) drawCursor(true);
	}
	cursor = show;
}

// draw methods
QPainter *Field::begin(QPainter *pt)
{
	return (pt ? pt : new QPainter(this));
}

void Field::end(QPainter *p, const QPainter *pt)
{
	if (pt==0) delete p;
}

void Field::drawBox(int x, int y, bool pressed, QPainter *pt)
{
	QPainter *p = begin(pt);
	p->eraseRect(x, y, cp.size, cp.size);
	qDrawWinPanel(p, x, y, cp.size, cp.size, colorGroup(), !pressed);
	end(p, pt);
}

void Field::drawCase(uint i, uint j, QPainter *pt)
{
	QPainter *p = begin(pt);
	int x = iToX(i);
	int y = jToY(j);

	switch (pfield(i, j).state) {
	case Covered:   drawBox(x, y, true, p);
		            break;
	case Marked:    drawBox(x, y, true, p);
		            p->drawPixmap(x, y, pm_flag);
		            break;
	case Error:     drawBox(x, y, true, p);
					p->drawPixmap(x, y, pm_error);
		            break;
	case Uncertain: drawBox(x, y, true, p);
		            p->setPen(black);
					p->drawText(x, y, cp.size, cp.size, AlignCenter, "?");
					break;
	case Exploded:  drawBox(x, y, false, p);
		            p->drawPixmap(x, y, pm_exploded);
					break;
	case Uncovered: drawBox(x, y, false, p);
		            if ( pfield(i, j).mine ) p->drawPixmap(x, y, pm_mine);
					else {
						uint n = computeNeighbours(i, j);
						if (n) {
							char nb[2] = "0";
							nb[0] += n;
							p->setPen(cp.numberColors[n-1]);
							p->drawText(x, y, cp.size, cp.size,
										AlignCenter, nb);
						}
					}
	}

	if ( cursor && (int)i==ic && (int)j==jc ) drawCursor(true, p);
	end(p, pt);
}

void Field::eraseField()
{
	QPainter p(this);
	p.eraseRect(0, 0, width(), height());
}

void Field::drawCursor(bool show, QPainter *pt)
{
	QPainter *p = begin(pt);
	if (show) p->drawPixmap(iToX(ic), jToY(jc), pm_cursor);
	else {
		bool b = cursor;
		if (b) cursor = false;
		drawCase(ic, jc, p);
		if (b) cursor = true;
	}
	end(p, pt);
}
