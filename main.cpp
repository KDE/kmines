#include "main.h"
#include "main.moc"

#include <qcstring.h>
#include <qwhatsthis.h>

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
#include <kstatusbar.h>
#include <kstdgameaction.h>

#include "version.h"
#include "status.h"


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
	keyAction.insert(5, new KAction(i18n("Mark mine"), Key_Control,
							   status, SLOT(mark()),
							   actionCollection(), "keyboard_markmine"));
	keyAction.insert(6, new KAction(i18n("Automatic reveal"), Key_Shift,
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
    levelAction.resize(NbLevels+1);
    QCString name("level_");
    for (uint i=0; i<NbLevels+1; i++) {
        levelAction.insert(i, new KRadioAction(i18n(LEVELS[i].i18nLabel), 0,
                         this, SLOT(changeLevel()), actionCollection(),
                         name + LEVELS[i].label));
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
	LevelData l = SettingsDialog::readLevel();
	if ( l.level!=Custom ) levelAction[l.level]->setChecked(true);
	status->newGame(l);

	bool visible = SettingsDialog::readMenuVisible();
	MENUBAR_ACTION->setChecked(visible);
	toggleMenubar();

    settingsChanged();
}

void MainWidget::changeLevel()
{
    Level level = Easy;
    for (uint i=0; i<levelAction.size(); i++)
        if ( levelAction[i]==sender() ) {
            level = (Level)i;
            break;
        }

	if ( !levelAction[level]->isChecked() ) return;

	LevelData l;
	if ( level==Custom ) {
		levelAction[Custom]->setChecked(false);
		l = status->currentLevel();
		level = l.level;
		CustomDialog cu(l, this);
		if ( !cu.exec() ) { // level unchanged
			if ( level!=Custom ) levelAction[level]->setChecked(true);
			return;
		}
	} else l = LEVELS[level];

	status->newGame(l);
	SettingsDialog::writeLevel(l);
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
