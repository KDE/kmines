#include "main.h"
#include "main.moc"

#include <qcstring.h>
#include <qwhatsthis.h>

#include <kaccel.h>
#include <kapplication.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kmenubar.h>
#include <khelpmenu.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kstatusbar.h>
#include <kstdgameaction.h>

#include "status.h"
#include "highscores.h"


ExtHighscores HIGHSCORES;
Highscores &highscores() { return HIGHSCORES; }


MainWidget::MainWidget()
    : KMainWindow(0)
{
    HIGHSCORES.init();

	installEventFilter(this);

	status = new Status(this);
	status->installEventFilter(this);
	connect(status, SIGNAL(gameStateChanged(GameState)),
			SLOT(gameStateChanged(GameState)));

	// Game & Popup
	KStdGameAction::gameNew(status, SLOT(restartGame()), actionCollection());
	KStdGameAction::pause(status, SLOT(pauseGame()), actionCollection());
	KStdGameAction::highscores(this, SLOT(showHighscores()),
                                   actionCollection());
	KStdGameAction::print(status, SLOT(print()), actionCollection());
	KStdGameAction::quit(qApp, SLOT(quit()), actionCollection());

	// keyboard
	QPtrVector<KAction> keyAction(7);
	keyAction.insert(0, new KAction(i18n("Move up"), Key_Up,
							   status, SLOT(moveUp()),
							   actionCollection(), "keyboard_moveup"));
	keyAction.insert(1, new KAction(i18n("Move down"), Key_Down,
							   status, SLOT(moveDown()),
							   actionCollection(), "keyboard_movedown"));
	keyAction.insert(2, new KAction(i18n("Move left"), Key_Left,
							   status, SLOT(moveLeft()),
							   actionCollection(), "keyboard_moveleft"));
	keyAction.insert(3, new KAction(i18n("Move right"), Key_Right,
							   status, SLOT(moveRight()),
							   actionCollection(), "keyboard_moveright"));
	keyAction.insert(4, new KAction(i18n("Reveal mine"), Key_Space,
							   status, SLOT(reveal()),
							   actionCollection(), "keyboard_revealmine"));
	keyAction.insert(5, new KAction(i18n("Mark mine"), Key_W,
							   status, SLOT(mark()),
							   actionCollection(), "keyboard_markmine"));
	keyAction.insert(6, new KAction(i18n("Automatic reveal"), Key_Return,
							   status, SLOT(autoReveal()),
							   actionCollection(), "keyboard_autoreveal"));
	KAccel *kacc = new KAccel(this);
	for (uint i=0; i<keyAction.size(); i++) {
		keyAction[i]->setGroup("keyboard_group");
		keyAction[i]->plugAccel(kacc);
	}

	// Settings
	KStdAction::showMenubar(this, SLOT(toggleMenubar()), actionCollection());
	KStdAction::preferences(this, SLOT(configureSettings()),
                            actionCollection());
	KStdAction::keyBindings(this, SLOT(configureKeys()), actionCollection());

	// Levels
    levelAction.resize(Level::NbLevels+1);
    for (uint i=0; i<Level::NbLevels+1; i++) {
        levelAction.insert(i,
              new KRadioAction(i18n(Level::data((Level::Type)i).i18nLabel), 0,
                               this, SLOT(changeLevel()), actionCollection(),
                               Level::actionName((Level::Type)i)));
        levelAction[i]->setExclusiveGroup("level");
    }

	createGUI();
	readSettings();
	setCentralWidget(status);
}

#define MENUBAR_ACTION \
    ((KToggleAction *)action(KStdAction::stdName(KStdAction::ShowMenubar)))

#define PAUSE_ACTION ((KToggleAction *)action("game_pause"))

void MainWidget::readSettings()
{
	Level level = SettingsDialog::readLevel();
	if ( level.type()!=Level::Custom )
        levelAction[level.type()]->setChecked(true);
	status->newGame(level);

	bool visible = SettingsDialog::readMenuVisible();
	MENUBAR_ACTION->setChecked(visible);
	toggleMenubar();

    settingsChanged();
}

