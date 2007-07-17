/*
    Copyright 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include "mainwindow.h"
#include "scene.h"

#include <kgameclock.h>
#include <KStandardGameAction>
#include <KActionCollection>
#include <KStatusBar>
#include <KScoreDialog>
#include <KLocale>

KMinesMainWindow::KMinesMainWindow()
    : m_scoreDialog(0)
{
    m_scene = new KMinesScene(this);
    connect(m_scene, SIGNAL(minesCountChanged(int)), SLOT(onMinesCountChanged(int)));
    connect(m_scene, SIGNAL(gameOver(bool)), SLOT(onGameOver(bool)));
    connect(m_scene, SIGNAL(firstClickDone()), SLOT(onFirstClick()));

    KMinesView* view = new KMinesView( m_scene, this );
    view->setCacheMode( QGraphicsView::CacheBackground );
    view->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    view->setFrameStyle(QFrame::NoFrame);

    view->setOptimizationFlags( QGraphicsView::DontClipPainter |
                                QGraphicsView::DontSavePainterState |
                                QGraphicsView::DontAdjustForAntialiasing );


    m_gameClock = new KGameClock(this, KGameClock::MinSecOnly);
    connect(m_gameClock, SIGNAL(timeChanged(const QString&)), SLOT(advanceTime(const QString&)));

    statusBar()->insertItem( i18n("Mines: 0/0"), 0 );
    statusBar()->insertItem( i18n("Time: 00:00"), 1);
    setCentralWidget(view);
    setupActions();

    m_scoreDialog = new KScoreDialog(KScoreDialog::Name | KScoreDialog::Custom1, this);
    m_scoreDialog->addField(KScoreDialog::Custom1, "Time", "time");
    m_scoreDialog->hideField(KScoreDialog::Score);

    // TODO: load this from config
    KGameDifficulty::setLevel( KGameDifficulty::easy );
    newGame();
}

void KMinesMainWindow::setupActions()
{
    KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());

    KStandardGameAction::quit(this, SLOT(close()), actionCollection());

    KGameDifficulty::init(this, this, SLOT(levelChanged(KGameDifficulty::standardLevel)),
                         SLOT(customLevelChanged(int)));
    KGameDifficulty::addStandardLevel(KGameDifficulty::easy);
    KGameDifficulty::addStandardLevel(KGameDifficulty::medium);
    KGameDifficulty::addStandardLevel(KGameDifficulty::hard);
    KGameDifficulty::addCustomLevel(0, i18n("Custom"));

    setupGUI();
}

void KMinesMainWindow::onMinesCountChanged(int count)
{
    statusBar()->changeItem( i18n("Mines %1/%2", count, m_scene->totalMines()), 0 );
}

void KMinesMainWindow::levelChanged(KGameDifficulty::standardLevel level)
{
    switch(level)
    {
        case KGameDifficulty::easy:
            m_scene->startNewGame(9, 9, 10);
            break;
        case KGameDifficulty::medium:
            m_scene->startNewGame(16,16,40);
            break;
        case KGameDifficulty::hard:
            m_scene->startNewGame(16,30,99);
            break;
        default:
            //unsupported
            break;
    }
}

void KMinesMainWindow::customLevelChanged(int)
{
    // TESTING
    m_scene->startNewGame(35,60,30*20);
}

void KMinesMainWindow::newGame()
{
    levelChanged(KGameDifficulty::level());
    statusBar()->changeItem( i18n("Time: 00:00"), 1);
}

void KMinesMainWindow::onGameOver(bool won)
{
    m_gameClock->pause();
    if(won)
    {
        m_scoreDialog->setConfigGroup( KGameDifficulty::levelString() );

        KScoreDialog::FieldInfo scoreInfo;
        // score-in-seconds will be hidden
        scoreInfo[KScoreDialog::Score].setNum(m_gameClock->seconds());
        //score-as-time will be shown
        scoreInfo[KScoreDialog::Custom1] = m_gameClock->timeString();

        // we keep highscores as number of seconds
        if( m_scoreDialog->addScore(scoreInfo, KScoreDialog::LessIsMore) != 0 )
            m_scoreDialog->exec();
    }
}

void KMinesMainWindow::advanceTime(const QString& timeStr)
{
    statusBar()->changeItem( i18n("Time: %1", timeStr), 1 );
}

void KMinesMainWindow::onFirstClick()
{
    m_gameClock->restart();
}

void KMinesMainWindow::showHighscores()
{
    m_scoreDialog->setConfigGroup( KGameDifficulty::levelString() );
    m_scoreDialog->exec();
}
