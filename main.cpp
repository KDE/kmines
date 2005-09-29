/*
 * Copyright (c) 1996-2004 Nicolas HADACEK (hadacek@kde.org)
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "main.h"
#include "main.moc"

#include <qptrvector.h>

#include <kaccel.h>
#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kstdaction.h>
#include <kkeydialog.h>
#include <kstdgameaction.h>
#include <kaction.h>
#include <kdebug.h>
#include <knotifyclient.h>
#include <knotifydialog.h>
#include <khighscore.h>
#include <kconfigdialog.h>

#include "settings.h"
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
  : KZoomMainWindow(4, 100, 1, "kmines")
{
    KNotifyClient::startDaemon();

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
	KStdAction::preferences(this, SLOT(configureSettings()),
                            actionCollection());
	KStdAction::keyBindings(this, SLOT(configureKeys()), actionCollection());
    KStdAction::configureNotifications(this, SLOT(configureNotifications()),
                                       actionCollection());
    KStdGameAction::configureHighscores(this, SLOT(configureHighscores()),
                                        actionCollection());
	// Levels
    _levels = KStdGameAction::chooseGameType(0, 0, actionCollection());
    QStringList list;
    for (uint i=0; i<=Level::NB_TYPES; i++)
        list += i18n(Level::LABELS[i]);
    _levels->setItems(list);
    connect(_levels, SIGNAL(activated(int)), _status, SLOT(newGame(int)));

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

	setupGUI( KMainWindow::Save | Create );
	readSettings();
        setCentralWidget(_status);
        init("popup");
        addWidget(_status->field());
}

bool MainWidget::queryExit()
{
    _status->checkBlackMark();
    return KZoomMainWindow::queryExit();
}

void MainWidget::readSettings()
{
    settingsChanged();
    Level::Type type = (Level::Type) Settings::level();
    _levels->setCurrentItem(type);
    _status->newGame(type);
}

void MainWidget::showHighscores()
{
    KExtHighscore::show(this);
}

void MainWidget::focusOutEvent(QFocusEvent *e)
{
    if ( Settings::pauseFocus() && e->reason()==QFocusEvent::ActiveWindow
          && _status->isPlaying() ) pause();
    KMainWindow::focusOutEvent(e);
}

void MainWidget::configureSettings()
{
    if ( KConfigDialog::showDialog("settings") ) return;

    KConfigDialog *dialog = new KConfigDialog(this, "settings", Settings::self());
    GameConfig *gc = new GameConfig;
    dialog->addPage(gc, i18n("Game"), "package_system");
    dialog->addPage(new AppearanceConfig, i18n("Appearance"), "style");
    CustomConfig *cc = new CustomConfig;
    dialog->addPage(cc, i18n("Custom Game"), "package_settings");
    connect(dialog, SIGNAL(settingsChanged()), SLOT(settingsChanged()));
    dialog->show();
    cc->init();
    gc->init();
}

void MainWidget::configureHighscores()
{
    KExtHighscore::configure(this);
}

void MainWidget::settingsChanged()
{
    bool enabled = Settings::keyboardGame();
    QValueList<KAction *> list = _keybCollection->actions();
    QValueList<KAction *>::Iterator it;
    for (it = list.begin(); it!=list.end(); ++it)
        (*it)->setEnabled(enabled);
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

void MainWidget::writeZoomSetting(uint zoom)
{
  Settings::setCaseSize(zoom);
  Settings::writeConfig();
}

uint MainWidget::readZoomSetting() const
{
  return Settings::caseSize();
}

void MainWidget::writeMenubarVisibleSetting(bool visible)
{
  Settings::setMenubarVisible(visible);
  Settings::writeConfig();
}

bool MainWidget::menubarVisibleSetting() const
{
  return Settings::menubarVisible();
}

//----------------------------------------------------------------------------
static const char *DESCRIPTION
    = I18N_NOOP("KMines is a classic mine sweeper game");

int main(int argc, char **argv)
{
    KHighscore::init("kmines");

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
