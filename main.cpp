#include "main.h"

#include <time.h>

#include <qdatetime.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kaboutdialog.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kmenubar.h>

#include "defines.h"
#include "version.h"
#include "status.h"

MainWidget::MainWidget()
{
	installEventFilter(this);
	
	status = new Status(this);
	status->installEventFilter(this);
	setView(status);
	
	kacc = new KAccel(this);
	
	// popup
	popup = new KActionMenu(i18n("&File"), this);
	menuBar()->insertItem(i18n("&File"), popup->popupMenu());
	toggleMenuAction = new KToggleAction(QString::null, 0, this, SLOT(toggleMenu()), this);
	popup->insert(toggleMenuAction);
	popup->insert( new KActionSeparator(this) );
	KAction *action = new KAction(i18n("&New game"), 0, status, SLOT(restartGame()), this);
	action->plugStdAccel(kacc, KStdAccel::New);
	popup->insert(action);
	action = new KAction(i18n("&Pause game"), Key_P, status, SLOT(pauseGame()), this);
	action->plugAccel(kacc, "pause");
	popup->insert(action);
	popup->insert( new KActionSeparator(this) );
	action = new KAction(i18n("&High scores"), Key_H, status, SLOT(showHighScores()), this);
	action->plugAccel(kacc, "highscores");
	popup->insert(action);
	action = new KAction(i18n("P&rint"), 0, status, SLOT(print()), this);
	action->plugStdAccel(kacc, KStdAccel::Print);
	popup->insert(action);
	popup->insert( new KActionSeparator(this) );
	action = new KAction(i18n("&Quit"), 0, qApp, SLOT(quit()), this);
	action->plugStdAccel(kacc, KStdAccel::Quit);
	popup->insert(action);
	
	// keyboard
	action = new KAction(i18n("Move up"), Key_Up, status, SLOT(moveUp()), this);
	action->plugAccel(kacc, "moveup");
	action = new KAction(i18n("Move down"), Key_Down, status, SLOT(moveDown()), this);
	action->plugAccel(kacc, "movedown");
	action = new KAction(i18n("Move left"), Key_Left, status, SLOT(moveLeft()), this);
	action->plugAccel(kacc, "moveleft");
	action = new KAction(i18n("Move right"), Key_Right, status, SLOT(mvoeRight()), this);
	action->plugAccel(kacc, "moveright");
	action = new KAction(i18n("Reveal mine"), Key_Enter, status, SLOT(reveal()), this);
	action->plugAccel(kacc, "revealmine");
	action = new KAction(i18n("Mark mine"), Key_Space, status, SLOT(mark()), this);
	action->plugAccel(kacc, "markmine");
	action = new KAction(i18n("Automatic reveal"), Key_BackSpace, status, SLOT(autoReveal()), this);
	action->plugAccel(kacc, "autoreveal");

	// options
	KActionMenu *options = new KActionMenu(i18n("&Options"), this);
	menuBar()->insertItem(i18n("&Options"), options->popupMenu());
	toggleMarkAction = new KToggleAction(i18n("? &mark"), 0, this, SLOT(toggleUMark()), this);
	options->insert(toggleMarkAction);
	options->insert( new KAction(i18n("&Keys"), 0, this, SLOT(configKeys()), this) );
	
	// levels
	KActionMenu *levels = new KActionMenu(i18n("&Level"), this);
	menuBar()->insertItem(i18n("&Level"), levels->popupMenu());
	levelAction[0] = new KToggleAction(i18n("&Easy"), 0, this, SLOT(easyLevel()), this);
	levelAction[1] = new KToggleAction(i18n("&Normal"), 0, this, SLOT(normalLevel()), this);
	levelAction[2] = new KToggleAction(i18n("&Expert"), 0, this, SLOT(expertLevel()), this);
	levelAction[3] = new KToggleAction(i18n("&Custom"), 0, this, SLOT(customLevel()), this);
	for (uint i=0; i<4; i++) {
		levelAction[i]->setExclusiveGroup("level");
		levels->insert(levelAction[i]);
		if ( i==2) levels->insert(new KActionSeparator(this));
	}
	
	// help
	menuBar()->insertSeparator();
	menuBar()->insertItem(i18n("&Help"), helpMenu());

	// read key settings
	kacc->readSettings();

	// init
	_toggleMenu(TRUE);
	_toggleUMark(TRUE);
	changeLevel(-1);
}

void MainWidget::changeLevel(int lev)
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	if ( lev==-1 ) {
		lev = conf->readUnsignedNumEntry(OP_LEVEL, 0);
		if ( lev<0 || lev>2 ) lev = 0;
	}
	conf->writeEntry(OP_LEVEL, lev);
	if ( !status->newGame(lev) ) return;
	levelAction[lev]->setChecked(TRUE);
}

bool MainWidget::eventFilter(QObject *, QEvent *e)
{
	switch (e->type()) {
	 case QEvent::MouseButtonPress : 
		if ( ((QMouseEvent *)e)->button()!=RightButton ) return FALSE;
		popup->popup(QCursor::pos());
		return TRUE;
	 default : return FALSE;
	}
}

void MainWidget::_toggleMenu(bool first)
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	bool show = conf->readBoolEntry(OP_MENU, TRUE);
	if ( !first ) show = !show;
	conf->writeEntry(OP_MENU, show);
	if (show) menuBar()->show(); else menuBar()->hide();
	toggleMenuAction->setText((show ? i18n("Hide menu") : i18n("Show menu")));
}

void MainWidget::_toggleUMark(bool first)
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	bool um = conf->readBoolEntry(OP_UMARK, TRUE);
	if ( !first ) um = !um;
	conf->writeEntry(OP_UMARK, um);
	toggleMarkAction->setChecked(um);
	status->changeUMark(um);
}

//----------------------------------------------------------------------------
static const char *DESCRIPTION = I18N_NOOP("KMines is a classical mine sweeper game.");

int main( int argc, char ** argv )
{
    KAboutData aboutData("kmines", I18N_NOOP("KMines"),
		QString("%1 (%2/%3/%4)").arg(VERSION).arg(DAY).arg(MONTH).arg(YEAR).latin1(),
		DESCRIPTION, KAboutData::License_GPL, 
        "(c) 1996-2000, Nicolas Hadacek",
		0, "http://azhyd.free.fr/KDE/kmines.php3");
    aboutData.addAuthor("Nicolas Hadacek", 0, "hadacek@kde.org");
    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication a;

    MainWidget *mw = new MainWidget;
    a.setMainWidget(mw);
    mw->show();
    return a.exec();
}
