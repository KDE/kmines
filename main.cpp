/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

#include "status.h"
#include "highscores.h"
#include "version.h"
#include "dialogs.h"

const MainWidget::KeyData MainWidget::KEY_DATA[NB_KEYS] = {
{I18N_NOOP("Move up"),     "keyboard_moveup",    Key_Up,    SLOT(moveUp())},
{I18N_NOOP("Move down"),   "keyboard_movedown",  Key_Down,  SLOT(moveDown())},
{I18N_NOOP("Move right"),  "keyboard_moveright", Key_Right, SLOT(moveRight())},
{I18N_NOOP("Move left"),   "keyboard_moveleft",  Key_Left,  SLOT(moveLeft())},
{I18N_NOOP("Move at left edge"), "keyboard_leftedge", Key_Home, SLOT(moveLeftEdge())},
{I18N_NOOP("Move at right edge"), "keyboard_rightedge", Key_End, SLOT(moveRightEdge())},
{I18N_NOOP("Move at top edge"), "keyboard_topedge", Key_PageUp, SLOT(moveTop())},
{I18N_NOOP("Move at bottom edge"), "keyboard_bottomedge", Key_PageDown, SLOT(moveBottom())},
{I18N_NOOP("Reveal mine"), "keyboard_revealmine", Key_Space, SLOT(reveal())},
{I18N_NOOP("Mark mine"),   "keyboard_markmine",  Key_W,     SLOT(mark())},
{I18N_NOOP("Automatic reveal"), "keyboard_autoreveal", Key_Return, SLOT(autoReveal())}
};


MainWidget::MainWidget()
{
	installEventFilter(this);

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
    _configCollection.createConfigItem("menubar visible", _menu);
	KStdAction::preferences(this, SLOT(configureSettings()),
                            actionCollection());
	KStdAction::keyBindings(this, SLOT(configureKeys()), actionCollection());

	// Levels
    _levels = new KSelectAction(actionCollection(), "levels");
    connect(_levels, SIGNAL(activated(int)), _status, SLOT(newGame(int)));
    _configCollection.createConfigItem("level", _levels);

    // Adviser
    _advise = new KAction(i18n("Advise"), CTRL + Key_A,
                          _status, SLOT(advise()),
                          actionCollection(), "advise");
    _solve = new KAction(i18n("Solve"), 0, _status, SLOT(solve()),
                         actionCollection(), "solve");
    (void)new KAction(i18n("Solving rate..."), 0, _status, SLOT(solveRate()),
                      actionCollection(), "solve_rate");

	createGUI();
	readSettings();
	setCentralWidget(_status);

    QPopupMenu *popup =
        static_cast<QPopupMenu *>(factory()->container("popup", this));
    if (popup) KContextMenuManager::insert(this, popup);
}

bool MainWidget::queryExit()
{
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
    KExtHighscores::showHighscores(this);
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
    d.append( KExtHighscores::createConfigWidget(this) );
    d.append(new CustomConfig);
    connect(&d, SIGNAL(saved()), SLOT(settingsChanged()));
    d.exec();
}

void MainWidget::settingsChanged()
{
    bool enabled =
        KConfigCollection::configItemValue("keyboard game").toBool();
	QValueList<KAction *> list = _keybCollection->actions();
	QValueList<KAction *>::Iterator it;
	for (it = list.begin(); it!=list.end(); ++it)
		(*it)->setEnabled(enabled);

    _pauseIfFocusLost =
        KConfigCollection::configItemValue("pause focus").toBool();
    _status->settingsChanged();
}

void MainWidget::configureKeys()
{
    KKeyDialog d(true, this);
    d.insert(_keybCollection);
    d.insert(actionCollection());
    d.configure();
}

void MainWidget::gameStateChanged(KMines::GameState state)
{
    stateChanged(KMines::STATES[state]);
    if ( state==Playing ) setFocus();
}

void MainWidget::pause()
{
    action("game_pause")->activate();
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
    KGlobal::locale()->insertCatalogue("libkdehighscores");
    KExtHighscores::ExtHighscores highscores;
    if ( a.isRestored() ) RESTORE(MainWidget)
    else {
        MainWidget *mw = new MainWidget;
        mw->show();
    }
    return a.exec();
}
