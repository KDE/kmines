#include "field.h"

#include <qdrawutil.h>
#include <qlayout.h>
#include <qbitmap.h>

#include <klocale.h>

Field::Field(QWidget *parent, const char *name)
: QFrame(parent, name), lev(LEVELS[0]), random(0),
  paused(FALSE), stopped(FALSE),
  left_down(FALSE), mid_down(FALSE), pt(this)
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

	pt.setFont( QFont("Helvetica", 14, QFont::Bold) );
}

void Field::setCaseSize(uint cs)
{
	_caseSize = cs;

	QBitmap mask;

	flagPixmap(mask, TRUE);
	flagPixmap(pm_flag, FALSE);
	pm_flag.setMask(mask);

	minePixmap(mask, TRUE, MINE);
	minePixmap(pm_mine, FALSE, MINE);
	pm_mine.setMask(mask);
  
	minePixmap(mask, TRUE, EXPLODED);
	minePixmap(pm_exploded, FALSE, EXPLODED);
	pm_exploded.setMask(mask);

	minePixmap(mask, TRUE, ERROR);
	minePixmap(pm_error, FALSE, ERROR);
	pm_error.setMask(mask);

	cursorPixmap(mask, TRUE);
	cursorPixmap(pm_cursor, FALSE);
	pm_cursor.setMask(mask);

	QFont f = pt.font();
	f.setPointSize(cs-6);
	pt.setFont(f);

	updateGeometry();
}

void Field::flagPixmap(QPixmap &pix, bool mask) const
{
	pix.resize(_caseSize, _caseSize);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 16, 16);
	p.setPen( (mask ? color1 : black) );
	p.drawLine(6, 13, 14, 13);
	p.drawLine(8, 12, 12, 12);
	p.drawLine(9, 11, 11, 11);
	p.drawLine(10, 2, 10, 10);
	if (!mask) p.setPen(red);
	p.setBrush( (mask ? color1 : red) );
	p.drawRect(4, 3, 6, 5);
}

void Field::cursorPixmap(QPixmap &pix, bool mask) const
{
	pix.resize(_caseSize, _caseSize);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 20, 20);
	p.setPen( (mask ? color1 : black) );
	p.drawRect(2, 2, 16, 16);
}

