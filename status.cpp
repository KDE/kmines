#include "status.h"

#include <qpainter.h>
#include <qpixmap.h>
#include <qprinter.h>
#include <qobjectlist.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>

#include "defines.h"
#include "dialogs.h"

#include "bitmaps/smile"
#include "bitmaps/smile_happy"
#include "bitmaps/smile_ohno"
#include "bitmaps/smile_stress"

#define BORDER   10
#define SEP      10

Status::Status(QWidget *parent, const char *name)
: QWidget(parent, name), s_ok(smile_xpm), s_happy(smile_happy_xpm),
  s_ohno(smile_ohno_xpm), s_stress(smile_stress_xpm)
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
	left->setSegmentStyle(QLCDNumber::Flat);
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

	// digital clock LCD
	dg = new DigitalClock(this);
	dg->installEventFilter(parent);
	hbl->addWidget(dg);
	
// mines field	
	field = new Field(this);
	connect( field, SIGNAL(changeCase(uint,uint)),
			 SLOT(changeCase(uint,uint)) );
	connect( field, SIGNAL(putMsg(const QString &)),
			 SLOT(putMessage(const QString &)) );
	connect( field, SIGNAL(updateStatus(bool)), SLOT(update(bool)) );
	connect( field, SIGNAL(endGame(int)), SLOT(endGame(int)) );
	connect( field, SIGNAL(startTimer()), dg, SLOT(start()) );
	connect( field, SIGNAL(freezeTimer()), dg, SLOT(freeze()) );
	connect( field, SIGNAL(updateSmiley(int)), this, SLOT(updateSmiley(int)) );
	top->addWidget(field);

	message = new QLabel(this);
	message->setAlignment(AlignCenter);
	top->addWidget(message);
}

void Status::initGame()
{
	uncovered = 0; uncertain = 0; marked = 0;
	message->setText(i18n("Game stopped"));
	update(FALSE);
	updateSmiley(OK);
	if ( _type!=Custom ) dg->setMaxTime( WHighScores::time(_type) );
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
	Level lev;
	if ( _type==Custom ) {
		lev = field->level();
		CustomDialog cu(lev, this);
		if ( !cu.exec() ) return FALSE;
	} else lev = LEVELS[l];
	
	field->start(lev);
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
	const Level &l = field->level();
	int r = l.nbMines - marked;
	int u = l.width*l.height - l.nbMines - uncovered; // cannot be negative
	if ( (r*left->value())<=0 ) {
		QPalette p = palette();
		if ( r<=0 && u!=0 ) p.setColor(QColorGroup::Background, QColor(200, 0, 0));
		left->setPalette(p);
	}
	left->display(r);
	if ( u==0 ) endGame(!mine);
}

void Status::updateSmiley(int mood)
{
	switch (mood) {
	 case OK      : smiley->setPixmap(s_ok); break;
	 case STRESS  : smiley->setPixmap(s_stress); break;
	 case HAPPY   : smiley->setPixmap(s_happy); break;
	 case UNHAPPY : smiley->setPixmap(s_ohno); break;
	}
}

void Status::putMessage(const QString &str)
{
	message->setText(str);
}

void Status::endGame(int win)
{
	field->stop();
	dg->freeze();
	field->showMines();
	
	if (win) {
		updateSmiley(HAPPY);
		if ( _type==Custom || dg->better() ) {
			message->setText(i18n("Yeeeesssssss!"));
			if ( dg->better() ) {
				Score score;
				score.sec = dg->sec();
				score.min = dg->min();
				score.mode = _type;
				highScores(&score);
			}
		} else message->setText(i18n("You did it ... but not in time."));
	} else {
		updateSmiley(UNHAPPY);
		message->setText(i18n("Bad luck!"));
	}
}

void Status::highScores(const Score *score)
{
	WHighScores whs(this, score);
	whs.exec();
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
}
