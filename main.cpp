#include "main.h"

#include <time.h>

#include <qdatetime.h>

#include <kapp.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kmenubar.h>
#include <khelpmenu.h>
#include <kstdaction.h>

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

	// Popup
	popup = new KActionMenu(i18n("Popup"), this);
	KToggleAction *menuAction
		= KStdAction::showMenubar(this, SLOT(toggleMenubar()),
								  actionCollection());
	popup->insert(menuAction);
	popup->insert( new KActionSeparator(this) );

	// File & Popup
	KAction *action = KStdAction::openNew(status, SLOT(restartGame()),
										  actionCollection(), "game_new");
	popup->insert(action);
	action = new KAction(i18n("&Pause game"), Key_P, status, SLOT(pauseGame()),
						 actionCollection(), "game_pause");
	action->plugAccel(kacc);
	popup->insert(action);
	popup->insert( new KActionSeparator(this) );
	action = new KAction(i18n("&High scores..."), Key_H,
						 status, SLOT(showHighScores()),
						 actionCollection(), "game_highscores");
	action->plugAccel(kacc);
	popup->insert(action);
	action = KStdAction::print(status, SLOT(print()),
							   actionCollection(), "game_print");
	popup->insert(action);
	popup->insert( new KActionSeparator(this) );
	action = KStdAction::quit(qApp, SLOT(quit()),
							  actionCollection(), "game_quit");
	popup->insert(action);

	// keyboard
	keyAction[0] = new KAction(i18n("Move up"), Key_Up, 
							   status, SLOT(moveUp()), this, "moveup");
	keyAction[0]->plugAccel(kacc);
	keyAction[1] = new KAction(i18n("Move down"), Key_Down,
							   status, SLOT(moveDown()), this, "movedown");
	keyAction[1]->plugAccel(kacc);
	keyAction[2] = new KAction(i18n("Move left"), Key_Left,
							   status, SLOT(moveLeft()), this, "moveleft");
	keyAction[2]->plugAccel(kacc);
	keyAction[3] = new KAction(i18n("Move right"), Key_Right,
							   status, SLOT(moveRight()), this, "moveright");
	keyAction[3]->plugAccel(kacc);
	keyAction[4] = new KAction(i18n("Reveal mine"), Key_Shift,
							   status, SLOT(reveal()), this, "revealmine");
	keyAction[4]->plugAccel(kacc);
	keyAction[5] = new KAction(i18n("Mark mine"), Key_Space,
							   status, SLOT(mark()), this, "markmine");
	keyAction[5]->plugAccel(kacc);
	keyAction[6] = new KAction(i18n("Automatic reveal"), Key_Control,
							   status, SLOT(autoReveal()), this, "autoreveal");
	keyAction[6]->plugAccel(kacc);

	// Settings
	KStdAction::preferences(status, SLOT(options()), actionCollection());
	KStdAction::keyBindings(this, SLOT(configKeys()), actionCollection());
	KToggleAction *markAction
		= new KToggleAction(i18n("? &mark"), 0, this, SLOT(toggleUMark()),
							actionCollection(), "options_?_mark");
	KToggleAction *keyboardAction
		= new KToggleAction(i18n("&Enable keyboard"),	0,
							this, SLOT(toggleKeyboard()),
							actionCollection(), "options_enable_keyboard");

	// Levels
	levelAction[0]
		= new KRadioAction(i18n("&Easy"), 0, this, SLOT(easyLevel()),
						   actionCollection(), "level_easy");
	levelAction[1]
		= new KRadioAction(i18n("&Normal"), 0, this, SLOT(normalLevel()),
						   actionCollection(), "level_normal");
	levelAction[2]
		= new KRadioAction(i18n("&Expert"), 0, this, SLOT(expertLevel()),
						   actionCollection(), "level_expert");
	levelAction[3]
		= new KRadioAction(i18n("&Custom..."), 0, this, SLOT(customLevel()),
						   actionCollection(), "level_custom");
	for (uint i=0; i<levelAction.size(); i++)
		levelAction[i]->setExclusiveGroup("level");

	kacc->readSettings();
	enableToolBar(KToolBar::Hide);
	createGUI("ui_kmines.rc");

	// init
	init = TRUE;
	changeLevel(0);
	menuAction->setChecked( toggleMenubar() );
	markAction->setChecked( toggleUMark() );
	keyboardAction->setChecked( toggleKeyboard() );
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

bool MainWidget::toggleMenubar()
{
	bool show = toggle(OP_MENUBAR);
	if (show) menuBar()->show(); else menuBar()->hide();
	return show;
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
		LONG_VERSION, DESCRIPTION, KAboutData::License_GPL, 
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
