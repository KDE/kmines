/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

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
#include <kaction.h>
#include <kdebug.h>
#include <knotifyclient.h>
#include <knotifydialog.h>

#include "status.h"
#include "highscores.h"
#include "version.h"
#include "dialogs.h"

const MainWidget::KeyData MainWidget::KEY_DATA[NB_KEYS] = {
{I18N_NOOP("Move Up"),     "keyboard_moveup",    Key_Up,    SLOT(moveUp())},
{I18N_NOOP("Move Down"),   "keyboard_movedown",  Key_Down,  SLOT(moveDown())},
{I18N_NOOP("Move Right"),  "keyboard_moveright", Key_Right, SLOT(moveRight())},
{I18N_NOOP("Move Left"),   "keyboard_moveleft",  Key_Left,  SLOT(moveLeft())},
{I18N_NOOP("Move at Left Edge"), "keyboard_leftedge", Key_Home, SLOT(moveLeftEdge())},
{I18N_NOOP("Move at Right Edge"), "keyboard_rightedge", Key_End, SLOT(moveRightEdge())},
{I18N_NOOP("Move at Top Edge"), "keyboard_topedge", Key_PageUp, SLOT(moveTop())},
{I18N_NOOP("Move at Bottom Edge"), "keyboard_bottomedge", Key_PageDown, SLOT(moveBottom())},
{I18N_NOOP("Reveal Mine"), "keyboard_revealmine", Key_Space, SLOT(reveal())},
{I18N_NOOP("Mark Mine"),   "keyboard_markmine",  Key_W,     SLOT(mark())},
{I18N_NOOP("Automatic Reveal"), "keyboard_autoreveal", Key_Return, SLOT(autoReveal())}
};


MainWidget::MainWidget()
{
    KNotifyClient::startDaemon();
	installEventFilter(this);
    KConfigCollection::init();

	_status = new Status(this);
	connect(_status, SIGNAL(gameStateChangedSignal(KMines::GameState)),
			SLOT(gameStateChanged(KMines::GameState)));
    connect(_status, SIGNAL(pause()), SLOT(pause()));

	// Game & Popup
	KStdGameAction::gameNew(_status, SLOT(restartGame()), actionCollection());
	_pause = KStdGameAction::pause(_status, SLOT(pauseGame()),
                                  actionCollection());
	KStdGameAction::highscores(this, SLOT(showHighscores()),
                               actionCollection());
	KStdGameAction::quit(qApp, SLOT(quit()), actionCollection());

	// keyboard
    _keybCollection = new KActionCollection(this);
    for (uint i=0; i<NB_KEYS; i++) {
        const KeyData &d = KEY_DATA[i];
        (void)new KAction(i18n(d.label), d.keycode, _status,
                          d.slot, _keybCollection, d.name);
    }

	// Settings
	_menu = KStdAction::showMenubar(this, SLOT(toggleMenubar()),
                                   actionCollection());
    _configCollection.plug("menubar visible", _menu);
	KStdAction::preferences(this, SLOT(configureSettings()),
                            actionCollection());
	KStdAction::keyBindings(this, SLOT(configureKeys()), actionCollection());
    KStdAction::configureNotifications(this, SLOT(configureNotifications()),
                                       actionCollection());
    KStdGameAction::configureHighscores(this, SLOT(configureHighscores()),
                                        actionCollection());

	// Levels
    _levels = KStdGameAction::chooseGameType(0, 0, actionCollection());
    connect(_levels, SIGNAL(activated(int)), _status, SLOT(newGame(int)));
    _configCollection.plug("level", _levels);

    // Adviser
    _advise =
        KStdGameAction::hint(_status, SLOT(advise()), actionCollection());
    _solve = KStdGameAction::solve(_status, SLOT(solve()), actionCollection());
    (void)new KAction(i18n("Solving Rate..."), 0, _status, SLOT(solveRate()),
                      actionCollection(), "solve_rate");

    // Log
    (void)new KAction(KGuiItem(i18n("View Log"), "viewmag"), 0,
                      _status, SLOT(viewLog()),
                      actionCollection(), "log_view");
    (void)new KAction(KGuiItem(i18n("Replay Log"), "player_play"),
                      0, _status, SLOT(replayLog()),
                      actionCollection(), "log_replay");
    (void)new KAction(KGuiItem(i18n("Save Log..."), "filesave"), 0,
                      _status, SLOT(saveLog()),
                      actionCollection(), "log_save");
    (void)new KAction(KGuiItem(i18n("Load Log..."), "fileopen"), 0,
                      _status, SLOT(loadLog()),
                      actionCollection(), "log_load");

	createGUI();
	readSettings();
	setCentralWidget(_status);

    QPopupMenu *popup =
        static_cast<QPopupMenu *>(factory()->container("popup", this));
    if (popup) KContextMenuManager::insert(this, popup);
}

MainWidget::~MainWidget()
{
    KConfigCollection::cleanUp();
}

bool MainWidget::queryExit()
{
    _status->checkBlackMark();
    _configCollection.save();
    return true;
}

void MainWidget::readSettings()
{
    _configCollection.load();
    _status->newGame( _levels->currentItem() );
	toggleMenubar();
    settingsChanged();
}

void MainWidget::showHighscores()
{
    KExtHighscore::show(this);
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
    if (  _pauseIfFocusLost && e->reason()==QFocusEvent::ActiveWindow
          && _status->isPlaying() ) pause();
    KMainWindow::focusOutEvent(e);
}

void MainWidget::toggleMenubar()
{
	if ( _menu->isChecked() ) menuBar()->show();
	else menuBar()->hide();
}

void MainWidget::configureSettings()
{
    KConfigDialog d(this);
    d.setIconListAllVisible(true);
    d.append(new GameConfig);
    d.append(new AppearanceConfig);
    d.append(new CustomConfig);
    connect(&d, SIGNAL(saved()), SLOT(settingsChanged()));
    d.exec();
}

void MainWidget::configureHighscores()
{
    KExtHighscore::configure(this);
}

void MainWidget::settingsChanged()
{
    bool enabled = KConfigCollection::configValue("keyboard game").toBool();
	QValueList<KAction *> list = _keybCollection->actions();
	QValueList<KAction *>::Iterator it;
	for (it = list.begin(); it!=list.end(); ++it)
		(*it)->setEnabled(enabled);

    _pauseIfFocusLost = KConfigCollection::configValue("pause focus").toBool();
    _status->settingsChanged();
}

void MainWidget::configureKeys()
{
    KKeyDialog d(true, this);
    d.insert(_keybCollection, i18n("Keyboard game"));
    d.insert(actionCollection(), i18n("General"));
    d.configure();
}

void MainWidget::configureNotifications()
{
    KNotifyDialog::configure(this);
}

void MainWidget::gameStateChanged(KMines::GameState state)
{
    stateChanged(KMines::STATES[state]);
    if ( state==Playing ) setFocus();
}

void MainWidget::pause()
{
    _pause->activate();
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
    aboutData.addCredit("Mikhail Kourinny", I18N_NOOP("Solver/Adviser"));
    aboutData.addCredit("Thomas Capricelli", I18N_NOOP("Magic reveal mode"));
    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication a;
    KGlobal::locale()->insertCatalogue("libkdegames");
    KExtHighscore::ExtManager manager;

    if ( a.isRestored() ) RESTORE(MainWidget)
    else {
        MainWidget *mw = new MainWidget;
        mw->show();
    }
    return a.exec();
}