void Field::minePixmap(QPixmap &pix, bool mask, uint type) const
{
	pix.resize(_caseSize, _caseSize);
	if (mask) pix.fill(color0);
	QPainter p(&pix);
	p.setWindow(0, 0, 20, 20);

	if ( type==EXPLODED ) p.fillRect(2, 2, 16, 16, (mask ? color1 : red));

	QPen pen(mask ? color1 : black, 1);
	p.setPen(pen);
	p.setBrush(mask ? color1 : black);
	p.drawLine(10,3,10,18);
	p.drawLine(3,10,18,10);
	p.drawLine(5, 5, 16, 16);
	p.drawLine(5, 15, 15, 5);
	p.drawEllipse(5, 5, 11, 11);
	
	p.fillRect(8, 8, 2, 2, (mask ? color1 : white));

	if ( type==ERROR ) {
		if (!mask) {
			pen.setColor(red);
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
	return QSize(2*frameWidth() + lev.width*_caseSize,
				 2*frameWidth() + lev.height*_caseSize);
}

QSizePolicy Field::sizePolicy() const
{
	return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

uint &Field::pfield(uint i, uint j) const
{
	return _pfield[i + j*(lev.width+2)];
}

uint Field::computeNeighbours(uint i, uint j) const
{
	uint nm = 0;
	
	if (pfield(i-1,   j) & MINE) nm++;
	if (pfield(i-1, j+1) & MINE) nm++;
	if (pfield(i-1, j-1) & MINE) nm++;
	if (pfield(  i, j+1) & MINE) nm++;
	if (pfield(  i, j-1) & MINE) nm++;
	if (pfield(i+1,   j) & MINE) nm++;
	if (pfield(i+1, j+1) & MINE) nm++;
	if (pfield(i+1, j-1) & MINE) nm++;
	
	return nm;
}

void Field::start(const Level &l)
{
	lev = l;
	restart(FALSE);
	updateGeometry();
}

void Field::restart(bool repaint)
{
	/* if game is paused : resume before restart */
	if ( paused ) {
		resume();
		emit freezeTimer();
	}

	stopped = FALSE;
	first_click = TRUE;

	_pfield.resize( (lev.width+2) * (lev.height+2) );
	
	for (uint i=0; i<lev.width+2; i++)
		for (uint j=0; j<lev.height+2; j++) {
			uint tmp = (i==0 || i==lev.width+1 || j==0 || j==lev.height+1
						? UNCOVERED : COVERED);
			if ( pfield(i, j)==tmp ) continue;
			pfield(i, j) = tmp;
			if (repaint && tmp==COVERED) drawCase(i, j);
		}

	if (repaint) drawCursor(FALSE);
	ic = lev.width/2;
	jc = lev.height/2;
	if ( repaint && cursor ) drawCursor(TRUE);
}

void Field::paintEvent(QPaintEvent *e)
{
	if (paused) return;		

	drawFrame(&pt);
	uint imin = (uint)QMAX(QMIN(xToI(e->rect().left()), (int)lev.width), 1);
	uint imax = (uint)QMAX(QMIN(xToI(e->rect().right()), (int)lev.width), 1);
	uint jmin = (uint)QMAX(QMIN(yToJ(e->rect().top()), (int)lev.height), 1);
	uint jmax = (uint)QMAX(QMIN(yToJ(e->rect().bottom()), (int)lev.height), 1);
	for (uint i=imin; i<=imax; i++)
	    for (uint j=jmin; j<=jmax; j++) drawCase(i, j);
}

void Field::changeCaseState(uint i, uint j, uint new_st)
{
	if (pfield(i, j) & MINE) {
		emit changeCase(pfield(i, j) ^ MINE, -1);
		pfield(i, j) = MINE | new_st;
	} else {
		emit changeCase(pfield(i, j), -1);
		pfield(i, j) = new_st;
	}
  
	emit changeCase(new_st, 1);
	drawCase(i, j);
	if (!stopped) emit updateStatus(pfield(i, j) & MINE);
}

int Field::iToX(uint i) const
{
	return (i-1)*_caseSize + frameWidth();
}

int Field::jToY(uint j) const
{
	return (j-1)*_caseSize + frameWidth();
}

int Field::xToI(int x) const
{
	// the cast is necessary when x-frameWidth() is negative (??)
	return (int)((float)(x - frameWidth())/_caseSize) + 1;
}

int Field::yToJ(int y) const
{
	return (int)((float)(y - frameWidth())/_caseSize) + 1;
}

void Field::drawCase(uint i, uint j)
{
	int x = iToX(i);
	int y = jToY(j);
	
	pt.eraseRect(x, y, _caseSize, _caseSize);
	bool unrevealed =  pfield(i, j) & (COVERED | MARKED | UNCERTAIN | ERROR);
	qDrawWinPanel(&pt, x, y, _caseSize, _caseSize, colorGroup(), !unrevealed);
  
	if (unrevealed) {
		if (pfield(i, j) & MARKED) bitBlt(this, x, y, &pm_flag);
		else if (pfield(i, j) & UNCERTAIN) {
			pt.setPen(black);
			pt.drawText(x, y, _caseSize, _caseSize, AlignCenter, "?");
		} else if (pfield(i, j) & ERROR) bitBlt(this, x, y, &pm_error);
	} else {
		if (pfield(i, j) & MINE) {
			const QPixmap &pm
				= (pfield(i, j) & EXPLODED ? pm_exploded : pm_mine);
			bitBlt(this, x, y, &pm);
		} else {
			uint nbs = computeNeighbours(i, j);
			if (nbs) {
				char nb[2] = "0";
				nb[0] += nbs;
				switch(nbs) {
				 case 1: pt.setPen(blue);        break;
				 case 2: pt.setPen(darkGreen);   break;
				 case 3: pt.setPen(darkYellow);  break;
				 case 4: pt.setPen(darkMagenta); break;
				 case 5: pt.setPen(red);         break;
				 case 6: pt.setPen(darkRed);     break;
				 case 7:
				 case 8: pt.setPen(black);       break;
				 }
				pt.drawText(x, y, _caseSize, _caseSize, AlignCenter, nb);
				}
		}
	}

	if ( cursor && (int)i==ic && (int)j==jc ) drawCursor(TRUE);
}

void Field::uncover(uint i, uint j)
{
	if ( !(pfield(i, j) & COVERED) ) return;
	uint nbs = computeNeighbours(i, j);
  
	if (!nbs) {
		changeCaseState(i,j,UNCOVERED);
		uncover(i-1, j+1);
		uncover(i-1,   j);
		uncover(i-1, j-1);
		uncover(  i, j+1);
		uncover(  i, j-1);
		uncover(i+1, j+1);
		uncover(i+1,   j);
		uncover(i+1, j-1);
	} else changeCaseState(i,j,UNCOVERED);
}

bool Field::inside(int i, int j) const
{
	return ( i>=1 && i<=(int)lev.width && j>=1 && j<=(int)lev.height);
}

void Field::mousePressEvent(QMouseEvent *e)
{
	if ( locked() ) return;
	setMood(Smiley::Stressed);
	if ( !placeCursor(xToI(e->pos().x()), yToJ(e->pos().y())) ) return;

	if ( e->button()==LeftButton ) {
		left_down = TRUE;
		pressCase(ic, jc, TRUE);
	} else if ( e->button()==RightButton ) mark();
	else if ( e->button()==MidButton ) {
		mid_down = TRUE;
		pressClearFunction(ic, jc, TRUE);
	}
}

void Field::mouseReleaseEvent(QMouseEvent *e)
{
	if ( locked() ) return;
	setMood(Smiley::Normal);
	if ( !inside(ic, jc) ) return;

	if ( e->button()==LeftButton ) {
		if ( !left_down ) return;
		left_down = FALSE;
		reveal();
	} else if ( e->button()==MidButton ) {
		mid_down = FALSE;
		pressClearFunction(ic, jc, FALSE);
		autoReveal();
	}
}

void Field::mouseMoveEvent(QMouseEvent *e)
{
	if ( locked() ) return; 

	if ( inside(ic, jc) ) {
		if (left_down) pressCase(ic, jc, FALSE);
		else if (mid_down) pressClearFunction(ic, jc, FALSE);
	}

	if ( !placeCursor(xToI(e->pos().x()), yToJ(e->pos().y())) ) return;

	if (left_down) pressCase(ic, jc, TRUE);
	else if (mid_down) pressClearFunction(ic, jc, TRUE);
}

void Field::showMines()
{
	for(uint i=1; i<=lev.width; i++)
		for(uint j=1; j<=lev.height; j++)
		    if ( (pfield(i, j) & MINE) ) {
				if ( !(pfield(i, j) & EXPLODED) && !(pfield(i, j) & MARKED) ) 
					changeCaseState(i,j,UNCOVERED);
			} else if (pfield(i, j) & MARKED) changeCaseState(i, j, ERROR);
}

void Field::pressCase(uint i, uint j, uint state)
{
	if (pfield(i, j) & COVERED) {
		int x = iToX(i);
		int y = jToY(j);
		pt.eraseRect(x, y, _caseSize, _caseSize);
		qDrawWinPanel(&pt, x, y, _caseSize, _caseSize, colorGroup(), state);
	}
}

void Field::pressClearFunction(uint i, uint j, uint state)
{
	pressCase(i-1, j+1, state);
	pressCase(i-1,   j, state);
	pressCase(i-1, j-1, state);
	pressCase(  i, j+1, state);
	pressCase(  i,   j, state);
	pressCase(  i, j-1, state);
	pressCase(i+1, j-1, state);
	pressCase(i+1,   j, state);
	pressCase(i+1, j+1, state);
}

#define M_OR_U(i, j) ( (pfield(i, j) & MARKED) || (pfield(i, j) & UNCERTAIN) )

void Field::autoReveal()
{
	if ( locked() ) return;
	if ( pfield(ic, jc) & (COVERED|MARKED|UNCERTAIN) ) return; 
	
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
	if (pfield(i, j) & COVERED) {
		if (pfield(i, j) & MINE) changeCaseState(i, j, UNCOVERED);
		else uncover(i, j);
	}
  
	/* to enable multiple explosions ... */
	if ( (pfield(i, j) & MINE) && (pfield(i, j) & UNCOVERED) ) {
		changeCaseState(i, j, EXPLODED);
		emit endGame(FALSE);
	}
}

void Field::pause()
{
	if ( first_click || stopped ) return;
	
	/* if already in pause : resume game */
	if ( paused ) resume();
	else {
		emit freezeTimer();

		pt.eraseRect(0, 0, width(), height());
		emit putMsg(i18n("Game paused"));
		pb->show();
		pb->setFocus();
		
		paused = TRUE;
	}
}

void Field::resume()
{
	paused = FALSE;
	emit putMsg(i18n("Playing"));
	pb->hide();
	emit startTimer();
	update();
}

void Field::up()
{
	placeCursor(ic, jc-1, TRUE);
}

void Field::down()
{
	placeCursor(ic, jc+1, TRUE);
}

void Field::left()
{
	placeCursor(ic-1, jc, TRUE);
}

void Field::right()
{
	placeCursor(ic+1, jc, TRUE);
}

void Field::reveal()
{
	if ( locked() ) return;
	if ( first_click ) {
		// set mines positions on field ; must avoid the first 
		// clicked case
		for(uint k=0; k<lev.nbMines; k++) {
			uint i, j;
			do {
				i = random.getLong(lev.width);
				j = random.getLong(lev.height);
			}
			while ( (pfield(i+1, j+1) & MINE)
				   || ((i+1)==(uint)ic && (j+1)==(uint)jc) );
			
			pfield(i+1, j+1) |= MINE;
		}
		emit startTimer();
		first_click = FALSE;
		emit putMsg(i18n("Playing"));
	}
	
	uncoverCase(ic, jc);
}

void Field::mark()
{
	if ( locked() ) return;
	if (pfield(ic, jc) & COVERED) changeCaseState(ic, jc, MARKED);
	else if (pfield(ic, jc) & MARKED)
		changeCaseState(ic, jc, (u_mark ? UNCERTAIN : COVERED));
	else if (pfield(ic, jc) & UNCERTAIN) changeCaseState(ic, jc, COVERED);
}

bool Field::placeCursor(int i, int j, bool check)
{
	if ( check && (locked() || !inside(i, j)) ) return FALSE;
	if (cursor) drawCursor(FALSE);
	ic = i;
	jc = j;
	if ( !check && !inside(i, j) ) return FALSE;
	if (cursor) drawCursor(TRUE);
	return TRUE;
}

void Field::drawCursor(bool show)
{
	if (show) bitBlt(this, iToX(ic), jToY(jc), &pm_cursor);
	else {
		bool b = cursor;
		if (b) cursor = FALSE;
		drawCase(ic, jc);
		if (b) cursor = TRUE;
	}
}

void Field::setCursor(bool show)
{
	if ( !locked() ) {
		if ( cursor && !show ) drawCursor(FALSE);
		if ( !cursor && show) drawCursor(TRUE);
	}
	cursor = show;
}
