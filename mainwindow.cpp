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
#include "minefielditem.h"
#include "scene.h"
#include "settings.h"
#include "renderer.h"

#include <KGameClock>
#include <KStandardGameAction>
#include <KActionCollection>
#include <KToggleAction>
#include <KStatusBar>
#include <KScoreDialog>
#include <KConfigDialog>
#include <KGameThemeSelector>
#include <KMessageBox>
#include <KLocale>
#include <QDesktopWidget>

#include "ui_customgame.h"
#include "ui_generalopts.h"

/*
 * Classes for config dlg pages
 */
class CustomGameConfig : public QWidget
{
    Q_OBJECT

public:
    CustomGameConfig(QWidget *parent)
        : QWidget(parent)
    {
        ui.setupUi(this);
        connect(ui.kcfg_CustomWidth, SIGNAL(valueChanged(int)), this, SLOT(updateMaxMines()));
        connect(ui.kcfg_CustomHeight, SIGNAL(valueChanged(int)), this, SLOT(updateMaxMines()));
    }

private slots:
    void updateMaxMines()
    {
        int width = ui.kcfg_CustomWidth->value();
        int height = ui.kcfg_CustomHeight->value();
        int max = qMax(1, width * height - MineFieldItem::MINIMAL_FREE);
        ui.kcfg_CustomMines->setMaximum(max);
    }

private:
    Ui::CustomGameConfig ui;
};

class GeneralOptsConfig : public QWidget
{
public:
    GeneralOptsConfig(QWidget *parent)
        : QWidget(parent)
    {
        ui.setupUi(this);
    }

private:
    Ui::GeneralOptsConfig ui;
};

/*
 * Main window
 */

KMinesMainWindow::KMinesMainWindow()
{
    m_scene = new KMinesScene(this);
    connect(m_scene, SIGNAL(minesCountChanged(int)), SLOT(onMinesCountChanged(int)));
    connect(m_scene, SIGNAL(gameOver(bool)), SLOT(onGameOver(bool)));
    connect(m_scene, SIGNAL(firstClickDone()), SLOT(onFirstClick()));

    m_view = new KMinesView( m_scene, this );
    m_view->setCacheMode( QGraphicsView::CacheBackground );
    m_view->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_view->setFrameStyle(QFrame::NoFrame);

    m_view->setOptimizationFlags( QGraphicsView::DontClipPainter |
                                QGraphicsView::DontSavePainterState |
                                QGraphicsView::DontAdjustForAntialiasing );


    m_gameClock = new KGameClock(this, KGameClock::MinSecOnly);
    connect(m_gameClock, SIGNAL(timeChanged(const QString&)), SLOT(advanceTime(const QString&)));

    statusBar()->insertItem( i18n("Mines: 0/0"), 0 );
    statusBar()->insertItem( i18n("Time: 00:00"), 1);
    setCentralWidget(m_view);
    setupActions();

    KGameDifficulty::setLevel( KGameDifficulty::standardLevel(Settings::level()) );

    newGame();
}

void KMinesMainWindow::setupActions()
{
    KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());

    KStandardGameAction::quit(this, SLOT(close()), actionCollection());
    KStandardAction::preferences( this, SLOT( configureSettings() ), actionCollection() );
    m_actionPause = KStandardGameAction::pause( this, SLOT(pauseGame(bool)), actionCollection() );

    KGameDifficulty::init(this, this, SLOT(levelChanged(KGameDifficulty::standardLevel)),
                         SLOT(customLevelChanged(int)));
    KGameDifficulty::setRestartOnChange(KGameDifficulty::RestartOnChange);
    KGameDifficulty::addStandardLevel(KGameDifficulty::Easy);
    KGameDifficulty::addStandardLevel(KGameDifficulty::Medium);
    KGameDifficulty::addStandardLevel(KGameDifficulty::Hard);
    KGameDifficulty::addStandardLevel(KGameDifficulty::Configurable);

    setupGUI(qApp->desktop()->availableGeometry().size()*0.4);
}

void KMinesMainWindow::onMinesCountChanged(int count)
{
    statusBar()->changeItem( i18n("Mines: %1/%2", count, m_scene->totalMines()), 0 );
}

void KMinesMainWindow::levelChanged(KGameDifficulty::standardLevel)
{
    Settings::setLevel(KGameDifficulty::level());
    Settings::self()->writeConfig();

    newGame();
}

void KMinesMainWindow::customLevelChanged(int)
{
    newGame();
}

