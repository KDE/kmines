#include "field.h"
#include "field.moc"

#include <math.h>

#include <qlayout.h>
#include <qbitmap.h>
#include <qapplication.h>

#include <klocale.h>

Field::Field(QWidget *parent, const char *name)
: QFrame(parent, name), lev(LEVELS[0]), random(0), state(Stopped),
  u_mark(false), cursor(false)
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

	dummy = new QPushButton(0);

	readSettings();
}

Field::~Field()
{
	delete dummy;
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
	dummy->resize(cp.size, cp.size);

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

void Field::setLevel(const LevelData &l)
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
	currentAction = None;
	ic = lev.width/2;
	jc = lev.height/2;

	_pfield.resize( (lev.width+2) * (lev.height+2) );


	for (uint i=0; i<lev.width+2; i++)
		for (uint j=0; j<lev.height+2; j++) {
			Case tmp;
			tmp.mine  = false;
			tmp.state = (i==0 || i==lev.width+1 || j==0 || j==lev.height+1
						 ? Uncovered : Covered);
			if ( pfield(i, j).mine==false && pfield(i, j).state==tmp.state )
				continue;
			pfield(i, j) = tmp;
			if (repaint && tmp.state==Covered) drawCase(i, j);
		}
}

void Field::paintEvent(QPaintEvent *e)
{
	QPainter p(this);
	drawFrame(&p);
	p.end();

	if ( state==Paused ) return;

	uint imin = (uint)QMAX(QMIN(xToI(e->rect().left()), (int)lev.width), 1);
	uint imax = (uint)QMAX(QMIN(xToI(e->rect().right()), (int)lev.width), 1);
	uint jmin = (uint)QMAX(QMIN(yToJ(e->rect().top()), (int)lev.height), 1);
	uint jmax = (uint)QMAX(QMIN(yToJ(e->rect().bottom()), (int)lev.height), 1);
	for (uint i=imin; i<=imax; i++)
	    for (uint j=jmin; j<=jmax; j++) drawCase(i, j);
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
	for(uint i=1; i<=lev.width; i++)
		for(uint j=1; j<=lev.height; j++) {
		    if ( !pfield(i, j).mine ) continue;
			if ( pfield(i, j).state!=Exploded && pfield(i, j).state!=Marked )
				changeCaseState(i, j, Uncovered);
		}
}

void Field::pressCase(uint i, uint j, bool pressed)
{
	if ( pfield(i, j).state==Covered ) drawBox(i, j, pressed);
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
		for (uint jj=1; jj<lev.height+1; jj++)
			if ( pfield(ii, jj).state==Marked && !pfield(ii, jj).mine )
				changeCaseState(ii, jj, Error);
	emit endGame();
}

void Field::pause()
{
	if ( first_click || state==Stopped ) return;

	/* if already in pause : resume game */
	if ( state==Paused ) resume();
	else {
		emit freezeTimer();
		QFont f = QApplication::font();
		f.setBold(true);
		pb->setFont(f);
		pb->show();
		pb->setFocus();
		state = Paused;
		emit gameStateChanged(Paused);
		update();
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

bool Field::placeCursor(int i, int j)
{
	if ( state!=Playing || !inside(i, j) ) return false;
	uint oldIc = ic;
	uint oldJc = jc;
	ic = (uint)i;
	jc = (uint)j;
	if (cursor) {
		drawCase(oldIc, oldJc);
		drawCase(ic, jc);
	}
	return true;
}

void Field::setCursor(bool show)
{
	cursor = show;
	if ( state==Playing ) drawCase(ic, jc);
}

// draw methods
void Field::drawBox(uint i, uint j, bool pressed,
					const QString &text, const QColor *textColor,
					const QPixmap *pixmap)
{
	QPainter p(this);
	int x = iToX(i);
	int y = jToY(j);

	// draw button
	p.translate(x, y);
	dummy->setDown(pressed);
	style().drawPushButton(dummy, &p);
	p.resetXForm();

	// draw text and pixmap
	style().drawItem(&p, x, y, cp.size, cp.size, AlignCenter, colorGroup(),
					 true, pixmap, text, -1, textColor);

	// draw cursor
	if ( cursor && i==ic && j==jc ) {
		QRect r = style().buttonRect(x+1, y+1, cp.size-2, cp.size-2);
		style().drawFocusRect(&p, r, colorGroup());
	}
}

void Field::drawCase(uint i, uint j)
{
	ASSERT( inside(i, j) );
	switch (pfield(i, j).state) {
	case Covered:   drawBox(i, j, false);
		            break;
	case Marked:    drawBox(i, j, false, &pm_flag);
		            break;
	case Error:     drawBox(i, j, true, &pm_error);
		            break;
	case Uncertain: drawBox(i, j, false, "?");
					break;
	case Exploded:  drawBox(i, j, true, &pm_exploded);
					break;
	case Uncovered: if ( pfield(i, j).mine )
		                drawBox(i, j, true, &pm_mine);
	                else {
						uint n = computeNeighbours(i, j);
						QString nb;
						if (n) nb.setNum(n);
						drawBox(i, j, true, nb,
								n ? &cp.numberColors[n-1] : 0);
					}
	}
}
