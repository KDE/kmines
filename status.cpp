#include "status.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qprinter.h>
#include <qobjectlist.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>

#include "defines.h"

Status::Status(QWidget *parent, const char *name)
: QWidget(parent, name)
{
// top layout
	QVBoxLayout *top = new QVBoxLayout(this, BORDER);
	top->setResizeMode( QLayout::Fixed );
	
// status bar
	QHBoxLayout *hbl = new QHBoxLayout(SEP);
	top->addLayout(hbl);

	// mines left LCD
	left = new QLCDNumber(this);
	left->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	left->installEventFilter(parent);
	hbl->addWidget(left);
	hbl->addStretch(1);
	
	// smiley
	smiley = new QPushButton(this);
	connect( smiley, SIGNAL(clicked()), SLOT(restartGame()) );
	smiley->installEventFilter(parent);
	smiley->setFixedSize(25, 25);
	smiley->setFocusPolicy(QWidget::NoFocus);
	hbl->addWidget(smiley);
	hbl->addStretch(1);
  
	QPainter pt;
	
	s_ok = new QPixmap(25,25);
	createSmileyPixmap(s_ok, &pt);
	pt.drawPoint(8,14); pt.drawPoint(16,14);
	pt.drawLine(9,15,15,15);
	pt.end();
	s_stress = new QPixmap(25,25);
	createSmileyPixmap(s_stress, &pt);
	pt.drawPoint(12,13);
	pt.drawLine(11,14,11,15); pt.drawLine(13,14,13,15);
	pt.drawPoint(12,16);
	pt.end();
	s_happy = new QPixmap(25,25);
	createSmileyPixmap(s_happy, &pt);
	pt.drawPoint(7,14); pt.drawPoint(17,14);
	pt.drawPoint(8,15); pt.drawPoint(16,15);
	pt.drawPoint(9,16); pt.drawPoint(15,16);
	pt.drawLine(10,17,14,17);
	pt.end();
	s_ohno = new QPixmap(25,25);
	createSmileyPixmap(s_ohno, &pt);  pt.drawPoint(12,11);
	pt.drawLine(10,13,14,13);
	pt.drawLine(9,14,9,17); pt.drawLine(15,14,15,17);
	pt.drawLine(10,18,14,18);
	pt.end();

	// digital clock LCD
	dg = new DigitalClock(this);
	dg->installEventFilter(parent);
	hbl->addWidget(dg);

// game over label	
	mesg = new QLabel(" ", this); // empty string to get the right height
	mesg->setFont(QFont("Times", 14, QFont::Bold) );
	mesg->setFrameStyle(  QFrame::Panel | QFrame::Sunken );
	mesg->installEventFilter(parent);
	mesg->setFixedHeight( mesg->sizeHint().height() );
	top->addWidget(mesg, 0, AlignCenter);
	
// mines field	
	field = new Field(this);
	connect( field, SIGNAL(changeCase(uint,uint)),
			 SLOT(changeCase(uint,uint)) );
	connect( field, SIGNAL(putMsg(const QString &)),
			 SLOT(setMsg(const QString &)) );
	connect( field, SIGNAL(updateStatus(bool)), SLOT(update(bool)) );
	connect( field, SIGNAL(endGame(int)), SLOT(endGame(int)) );
	connect( field, SIGNAL(startTimer()), dg, SLOT(start()) );
	connect( field, SIGNAL(freezeTimer()), dg, SLOT(freeze()) );
	connect( field, SIGNAL(updateSmiley(int)), this, SLOT(updateSmiley(int)) );
	top->addWidget(field);
	
	/* configuration & highscore initialisation */
	KConfig *kconf = kapp->getConfig();
	for (int i=0; i<3; i++) {
		kconf->setGroup(HS_GRP[i]);
		if ( !kconf->hasKey(HS_NAME_KEY) )
			kconf->writeEntry(HS_NAME_KEY, i18n("Anonymous"));
		if ( !kconf->hasKey(HS_MIN_KEY) )
			kconf->writeEntry(HS_MIN_KEY, 59);
		if ( !kconf->hasKey(HS_SEC_KEY) )
			kconf->writeEntry(HS_SEC_KEY, 59);    
	}
}

