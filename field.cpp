#include "field.h"

#include <qdrawutil.h>
#include <qlayout.h>

#include <klocale.h>

#include "defines.h"

Field::Field(QWidget *parent, const char *name)
: QFrame(parent, name), nb_w(0), nb_h(0), nb_m(1),
  _stop(FALSE), isPaused(FALSE), left_down(FALSE), mid_down(FALSE),
  pt(this)
{
	setFrameStyle( QFrame::Box | QFrame::Raised );
	setLineWidth(2);
	setMidLineWidth(2);

	initRandomWithTime();
  
	QPainter p;
	pm_flag.resize(16, 16);
	pm_flag.fill( backgroundColor() );
	p.begin(&pm_flag);
	p.setPen(black);
	p.drawLine(6, 13, 14, 13);
	p.drawLine(8, 12, 12, 12);
	p.drawLine(9, 11, 11, 11);
	p.drawLine(10, 2, 10, 10);
	p.setPen(red);
	p.setBrush(red);
	p.drawRect(4, 3, 6, 5);
	p.end();

	pm_mine.resize(20, 20);
	pm_mine.fill( backgroundColor() );
	p.begin(&pm_mine);
	createMinePixmap(p);
	p.end();
  
	pm_exploded.resize(20, 20);
	pm_exploded.fill(red);
	p.begin(&pm_exploded);
	createMinePixmap(p);
	p.end();
	
	pm_error.resize(20,20);
	pm_error.fill( backgroundColor() );
	p.begin(&pm_error);
	createMinePixmap(p);
	p.setPen(red);
	p.drawLine(3, 3, 17, 17);
	p.drawLine(4, 3, 17, 16);
	p.drawLine(3, 4, 16, 17);
	p.drawLine(3, 17, 17, 3);
	p.drawLine(3, 16, 16, 3);
	p.drawLine(4, 17, 17, 4);
	p.end();

	QVBoxLayout *top = new QVBoxLayout(this, 0);
	top->addStretch(1);
	pb = new QPushButton(i18n("Press to resume"), this);
	pb->hide();
	top->addWidget(pb, 0, AlignCenter);
	connect(pb, SIGNAL(clicked()), this, SLOT(resume()));
	top->addStretch(1);
	
	setFont( QFont("Helvetica", 14, QFont::Bold) );
}

QSize Field::sizeHint() const
{
	return QSize(2*frameWidth() + nbWidth()*CASE_W,
				 2*frameWidth() + nbHeight()*CASE_H);
}

