#include "status.h"
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
#include "highscores.h"

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
	connect( field, SIGNAL(changeCase(CaseState, int)),
			 SLOT(changeCase(CaseState, int)) );
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
	Level level = field->level().level;
	if ( level!=Custom ) {
        ExtScore score(level);
        dg->setBestScores( score.firstScore(), score.lastScore() );
    }

	dg->zero();
}

void Status::restartGame()
{
	field->restart();
	initGame();
}

void Status::newGame(const LevelData &l)
{
	field->setLevel(l);
	initGame();
}

void Status::changeCase(CaseState cs, int inc)
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
	const LevelData &l = field->level();
	int r = l.nbMines - marked;
	int u = l.width*l.height - l.nbMines - uncovered; // cannot be negative
    QColor color = (r<0 && u!=0 ? red : white);
    left->setColor(color);
	left->display(r);
	if ( u==0 ) _endGame(!mine);
}

void Status::_endGame(bool win)
{
	field->stop();
	dg->freeze();
	field->showMines();
	setGameState(Stopped);

	if ( !win ) {
        smiley->setMood(Smiley::Sad);
		emit message(i18n("Bad luck!"));
        return;
	}

    Level level = field->level().level;
    smiley->setMood(Smiley::Happy);
    if ( level!=Custom ) {
        ExtScore score(level, dg->score());
        ExtPlayerInfos infos(level);
        int localRank = infos.submitScore(score, this);
        if ( localRank==-1 ) {
            emit message(i18n("You did it ... but not in time."));
            return;
        }
        ShowHighscores hs(localRank, this, score, infos);
        hs.exec();
    }
    emit message(i18n("Yeeeesssssss!"));
}

void Status::showHighscores(int level)
{
	if ( !field->isPaused() ) field->pause();
    ExtScore score((Level)level);
    ExtPlayerInfos infos((Level)level);
    ShowHighscores hs(-1, this, score, infos);
	hs.exec();
	field->pause();
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
