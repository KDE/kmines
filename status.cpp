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


Status::Status(QWidget *parent, const char *name)
: QWidget(parent, name)
{
// top layout
	QVBoxLayout *top = new QVBoxLayout(this, 10);
	top->setResizeMode(QLayout::Fixed);

// status bar
	QHBoxLayout *hbl = new QHBoxLayout(top);

	// mines left LCD
	left = new LCDNumber(this);
	left->installEventFilter(parent);
	QWhatsThis::add(left, i18n("<qt>Mines left.<br/>"
                               "It turns <font color=\"red\">red</font> "
                               "when you have flagged more cases than "
                               "present mines.</qt>"));
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
	QWhatsThis::add(dg, i18n("<qt>Time elapsed.<br/>"
                             "It turns <font color=\"blue\">blue</font> "
                             "if it is a highscore "
                             "and <font color=\"red\">red</font> "
                             "if it is the best time.</qt>"));
	hbl->addWidget(dg);

// mines field
    hbl = new QHBoxLayout(top);
    hbl->addStretch(1);

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
			SIGNAL(gameStateChanged(GameState)) );
	QWhatsThis::add(field, i18n("Mis field."));
	hbl->addWidget(field);
    hbl->addStretch(1);
}

void Status::initGame()
{
	uncovered = 0;
	uncertain = 0;
	marked    = 0;
	emit gameStateChanged(Stopped);
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
	emit gameStateChanged(Stopped);

	if ( !win ) {
        smiley->setMood(Smiley::Sad);
        return;
	}

    Level level = field->level().level;
    smiley->setMood(Smiley::Happy);
    if ( level!=Custom ) {
        ExtScore score(level, dg->score());
        ExtPlayerInfos infos(level);
        int localRank = infos.submitScore(score, this);
        if ( localRank==-1 ) return;
        ShowHighscores hs(localRank, this, score, infos);
        hs.exec();
    }
}

void Status::showHighscores(int level)
{
    ExtScore score((Level)level);
    ExtPlayerInfos infos((Level)level);
    ShowHighscores hs(-1, this, score, infos);
	hs.exec();
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