QSizePolicy Field::sizePolicy() const
{
	return QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

void Field::createMinePixmap(QPainter &p) const
{
	p.setPen(black);
	p.drawLine(0,0,19,0);
	p.drawLine(0,19,19,19);
	p.drawLine(0,1,0,18);
	p.drawLine(19,1,19,18);
	p.drawLine(10,3,10,16);
	p.drawLine(3,10,16,10);
	p.drawPoint(5,5); p.drawLine(8,5,12,5); p.drawPoint(15,5);
	p.drawRect(6,6,9,2);
	p.drawRect(5,8,3,2); p.drawRect(10,8,6,2);
	p.drawRect(5,11,11,2);
	p.drawRect(6,13,9,2);
	p.drawPoint(5,15); p.drawLine(8,15,12,15); p.drawPoint(15,15);
}

uint &Field::pfield(uint i, uint j) const
{
	return _pfield[i + j*(nb_w+2)];
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

void Field::start(uint nb_width, uint nb_height, uint nb_mines)
{
	nb_w = nb_width;
	nb_h = nb_height;
	nb_m = nb_mines;
	restart(FALSE);
	updateGeometry();
}

void Field::restart(bool repaint)
{
	/* if game is paused : resume before restart */
	if ( isPaused ) {
		resume();
		emit freezeTimer();
	}

	_stop = FALSE;
	first_click = TRUE;

	_pfield.resize((nb_w+2) * (nb_h+2));
	
	uint tmp;
	for (uint i=0; i<nb_w+2; i++)
		for (uint j=0; j<nb_h+2; j++) {
			tmp = (i==0 || i==nb_w+1 || j==0 || j==nb_h+1 ? UNCOVERED : COVERED);
			if ( pfield(i, j)==tmp ) continue;
			pfield(i, j) = tmp;
			if (repaint && tmp==COVERED) drawCase(i, j);
		}
}

void Field::paintEvent(QPaintEvent *)
{
	if (isPaused) return;		

	drawFrame(&pt);
	for (uint i=1; i<=nb_w; i++)
	    for (uint j=1; j<=nb_h; j++) drawCase(i, j);
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
	if (!_stop) emit updateStatus(pfield(i, j) & MINE);
}

int Field::iToX(uint i) const
{
	return (i-1)*CASE_W + frameWidth();
}

int Field::jToY(uint j) const
{
	return (j-1)*CASE_H + frameWidth();
}

int Field::xToI(int x) const
{
	return (x - frameWidth()) / CASE_W + 1;
}

int Field::yToJ(int y) const
{
	return (y - frameWidth()) / CASE_H + 1;
}

void Field::drawCase(uint i, uint j)
{
	int x = iToX(i);
	int y = jToY(j);
	
	pt.eraseRect(x, y, CASE_W, CASE_H);
  
	if (pfield(i, j) & COVERED)
	    qDrawWinPanel(&pt, x, y, CASE_W, CASE_H, colorGroup());
	else if (pfield(i, j) & MARKED) {
		qDrawWinPanel(&pt, x, y, CASE_W, CASE_H, colorGroup());
		bitBlt(this, x+2, y+2, &pm_flag);
	} else if (pfield(i, j) & UNCERTAIN) {
		qDrawWinPanel(&pt, x, y, CASE_W, CASE_H, colorGroup());
		pt.setPen(black);
		pt.drawText(x, y, CASE_W, CASE_H, AlignCenter, "?");
	} else if (pfield(i, j) & ERROR)
	    bitBlt(this, x, y, &pm_error);
	else {
		if (pfield(i, j) & MINE) {
			if (pfield(i, j) & EXPLODED)
			    bitBlt(this, x, y, &pm_exploded);
			else
			    bitBlt(this, x, y, &pm_mine);
		} else {
			qDrawWinPanel(&pt, x, y, CASE_W, CASE_H, colorGroup(), TRUE);
			uint nbs = computeNeighbours(i, j);
			if (nbs) {
				char nb[2] = "0";
				nb[0] += nbs;
				switch(nbs) {
				 case 1 : pt.setPen(blue);        break;
				 case 2 : pt.setPen(darkGreen);   break;
				 case 3 : pt.setPen(darkYellow);  break;
				 case 4 : pt.setPen(darkMagenta); break;
				 case 5 : pt.setPen(red);         break;
				 case 6 : pt.setPen(darkRed);     break;
				 case 7 :
				 case 8 : pt.setPen(black);       break;
				 }
				pt.drawText(x, y, CASE_W, CASE_H, AlignCenter, nb);
				}
		}
	}
}

void Field::uncover(uint i, uint j)
{
	uint nbs;
	
	if ( !(pfield(i, j) & COVERED) ) return;
	nbs = computeNeighbours(i,j);
  
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
	} else 
	    changeCaseState(i,j,UNCOVERED);
}

void Field::mousePressEvent( QMouseEvent *e )
{
	if ( _stop || isPaused ) return;

	updateSmiley(STRESS);

	ic = xToI(e->pos().x());
	jc = yToJ(e->pos().y());
	
	if (first_click) emit startTimer();

	if (e->button()==LeftButton) {
		left_down = TRUE;
		pressCase(ic, jc, TRUE);
	} else if (e->button()==RightButton) {
		if (pfield(ic, jc) & COVERED)
			changeCaseState(ic, jc, MARKED);
		else if (pfield(ic, jc) & MARKED) /* ? mark option */
			changeCaseState(ic, jc, (u_mark ? UNCERTAIN : COVERED));
		else if (pfield(ic, jc) & UNCERTAIN)
			changeCaseState(ic, jc, COVERED);
	} else if (e->button()==MidButton) {
		mid_down = TRUE;
		pressClearFunction(ic, jc, TRUE);
	}
}

void Field::mouseReleaseEvent( QMouseEvent *e )
{
	if ( _stop || isPaused ) return;

	/* if not out of the limits of the field */
	if (ic>=1 && ic<=nb_w && jc>=1 && jc<=nb_h) {
		if (e->button()==LeftButton) {
			left_down = FALSE;
			
			if ( first_click ) {
				// set mines positions on field ; must avoid the first 
				// clicked case
				for(uint k=0; k<nb_m; k++) {
					uint i, j;
					do {
						i = randomInt(0, nb_w-1);
						j = randomInt(0, nb_h-1);
					}
					while ( (pfield(i+1, j+1) & MINE)
						    || ((i+1)==ic && (j+1)==jc) );
			
					pfield(i+1, j+1) |= MINE;
				}
				first_click = FALSE;
				emit putMsg(i18n("Playing"));
			}
			
			uncoverCase(ic, jc);
		} else if (e->button()==MidButton) {
			mid_down = FALSE;
			clearFunction(ic, jc);
		}
	}
  
	if (!_stop)	updateSmiley(OK);
}

void Field::mouseMoveEvent( QMouseEvent *e )
{
	if (_stop) return; 

	/* if not out of the limits of the field */
	if (ic>=1 && ic<=nb_w && jc>=1 && jc<=nb_h) {
		if (left_down)
			pressCase(ic, jc, FALSE);
		else if (mid_down)
			pressClearFunction(ic, jc, FALSE);
	}
	
	ic = xToI(e->pos().x());
	jc = yToJ(e->pos().y());

	/* if not out of the limits of the field */
	if (ic>=1 && ic<=nb_w && jc>=1 && jc<=nb_h) {
		if (left_down)
			pressCase(ic, jc, TRUE);
		else if (mid_down)
			pressClearFunction(ic, jc, TRUE);
	} 
}

/* Shown mines on explosion */
void Field::showMines(uint i0, uint j0)
{
	for(uint i=1; i<=nb_w; i++)
		for(uint j=1; j<=nb_h; j++)
		    if ( (pfield(i, j) & MINE) ) {
				if ( !(pfield(i, j) & EXPLODED) && !(pfield(i, j) & MARKED) ) 
					changeCaseState(i,j,UNCOVERED);
			} else if (pfield(i, j) & MARKED) changeCaseState(i, j, ERROR);
			
	changeCaseState(i0, j0, EXPLODED);
}

void Field::pressCase(uint i, uint j, uint state)
{
	int x = iToX(i);
	int y = jToY(j);
	
	if (pfield(i, j) & COVERED) {
		pt.eraseRect(x, y, CASE_W, CASE_H);
		qDrawWinPanel(&pt, x, y, CASE_W, CASE_H, colorGroup(), state);
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

void Field::clearFunction(uint i, uint j)
{
	pressClearFunction(i, j, FALSE);
	
	if ( pfield(i, j) & (COVERED|MARKED|UNCERTAIN) ) return; 
	
	uint nm;
	/* number of mines around the case */
	nm = computeNeighbours(i, j);
	
	if M_OR_U(i-1,   j) nm--;
	if M_OR_U(i-1, j+1) nm--;
	if M_OR_U(i-1, j-1) nm--;
	if M_OR_U(  i, j+1) nm--;
	if M_OR_U(  i, j-1) nm--;
	if M_OR_U(i+1,   j) nm--;
	if M_OR_U(i+1, j+1) nm--;
	if M_OR_U(i+1, j-1) nm--;
	
	if (!nm) { /* the number of surrounding mines is equal */
		       /* to the number of marks :) */
		uncoverCase(i+1, j+1);
		uncoverCase(i+1,   j);
		uncoverCase(i+1, j-1);
		uncoverCase(  i, j+1);
		uncoverCase(  i, j-1);
		uncoverCase(i-1, j+1);
		uncoverCase(i-1,   j);
		uncoverCase(i-1, j-1);
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
		emit endGame(FALSE);
		showMines(i, j);
	}
}

void Field::pause()
{
	if ( first_click || _stop ) return;
	
	/* if already in pause : resume game */
	if ( isPaused ) {
		resume();
		return;
	}

	emit freezeTimer();

	pt.eraseRect(0, 0, width(), height());
	emit putMsg(i18n("Paused"));
	pb->show();
	pb->setFocus();

	isPaused = TRUE;
}

void Field::resume()
{
	isPaused = FALSE;
	emit putMsg(i18n("Playing"));
	pb->hide();
	emit startTimer(); 
}
