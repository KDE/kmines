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

#define MENUBAR_ACTION \
    ((KToggleAction *)actionCollection() \
	 ->action(KStdAction::stdName(KStdAction::ShowMenubar)))
#define SAVE_SETTINGS_ACTION \
    actionCollection()->action(KStdAction::stdName(KStdAction::SaveOptions))
#define UMARK_ACTION \
    ((KToggleAction *)actionCollection()->action("options_?_mark"))
#define KEYBOARD_ACTION \
    ((KToggleAction *)actionCollection()->action("options_enable_keyboard"))

MainWidget::MainWidget()
: levelAction(NbLevels)
{
	installEventFilter(this);

	status = new Status(this);
	status->installEventFilter(this);
	kacc = new KAccel(this);

	// File & Popup
	KStdAction::openNew(status, SLOT(restartGame()),
						actionCollection(), "game_new");
	KAction *action
		= new KAction(i18n("&Pause"), Key_P, status, SLOT(pauseGame()),
					  actionCollection(), "game_pause");
	action->plugAccel(kacc);
	action = new KAction(i18n("&High scores..."), Key_H,
						 status, SLOT(showHighScores()),
						 actionCollection(), "game_highscores");
	action->plugAccel(kacc);
	KStdAction::print(status, SLOT(print()), actionCollection(), "game_print");
	KStdAction::quit(qApp, SLOT(quit()), actionCollection(), "game_quit");

	// keyboard
	QArray<KAction *> keyAction(7);
	keyAction[0] = new KAction(i18n("Move up"), Key_Up, 
							   status, SLOT(moveUp()),
							   actionCollection(), "keyboard_moveup");
	keyAction[1] = new KAction(i18n("Move down"), Key_Down,
							   status, SLOT(moveDown()),
							   actionCollection(), "keyboard_movedown");
	keyAction[2] = new KAction(i18n("Move left"), Key_Left,
							   status, SLOT(moveLeft()),
							   actionCollection(), "keyboard_moveleft");
	keyAction[3] = new KAction(i18n("Move right"), Key_Right,
							   status, SLOT(moveRight()),
							   actionCollection(), "keyboard_moveright");
	keyAction[4] = new KAction(i18n("Reveal mine"), Key_Shift,
							   status, SLOT(reveal()),
							   actionCollection(), "keyboard_revealmine");
	keyAction[5] = new KAction(i18n("Mark mine"), Key_Space,
							   status, SLOT(mark()),
							   actionCollection(), "keyboard_markmine");
	keyAction[6] = new KAction(i18n("Automatic reveal"), Key_Control,
							   status, SLOT(autoReveal()),
							   actionCollection(), "keyboard_autoreveal");
	for (uint i=0; i<keyAction.size(); i++) {
		keyAction[i]->setGroup("keyboard_group");
		keyAction[i]->plugAccel(kacc);
		keyAction[i]->setEnabled(FALSE);
	}

	// Settings
	KStdAction::showMenubar(this, SLOT(toggleMenubar()), actionCollection());
	KStdAction::saveOptions(this, SLOT(saveSettings()), actionCollection());
	KStdAction::preferences(status, SLOT(options()), actionCollection());
	KStdAction::keyBindings(this, SLOT(configKeys()), actionCollection());
	(void)new KToggleAction(i18n("? &mark"), 0, this, SLOT(toggleUMark()),
							actionCollection(), "options_?_mark");
	(void)new KToggleAction(i18n("&Enable keyboard"),	0,
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

	createGUI("ui_kmines.rc");
	readSettings();
	setView(status);
}

void MainWidget::readSettings()
{
	kacc->readSettings();

	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	GameType lev = (GameType)conf->readUnsignedNumEntry(OP_LEVEL, 0);
	if ( lev>=Custom ) lev = Easy;
	bool visible = conf->readBoolEntry(OP_MENUBAR, TRUE);
	bool umark = conf->readBoolEntry(OP_UMARK, TRUE);
	bool keyboard = conf->readBoolEntry(OP_KEYBOARD, TRUE);

	levelAction[lev]->setChecked(TRUE);
	MENUBAR_ACTION->setChecked(visible);
	UMARK_ACTION->setChecked(umark);
	KEYBOARD_ACTION->setChecked(keyboard);
}

void MainWidget::saveSettings()
{
	KConfig *conf = kapp->config();
	conf->setGroup(OP_GRP);
	uint i = 0;
	for (; i<levelAction.size(); i++)
		if ( levelAction[i]->isChecked() ) break;
	if ( (GameType)i<Custom ) conf->writeEntry(OP_LEVEL, i);
	conf->writeEntry(OP_MENUBAR, MENUBAR_ACTION->isChecked());
	conf->writeEntry(OP_UMARK, UMARK_ACTION->isChecked());
	conf->writeEntry(OP_KEYBOARD, KEYBOARD_ACTION->isChecked());
}

void MainWidget::changeLevel(uint i)
{
	if ( !levelAction[i]->isChecked() ) return;
	GameType lev = (GameType)i;
	if ( !status->newGame(lev) ) levelAction[lev]->setChecked(TRUE);
}

bool MainWidget::eventFilter(QObject *, QEvent *e)
{
	QPopupMenu *popup;
	switch (e->type()) {
	 case QEvent::MouseButtonPress : 
		if ( ((QMouseEvent *)e)->button()!=RightButton ) return FALSE;
		popup = (QPopupMenu*)factory()->container("popup", this);
		popup->popup(QCursor::pos());
		return TRUE;
	 default : return FALSE;
	}
}

void MainWidget::toggleMenubar()
{
	if ( MENUBAR_ACTION->isChecked() ) menuBar()->show();
	else menuBar()->hide();
}

void MainWidget::toggleUMark()
{
	status->setUMark(UMARK_ACTION->isChecked());
}

void MainWidget::toggleKeyboard()
{
	bool keyboard = KEYBOARD_ACTION->isChecked();
	QValueList<KAction *> list = actionCollection()->actions("keyboard_group");
	QValueList<KAction *>::Iterator it;
	for (it = list.begin(); it!=list.end(); ++it)
		(*it)->setEnabled(keyboard);
	status->setCursor(keyboard);
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
	aboutData.addCredit("Andreas Zehender", "Smiley pixmaps");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication a;
    MainWidget *mw = new MainWidget;
    a.setMainWidget(mw);
    mw->show();
    return a.exec();
}