void MainWidget::changeLevel()
{
    Level::Type type = Level::Easy;
    for (uint i=0; i<levelAction.size(); i++)
        if ( levelAction[i]==sender() ) {
            type = (Level::Type)i;
            break;
        }

	if ( !levelAction[type]->isChecked() ) return;

	Level level = (type==Level::Custom ? status->currentLevel() : Level(type));
	if ( type==Level::Custom ) {
		levelAction[Level::Custom]->setChecked(false);
        type = level.type();
		CustomDialog cu(level, this);
		bool res = cu.exec();
        if ( level.type()!=Level::Custom ) {
            levelAction[type]->setChecked(true);
            if ( level.type()==type ) return; // level unchanged
        }
        if ( !res ) return; // canceled
	}

	status->newGame(level);
	SettingsDialog::writeLevel(level);
}

void MainWidget::showHighscores()
{
    HIGHSCORES.showHighscores(this);
}

bool MainWidget::eventFilter(QObject *, QEvent *e)
{
	QPopupMenu *popup;
	switch (e->type()) {
	case QEvent::MouseButtonPress :
		if ( ((QMouseEvent *)e)->button()!=RightButton ) return false;
		popup = (QPopupMenu*)factory()->container("popup", this);
		if ( popup ) popup->popup(QCursor::pos());
		return true;
	case QEvent::LayoutHint:
		setFixedSize(minimumSize()); // because QMainWindow and KMainWindow
		                             // do not manage fixed central widget and
		                             // hidden menubar ...
		return false;
	default : return false;
	}
}

void MainWidget::focusOutEvent(QFocusEvent *e)
{
    if ( pauseFocus && e->reason()==QFocusEvent::ActiveWindow
         && !status->isPaused() ) status->pauseGame();
    KMainWindow::focusOutEvent(e);
}

void MainWidget::toggleMenubar()
{
	bool b = MENUBAR_ACTION->isChecked();
	if (b) menuBar()->show();
	else menuBar()->hide();
	SettingsDialog::writeMenuVisible(b);
}

void MainWidget::configureSettings()
{
	SettingsDialog od(this);
	if ( !od.exec() ) return;
    settingsChanged();
}

void MainWidget::settingsChanged()
{
    bool enabled = GameSettingsWidget::readKeyboard();
	QValueList<KAction *> list = actionCollection()->actions("keyboard_group");
	QValueList<KAction *>::Iterator it;
	for (it = list.begin(); it!=list.end(); ++it)
		(*it)->setEnabled(enabled);

    pauseFocus = GameSettingsWidget::readPauseFocus();

    status->settingsChanged();
}

void MainWidget::configureKeys()
{
	KKeyDialog::configureKeys(actionCollection(), xmlFile(), true, this);
}

void MainWidget::gameStateChanged(GameState s)
{
	switch (s) {
	case Stopped:
		PAUSE_ACTION->setEnabled(false);
        break;
	case Paused:
		PAUSE_ACTION->setChecked(true);
		break;
	case Playing:
		PAUSE_ACTION->setChecked(false);
		PAUSE_ACTION->setEnabled(true);
        setFocus();
		break;
	}
}

//----------------------------------------------------------------------------
static const char *DESCRIPTION
    = I18N_NOOP("KMines is a classic mine sweeper game.");

int main(int argc, char **argv)
{
    KAboutData aboutData("kmines", I18N_NOOP("KMines"), LONG_VERSION,
						 DESCRIPTION, KAboutData::License_GPL,
						 COPYLEFT, 0, HOMEPAGE);
    aboutData.addAuthor("Nicolas Hadacek", 0, EMAIL);
	aboutData.addCredit("Andreas Zehender", I18N_NOOP("Smiley pixmaps"));
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication a;
    KGlobal::locale()->insertCatalogue("libkdegames");
    if ( a.isRestored() ) RESTORE(MainWidget)
    else {
        MainWidget *mw = new MainWidget;
        mw->show();
    }
    return a.exec();
}
