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
: keyAction(7), levelAction(NbLevels)
{
	installEventFilter(this);
	
	status = new Status(this);
	status->installEventFilter(this);
	
	kacc = new KAccel(this);
	
	// popup
	popup = new KActionMenu(i18n("&File"), this);
	menuBar()->insertItem(i18n("&File"), popup->popupMenu());
	menuAction = new KToggleAction(QString::null, 0,
								   this, SLOT(toggleMenu()), this);
	popup->insert(menuAction);
	popup->insert( new KActionSeparator(this) );
	KAction *action = new KAction(i18n("&New game"), 0,
								  status, SLOT(restartGame()), this);
	action->plugStdAccel(kacc, KStdAccel::New);
	popup->insert(action);
	action = new KAction(i18n("&Pause game"), Key_P,
						 status, SLOT(pauseGame()), this);
	action->plugAccel(kacc, "pause");
	popup->insert(action);
	popup->insert( new KActionSeparator(this) );
	action = new KAction(i18n("&High scores"), Key_H,
						 status, SLOT(showHighScores()), this);
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
	keyAction[0] = new KAction(i18n("Move up"), Key_Up, 
							   status, SLOT(moveUp()), this);
	keyAction[0]->plugAccel(kacc, "moveup");
	keyAction[1] = new KAction(i18n("Move down"), Key_Down,
							   status, SLOT(moveDown()), this);
	keyAction[1]->plugAccel(kacc, "movedown");
	keyAction[2] = new KAction(i18n("Move left"), Key_Left,
							   status, SLOT(moveLeft()), this);
	keyAction[2]->plugAccel(kacc, "moveleft");
	keyAction[3] = new KAction(i18n("Move right"), Key_Right,
							   status, SLOT(moveRight()), this);
	keyAction[3]->plugAccel(kacc, "moveright");
	keyAction[4] = new KAction(i18n("Reveal mine"), Key_Shift,
							   status, SLOT(reveal()), this);
	keyAction[4]->plugAccel(kacc, "revealmine");
	keyAction[5] = new KAction(i18n("Mark mine"), Key_Space,
							   status, SLOT(mark()), this);
	keyAction[5]->plugAccel(kacc, "markmine");
	keyAction[6] = new KAction(i18n("Automatic reveal"), Key_Control,
							   status, SLOT(autoReveal()), this);
	keyAction[6]->plugAccel(kacc, "autoreveal");

	// options
	KActionMenu *options = new KActionMenu(i18n("&Settings"), this);
	menuBar()->insertItem(i18n("&Settings"), options->popupMenu());
	QArray<KToggleAction *> toggleAction(2);
	toggleAction[0] = new KToggleAction(i18n("? &mark"), 0,
										this, SLOT(toggleUMark()), this);
	options->insert(toggleAction[0]);
	toggleAction[1] = new KToggleAction(i18n("keyboard play"), 0,
										this, SLOT(toggleKeyboard()), this);
	options->insert(toggleAction[1]);
	action = new KAction(i18n("Other settings"), 0,
						 status, SLOT(options()), this);
	options->insert(action);
	action = new KAction(i18n("&Keys"), 0, this, SLOT(configKeys()), this);
	options->insert(action);
	
	// levels
	KActionMenu *levels = new KActionMenu(i18n("&Level"), this);
	menuBar()->insertItem(i18n("&Level"), levels->popupMenu());
	levelAction[0] = new KRadioAction(i18n("&Easy"), 0,
									  this, SLOT(easyLevel()), this);
	levelAction[1] = new KRadioAction(i18n("&Normal"), 0,
									  this, SLOT(normalLevel()), this);
	levelAction[2] = new KRadioAction(i18n("&Expert"), 0,
									  this, SLOT(expertLevel()), this);
	levelAction[3] = new KRadioAction(i18n("&Custom"), 0,
									  this, SLOT(customLevel()), this);
	for (uint i=0; i<levelAction.size(); i++) {
		levelAction[i]->setExclusiveGroup("level");
		levels->insert(levelAction[i]);
		if ( (GameType)i==Custom-1) levels->insert(new KActionSeparator(this));
	}

	// help
	menuBar()->insertSeparator();
	menuBar()->insertItem(i18n("&Help"), helpMenu());

	// read key settings
	kacc->readSettings();

	// init
	init = TRUE;
	toggleMenu();
	changeLevel(0);
	toggleAction[0]->setChecked( toggleUMark() );
	toggleAction[1]->setChecked( toggleKeyboard() );
	init = FALSE;

	setView(status);
}

void MainWidget::changeLevel(uint i)
{
	if ( !init && !levelAction[i]->isChecked() ) return;
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	GameType lev;
	if (init) {
		lev = (GameType)conf->readUnsignedNumEntry(OP_LEVEL, 0);
		if ( lev>=Custom ) lev = Easy;
		levelAction[lev]->setChecked(TRUE);
	} else {
		lev = (GameType)i;
		if ( lev<Custom ) conf->writeEntry(OP_LEVEL, (uint)lev);
	}

	if ( !status->newGame(lev) ) levelAction[lev]->setChecked(TRUE);
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

bool MainWidget::toggle(const char *name)
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	bool res = conf->readBoolEntry(name, TRUE);
	if ( !init ) {
		res = !res;
		conf->writeEntry(name, res);
	}
	return res;
}

void MainWidget::toggleMenu()
{
	bool show = toggle(OP_MENU);
	if (show) menuBar()->show(); else menuBar()->hide();
	menuAction->setText((show ? i18n("Hide menu") : i18n("Show menu")));
}

bool MainWidget::toggleUMark()
{
	bool mark = toggle(OP_UMARK);
	status->setUMark(mark);
	return mark;
}

bool MainWidget::toggleKeyboard()
{
	bool keyb = toggle(OP_KEYBOARD);
	for (uint i=0; i<keyAction.size(); i++) keyAction[i]->setEnabled(keyb);
	status->setCursor(keyb);
	return keyb;
}

//----------------------------------------------------------------------------
static const char *DESCRIPTION
    = I18N_NOOP("KMines is a classical mine sweeper game.");

int main(int argc, char **argv)
{
    KAboutData aboutData("kmines", I18N_NOOP("KMines"),
		QString("%1 (%2/%3/%4)").arg(VERSION).arg(DAY).arg(MONTH).
		arg(YEAR).latin1(), DESCRIPTION, KAboutData::License_GPL, 
        "(c) 1996-2000, Nicolas Hadacek",
		0, "http://azhyd.free.fr/KDE/kmines.php3");
    aboutData.addAuthor("Nicolas Hadacek", 0, "hadacek@kde.org");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication a;
    MainWidget *mw = new MainWidget;
    a.setMainWidget(mw);
    mw->show();
    return a.exec();
}
