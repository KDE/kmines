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
#include <kkeydialog.h>

#include "version.h"
#include "status.h"

MainWidget::MainWidget()
: levelAction(NbLevels)
{
	installEventFilter(this);

	status = new Status(this);
	status->installEventFilter(this);
	connect(status, SIGNAL(keyboardEnabled(bool)),
			SLOT(setKeyboardEnabled(bool)));
	connect(status, SIGNAL(gameStateChanged(GameState)),
			SLOT(gameStateChanged(GameState)));

	// File & Popup
	KStdAction::openNew(status, SLOT(restartGame()),
						actionCollection(), "game_new");
	(void)new KAction(i18n("Pause"), Key_P, status, SLOT(pauseGame()),
					  actionCollection(), "game_pause");
	(void)new KAction(i18n("High Scores..."), Key_H,
					  status, SLOT(showHighScores()),
					  actionCollection(), "game_highscores");
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
	KAccel *kacc = new KAccel(this);
	for (uint i=0; i<keyAction.size(); i++) {
		keyAction[i]->setGroup("keyboard_group");
		keyAction[i]->plugAccel(kacc);
	}

	// Settings
	KStdAction::showMenubar(this, SLOT(toggleMenubar()), actionCollection());
	KStdAction::preferences(status, SLOT(preferences()), actionCollection());
	KStdAction::keyBindings(this, SLOT(configureKeys()), actionCollection());

	// Levels
	levelAction[0]
		= new KRadioAction(i18n("Easy"), 0, this, SLOT(easyLevel()),
						   actionCollection(), "level_easy");
	levelAction[1]
		= new KRadioAction(i18n("Normal"), 0, this, SLOT(normalLevel()),
						   actionCollection(), "level_normal");
	levelAction[2]
		= new KRadioAction(i18n("Expert"), 0, this, SLOT(expertLevel()),
						   actionCollection(), "level_expert");
	levelAction[3]
		= new KRadioAction(i18n("Custom..."), 0, this, SLOT(customLevel()),
						   actionCollection(), "level_custom");
	for (uint i=0; i<levelAction.size(); i++)
		levelAction[i]->setExclusiveGroup("level");

	createGUI();
	readSettings();
	setView(status);
}

#define MENUBAR_ACTION \
    ((KToggleAction *)actionCollection() \
	 ->action(KStdAction::stdName(KStdAction::ShowMenubar)))

#define PAUSE_ACTION \
    ((KToggleAction *)actionCollection()->action("game_pause"))

void MainWidget::readSettings()
{
	GameType type = OptionDialog::readLevel();
	levelAction[type]->setChecked(TRUE);

	bool visible = OptionDialog::readMenuVisible();
	MENUBAR_ACTION->setChecked(visible);

	setKeyboardEnabled( OptionDialog::readKeyboard() );
}

void MainWidget::changeLevel(uint i)
{
	if ( !levelAction[i]->isChecked() ) return;
	GameType lev = (GameType)i;
	if ( !status->newGame(lev) ) { // level unchanged
		levelAction[lev]->setChecked(TRUE);
		return;
	}
	
	OptionDialog::writeLevel(lev);
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
	bool b = MENUBAR_ACTION->isChecked();
	if (b) menuBar()->show();
	else {
		menuBar()->hide();
		// #### sort of hack : because KTMainWindow does not manage correctly
		// main widget with a fixed layout
		updateRects();
		adjustSize();
	}

	OptionDialog::writeMenuVisible(b);
}

void MainWidget::setKeyboardEnabled(bool enable)
{
	QValueList<KAction *> list = actionCollection()->actions("keyboard_group");
	QValueList<KAction *>::Iterator it;
	for (it = list.begin(); it!=list.end(); ++it)
		(*it)->setEnabled(enable);
}

void MainWidget::configureKeys()
{
	KKeyDialog::configureKeys(actionCollection(), xmlFile());
}

void MainWidget::gameStateChanged(GameState s)
{
	switch (s) {
	case Stopped:
		PAUSE_ACTION->setEnabled(FALSE);
        break;
	case Paused:
		PAUSE_ACTION->setText(i18n("&Resume"));
		break;
	case Playing:
		PAUSE_ACTION->setText(i18n("&Pause"));
		PAUSE_ACTION->setEnabled(TRUE);
		break;
    case GameOver:
		PAUSE_ACTION->setEnabled(FALSE);
		break;
	}
}

//----------------------------------------------------------------------------
static const char *DESCRIPTION
    = I18N_NOOP("KMines is a classical mine sweeper game.");

int main(int argc, char **argv)
{
    KAboutData aboutData("kmines", I18N_NOOP("KMines"), LONG_VERSION,
						 DESCRIPTION, KAboutData::License_GPL,
						 COPYLEFT, 0, HOMEPAGE);
    aboutData.addAuthor("Nicolas Hadacek", 0, "hadacek@kde.org");
	aboutData.addCredit("Andreas Zehender", "Smiley pixmaps");
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication a;
    MainWidget *mw = new MainWidget;
    a.setMainWidget(mw);
    mw->show();
    return a.exec();
}
