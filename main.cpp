#include "main.h"
#include "main.moc"

#include <qptrvector.h>

#include <kaccel.h>
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kmenubar.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kstdgameaction.h>
#include <kcmenumngr.h>

#include "status.h"
#include "highscores.h"


MainWidget::MainWidget()
{
	installEventFilter(this);

	status = new Status(this);
	connect(status, SIGNAL(gameStateChanged(GameState)),
			SLOT(gameStateChanged(GameState)));

	// Game & Popup
	KStdGameAction::gameNew(status, SLOT(restartGame()), actionCollection());
	pause = KStdGameAction::pause(status, SLOT(pauseGame()),
                                  actionCollection());
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
	menu = KStdAction::showMenubar(this, SLOT(toggleMenubar()),
                                   actionCollection());
    settings.plug(menu, OP_GROUP, "menubar visible", true);
	KStdAction::preferences(this, SLOT(configureSettings()),
                            actionCollection());
	KStdAction::keyBindings(this, SLOT(configureKeys()), actionCollection());

	// Levels
    levels = new KSelectAction(i18n("Choose &Level"), 0,
                   0, 0, actionCollection(), "levels");
    settings.plug(levels, OP_GROUP, "Level", Level::data(Level::Easy).label);
    connect(levels, SIGNAL(activated(int)), status, SLOT(newGame(int)));
    QStringList list;
    for (uint i=0; i<Level::NbLevels+1; i++) {
        list.append(i18n(Level::data((Level::Type)i).i18nLabel));
        settings.map(levels, i, Level::data((Level::Type)i).label);
    }
    levels->setItems(list);

	createGUI();
	readSettings();
	setCentralWidget(status);

    QPopupMenu *popup =
        static_cast<QPopupMenu *>(factory()->container("popup", this));
    if (popup) KContextMenuManager::insert(this, popup);
}

bool MainWidget::queryExit()
{
    settings.save();
    return true;
}

void MainWidget::readSettings()
{
    settings.load();

    status->newGame( levels->currentItem() );
	toggleMenubar();
    settingsChanged();
}

void MainWidget::showHighscores()
{
    kHighscores->showHighscores(this);
}

bool MainWidget::eventFilter(QObject *, QEvent *e)
{
    if ( e->type()==QEvent::LayoutHint )
		setFixedSize(minimumSize()); // because QMainWindow and KMainWindow
		                             // do not manage fixed central widget and
		                             // hidden menubar ...
    return false;
}

void MainWidget::focusOutEvent(QFocusEvent *e)
{
    if ( pauseFocus && e->reason()==QFocusEvent::ActiveWindow
         && !status->isPaused() ) status->pauseGame();
    KMainWindow::focusOutEvent(e);
}

void MainWidget::toggleMenubar()
{
	if ( menu->isChecked() ) menuBar()->show();
	else menuBar()->hide();
}

void MainWidget::configureSettings()
{
	SettingsDialog od(this);
    connect(&od, SIGNAL(settingsSaved()), SLOT(settingsChanged()));
    od.exec();
}

void MainWidget::settingsChanged()
{
    bool enabled = GameSettings::readKeyboard();
	QValueList<KAction *> list = actionCollection()->actions("keyboard_group");
	QValueList<KAction *>::Iterator it;
	for (it = list.begin(); it!=list.end(); ++it)
		(*it)->setEnabled(enabled);

    pauseFocus = GameSettings::readPauseFocus();

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
		pause->setEnabled(false);
        break;
	case Paused:
		pause->setChecked(true);
		break;
	case Playing:
		pause->setChecked(false);
		pause->setEnabled(true);
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
    KGlobal::locale()->insertCatalogue("libkdehighscores");
    KExtHighscores::ExtHighscores highscores;
    if ( a.isRestored() ) RESTORE(MainWidget)
    else {
        MainWidget *mw = new MainWidget;
        mw->show();
    }
    return a.exec();
}