void Status::createSmileyPixmap(QPixmap *pm, QPainter *pt)
{
	pm->fill(yellow);
	pt->begin(pm);
	pt->setPen(black);
	pt->drawLine(9,3,15,3);
	pt->drawLine(7,4,8,4); pt->drawLine(16,4,17,4);
	pt->drawPoint(6,5); pt->drawPoint(18,5);
	pt->drawPoint(5,6); pt->drawPoint(19,6);
	pt->drawLine(4,7,4,8); pt->drawLine(20,7,20,8);
	pt->drawLine(8,7,9,7); pt->drawLine(15,7,16,7);
	pt->drawLine(3,9,3,14); pt->drawLine(21,9,21,14);
	pt->drawPoint(12,10);
	pt->drawLine(4,15,4,17); pt->drawLine(20,15,20,17);
	pt->drawPoint(5,18); pt->drawPoint(19,18);
	pt->drawLine(6,19,7,19); pt->drawLine(17,19,18,19);
	pt->drawLine(8,20,9,20); pt->drawLine(15,20,16,20);
	pt->drawLine(10,21,14,21);
}

void Status::initGame()
{
	uncovered = 0; uncertain = 0; marked = 0;
	setMsg(i18n("Stopped"));
	update(FALSE);
	updateSmiley(OK);
	dg->zero();
}

void Status::restartGame()
{
	field->restart();
	initGame();
}

bool Status::newGame(uint l)
{
	_type = (GameType)l;
	uint nb_w, nb_h, nb_m;
	if ( _type==Custom ) {
		nb_w = field->nbWidth();
		nb_h = field->nbHeight();
		nb_m = field->nbMines();
		CustomDialog cu(&nb_w, &nb_h, &nb_m, this);
		if ( !cu.exec() ) return FALSE;
	} else {
		nb_w = MODES[l][0];
		nb_h = MODES[l][1];
		nb_m = MODES[l][2];
	}
	
	field->start(nb_w, nb_h, nb_m);
	initGame();
	return TRUE;
}
	
void Status::changeCase(uint case_mode, uint inc)
{
	switch(case_mode) {
	 case UNCOVERED : uncovered += inc; break;
	 case UNCERTAIN : uncertain += inc; break;
	 case MARKED    : marked    += inc; break;
	}
}

void Status::update(bool mine)
{
	QString str;
	str.setNum(field->nbMines() - marked);
	left->display(str);
	
	if ( uncovered==(field->nbWidth()*field->nbHeight() - field->nbMines()) ) endGame(!mine);
}

void Status::updateSmiley(int mood)
{
	switch (mood) {
	 case OK      : smiley->setPixmap(*s_ok); break;
	 case STRESS  : smiley->setPixmap(*s_stress); break;
	 case HAPPY   : smiley->setPixmap(*s_happy); break;
	 case UNHAPPY : smiley->setPixmap(*s_ohno); break;
	}
}

void Status::endGame(int win)
{
	field->stop();
	dg->freeze();
	
	if (win) {
		emit updateSmiley(HAPPY);
		int res = 0;
		if ( _type!=Custom ) res = setHighScore(dg->sec(), dg->min(), _type);
		if ( res!=0 ) setMsg(i18n("You did it ... but not in time."));
		else setMsg(i18n("Yeeessss !"));
	} else {
		emit updateSmiley(UNHAPPY);
		setMsg(i18n("Bad luck !"));
	}
}

void Status::showHighScores()
{
	int dummy;
	WHighScores whs(TRUE, 0, 0, 0, dummy, this);
}

int Status::setHighScore(int sec, int min, int mode)
{
	int res = 0;
	WHighScores whs(FALSE, sec, min, mode, res, this);
	return res;
} 

void Status::print()
{
	QPrinter prt;
	if ( !prt.setup() ) return;

	// repaint all children widgets
	repaint(FALSE);
	const QObjectList *ol = children();
	QObjectListIt it(*ol);
	QObject *o;
	QWidget *w;
	while ( (o=it.current()) ) {
		++it;
		if ( !o->isWidgetType()) continue;
		w = (QWidget *)o;
		w->repaint(FALSE);
	}

	// write the screen region corresponding to the window
	QPainter p(&prt);
	p.drawPixmap(0, 0, QPixmap::grabWindow(winId())); 

//  QRect r = p.viewport();
}
