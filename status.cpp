#include "status.moc"

#include <qpainter.h>
#include <qpixmap.h>
#include <kprinter.h>
#include <qobjectlist.h>
#include <qwhatsthis.h>
#include <qlayout.h>

#include <kapp.h>
#include <klocale.h>
#include <kconfig.h>

#include "dialogs.h"

#define BORDER   10
#define SEP      10

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
	left = new LCDNumber(this);
	left->installEventFilter(parent);
	QWhatsThis::add(left, i18n("Mines left"));
	hbl->addWidget(left);
	hbl->addStretch(1);
	
	// smiley
	smiley = new Smiley(this);
	connect( smiley, SIGNAL(clicked()), SLOT(restartGame()) );
	smiley->installEventFilter(parent);
	smiley->setFocusPolicy(QWidget::NoFocus);
	QWhatsThis::add(smiley, i18n("Press to start a new game"));
	hbl->addWidget(smiley);
	hbl->addStretch(1);

	// digital clock LCD
	dg = new DigitalClock(this);
	dg->installEventFilter(parent);
	QWhatsThis::add(dg, i18n("Time elapsed"));
	hbl->addWidget(dg);
	
// mines field	
	field = new Field(this);
	connect( field, SIGNAL(changeCase(CaseState, uint)),
			 SLOT(changeCase(CaseState, uint)) );
	connect( field, SIGNAL(updateStatus(bool)), SLOT(update(bool)) );
	connect( field, SIGNAL(endGame()), SLOT(endGame()) );
	connect( field, SIGNAL(startTimer()), dg, SLOT(start()) );
	connect( field, SIGNAL(freezeTimer()), dg, SLOT(freeze()) );
	connect( field, SIGNAL(setMood(Smiley::Mood)),
			 smiley, SLOT(setMood(Smiley::Mood)) );
	connect(field, SIGNAL(gameStateChanged(GameState)),
			SLOT(setGameState(GameState)) );
	QWhatsThis::add(field, i18n("Mines field"));
	top->addWidget(field);
}

void Status::setGameState(GameState s)
{
	switch (s) {
	case Stopped: emit message(i18n("Game stopped")); break;
	case Playing: emit message(i18n("Playing"));      break;
	case Paused:  emit message(i18n("Game paused"));  break;
	}
	emit gameStateChanged(s);
}

void Status::initGame()
{
	uncovered = 0;
	uncertain = 0;
	marked    = 0;
	setGameState(Stopped);
	update(false);
	smiley->setMood(Smiley::Normal);
	GameType type = field->level().type;
	if ( type!=Custom )	dg->setMaxTime( WHighScores::time(type) );
	dg->zero();
}

void Status::restartGame()
{
	field->restart();
	initGame();
}

void Status::newGame(const Level &l)
{
	field->setLevel(l);
	initGame();
}
	
void Status::changeCase(CaseState cs, uint inc)
{
	switch (cs) {
	case Uncovered: uncovered += inc; break;
	case Uncertain: uncertain += inc; break;
	case Marked:    marked    += inc; break;
	default:                          break;
	}
}

void Status::update(bool mine)
{
	QString str;
	const Level &l = field->level();
	int r = l.nbMines - marked;
	int u = l.width*l.height - l.nbMines - uncovered; // cannot be negative
	left->setState(r<0 && u!=0);
	left->display(r);
	if ( u==0 ) _endGame(!mine);
}

void Status::_endGame(bool win)
{
	field->stop();
	dg->freeze();
	field->showMines();
	setGameState(Stopped);
	
	if (win) {
		GameType type = field->level().type;
		smiley->setMood(Smiley::Happy);
		if ( type==Custom || dg->better() ) {
			emit message(i18n("Yeeeesssssss!"));
			if ( type!=Custom && dg->better() ) {
				Score score;
				score.sec  = dg->sec();
				score.min  = dg->min();
				score.type = type;
				highScores(&score);
			}
		} else emit message(i18n("You did it ... but not in time."));
	} else {
		smiley->setMood(Smiley::Sad);
		emit message(i18n("Bad luck!"));
	}
}

void Status::showHighScores()
{
	field->pause();
	WHighScores whs(this, 0);
	whs.exec();
	field->pause();
}

void Status::highScores(const Score *score)
{
	WHighScores whs(this, score);
	whs.exec();
}

void Status::print()
{
	KPrinter prt;
	if ( !prt.setup() ) return;

	// repaint all children widgets
	repaint(false);
	const QObjectList *ol = children();
	QObjectListIt it(*ol);
	QObject *o;
	QWidget *w;
	while ( (o=it.current()) ) {
		++it;
		if ( !o->isWidgetType()) continue;
		w = (QWidget *)o;
		w->repaint(false);
	}

	// write the screen region corresponding to the window
	QPainter p(&prt);
	p.drawPixmap(0, 0, QPixmap::grabWindow(winId()));
}

void Status::preferences()
{
	OptionDialog od(this);
	if ( !od.exec() ) return;
	field->readSettings();
	emit keyboardEnabled(OptionDialog::readKeyboard());
}
