#include "status.h"
#include "status.moc"

#include <qpainter.h>
#include <qpixmap.h>
#include <qwhatsthis.h>
#include <qlayout.h>

#include <kapplication.h>
#include <klocale.h>
#include <kconfig.h>
#include "ghighscores.h"


Status::Status(QWidget *parent, const char *name)
: QWidget(parent, name)
{
// top layout
	QGridLayout *top = new QGridLayout(this, 2, 5, 10, 10);
    top->setResizeMode(QLayout::Fixed);
    top->setColStretch(1, 1);
    top->setColStretch(3, 1);

// status bar
	// mines left LCD
	left = new LCD(5, this);
    left->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    left->setDefaultColors(white, black);
	QWhatsThis::add(left, i18n("<qt>Mines left.<br/>"
                               "It turns <font color=\"red\">red</font> "
                               "when you have flagged more cases than "
                               "present mines.</qt>"));
    top->addWidget(left, 0, 0);

	// smiley
	smiley = new Smiley(this);
	connect(smiley, SIGNAL(clicked()), SLOT(smileyClicked()));
	smiley->setFocusPolicy(QWidget::NoFocus);
	QWhatsThis::add(smiley, i18n("Press to start a new game"));
    top->addWidget(smiley, 0, 2);

	// digital clock LCD
	dg = new DigitalClock(this);
	QWhatsThis::add(dg, i18n("<qt>Time elapsed.<br/>"
                             "It turns <font color=\"blue\">blue</font> "
                             "if it is a highscore "
                             "and <font color=\"red\">red</font> "
                             "if it is the best time.</qt>"));
    top->addWidget(dg, 0, 4);

// mines field
    _fieldContainer = new QWidget(this);
    QGridLayout *g = new QGridLayout(_fieldContainer, 1, 1);
    field = new Field(_fieldContainer);
    g->addWidget(field, 0, 0, AlignCenter);
    connect( field, SIGNAL(changeCase(CaseState, int)),
			 SLOT(changeCase(CaseState, int)) );
	connect( field, SIGNAL(updateStatus(bool)), SLOT(update(bool)) );
	connect( field, SIGNAL(gameLost()), SLOT(gameLost()) );
	connect( field, SIGNAL(startTimer()), dg, SLOT(start()) );
	connect( field, SIGNAL(stopTimer()), dg, SLOT(stop()) );
	connect( field, SIGNAL(setMood(Smiley::Mood)),
			 smiley, SLOT(setMood(Smiley::Mood)) );
	connect(field, SIGNAL(gameStateChanged(GameState)),
			SLOT(gameStateChangedSlot(GameState)) );
    connect(field, SIGNAL(incActions()), dg, SLOT(incActions()));
	QWhatsThis::add(field, i18n("Mines field."));

// resume button
    _resumeContainer = new QWidget(this);
    g = new QGridLayout(_resumeContainer, 1, 1);
    QFont f = font();
    f.setBold(true);
    QPushButton *pb
        = new QPushButton(i18n("Press to Resume"), _resumeContainer);
    pb->setFont(f);
    connect(pb, SIGNAL(clicked()), field, SLOT(resume()));
    g->addWidget(pb, 0, 0, AlignCenter);

    _stack = new QWidgetStack(this);
    _stack->addWidget(_fieldContainer);
    _stack->addWidget(_resumeContainer);
    _stack->raiseWidget(_fieldContainer);
    top->addMultiCellWidget(_stack, 1, 1, 0, 4);
}

void Status::initGame()
{
	uncovered = 0;
	uncertain = 0;
	marked    = 0;
	gameStateChangedSlot(Stopped);
	update(false);
	smiley->setMood(Smiley::Normal);

    KExtHighscores::Score first(KExtHighscores::Won);
    KExtHighscores::Score last(KExtHighscores::Won);
	if ( field->level().type()!=Level::Custom ) {
        first = kHighscores->firstScore();
        last = kHighscores->lastScore();
    }
	dg->reset(first, last);
}

void Status::smileyClicked()
{
    if ( field->isPaused() ) field->resume();
    else restartGame();
}

void Status::restartGame()
{
	field->restart();
	initGame();
}

void Status::newGame(int t)
{
    Level::Type type = (Level::Type)t;
    if ( type!=Level::Custom ) {
        kHighscores->setGameType(type);
        field->setLevel(Level(type));
    } else field->setLevel(CustomSettings::readLevel());

	initGame();
    updateGeometry();
}

void Status::settingsChanged()
{
    field->readSettings();

    Level current = field->level();
    if ( current.type()!=Level::Custom ) return;
    Level l = CustomSettings::readLevel();
    if ( l.width()==current.width() && l.height()==current.height()
         && l.nbMines()==current.nbMines() ) return;
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
	const Level &level = field->level();
	int r = level.nbMines() - marked;
	int u = level.width()*level.height()
            - level.nbMines() - uncovered; // cannot be negative
    QColor color = (r<0 && u!=0 ? red : white);
    left->setColor(color);
	left->display(r);

	if ( u==0 && !mine ) _endGame(true); // ends only for wins
}

void Status::_endGame(bool won)
{
    field->showMines();
	field->stop();
	dg->stop();
	emit gameStateChanged(Stopped);
    smiley->setMood(won ? Smiley::Happy : Smiley::Sad);

    if ( field->level().type()==Level::Custom || !won ) return;
    kHighscores->submitScore(dg->score(), this);
}

void Status::gameStateChangedSlot(GameState state)
{
    if ( state==Paused ) _stack->raiseWidget(_resumeContainer);
    else _stack->raiseWidget(_fieldContainer);
    emit gameStateChanged(state);
}