void KMinesMainWindow::newGame()
{
    m_gameClock->restart();
    m_gameClock->pause(); // start only with the 1st click
    
    // some things to manage pause
    if( m_actionPause->isChecked() )
    {
            m_scene->setGamePaused(false);
            m_actionPause->setChecked(false);
    }
    m_actionPause->setEnabled(false);
    
    KGameDifficulty::setRunning(false);
    switch(KGameDifficulty::level())
    {
        case KGameDifficulty::Easy:
            m_scene->startNewGame(9, 9, 10);
            break;
        case KGameDifficulty::Medium:
            m_scene->startNewGame(16,16,40);
            break;
        case KGameDifficulty::Hard:
            m_scene->startNewGame(16,30,99);
            break;
        case KGameDifficulty::Configurable:
            m_scene->startNewGame(Settings::customHeight(),
                                  Settings::customWidth(),
                                  Settings::customMines());
        default:
            //unsupported
            break;
    }
    statusBar()->changeItem( i18n("Time: 00:00"), 1);
}

void KMinesMainWindow::onGameOver(bool won)
{
    m_gameClock->pause();
    m_actionPause->setEnabled(false);
    KGameDifficulty::setRunning(false);
    if(won)
    {
        KScoreDialog scoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
        scoreDialog.addLocalizedConfigGroupNames(KGameDifficulty::localizedLevelStrings()); //Add all the translations of the group names
        scoreDialog.setConfigGroupWeights(KGameDifficulty::levelWeights());
        scoreDialog.hideField(KScoreDialog::Score);
        
        QPair<QByteArray, QString> group = KGameDifficulty::localizedLevelString();
        if(group.first.isEmpty())
            group = qMakePair(QByteArray("Custom"), i18n("Custom"));
        scoreDialog.setConfigGroup( group );

        KScoreDialog::FieldInfo scoreInfo;
        // score-in-seconds will be hidden
        scoreInfo[KScoreDialog::Score].setNum(m_gameClock->seconds());
        //score-as-time will be shown
        scoreInfo[KScoreDialog::Time] = m_gameClock->timeString();

        // we keep highscores as number of seconds
        if( scoreDialog.addScore(scoreInfo, KScoreDialog::LessIsMore) != 0 )
            scoreDialog.exec();
    }
}

void KMinesMainWindow::advanceTime(const QString& timeStr)
{
    statusBar()->changeItem( i18n("Time: %1", timeStr), 1 );
}

void KMinesMainWindow::onFirstClick()
{
    // enable pause action
    m_actionPause->setEnabled(true);
    // start clock
    m_gameClock->resume();
    KGameDifficulty::setRunning(true);
}

void KMinesMainWindow::showHighscores()
{
    KScoreDialog scoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
    scoreDialog.addLocalizedConfigGroupNames(KGameDifficulty::localizedLevelStrings()); //Add all the translations of the group names
    scoreDialog.setConfigGroupWeights(KGameDifficulty::levelWeights());
    scoreDialog.hideField(KScoreDialog::Score);
    scoreDialog.setConfigGroup( KGameDifficulty::localizedLevelString() );
    scoreDialog.exec();
}

void KMinesMainWindow::configureSettings()
{
    if ( KConfigDialog::showDialog( "settings" ) )
        return;
    KConfigDialog *dialog = new KConfigDialog( this, "settings", Settings::self() );
    dialog->addPage( new GeneralOptsConfig( dialog ), i18n("General"), "games-config-options" );
    dialog->addPage( new KGameThemeSelector( dialog, Settings::self(), KGameThemeSelector::NewStuffDisableDownload ), i18n( "Theme" ), "games-config-theme" );
    dialog->addPage( new CustomGameConfig( dialog ), i18n("Custom Game"), "games-config-custom" );
    connect( dialog, SIGNAL( settingsChanged(const QString&) ), this, SLOT( loadSettings() ) );
    dialog->setHelp(QString(),"kmines");
    dialog->show();
}

void KMinesMainWindow::pauseGame(bool paused)
{
    m_scene->setGamePaused( paused );
    if( paused )
        m_gameClock->pause();
    else
        m_gameClock->resume();
}

void KMinesMainWindow::loadSettings()
{
    if ( !KMinesRenderer::self()->loadTheme(Settings::theme()) )
    {
        KMessageBox::error( this,  i18n( "Failed to load \"%1\" theme. Please check your installation.", Settings::theme() ) );
        return;
    }

    m_view->resetCachedContent();
    // trigger complete redraw
    m_scene->resizeScene( (int)m_scene->sceneRect().width(),
                          (int)m_scene->sceneRect().height() );
}

#include "mainwindow.moc"
#include "moc_mainwindow.cpp"
