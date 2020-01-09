/*
    Copyright 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    Copyright 2010 Brian Croom <brian.s.croom@gmail.com>

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
#include "kmines_debug.h"

#include <KGameClock>
#include <KgDifficulty>
#include <KStandardGameAction>
#include <KActionCollection>
#include <KScoreDialog>
#include <KConfigDialog>
#include <KgThemeSelector>
#include <QScreen>

#include <QStatusBar>
#include <QDesktopWidget>
#include <QMessageBox>
#include <KLocalizedString>

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

private Q_SLOTS:
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
    
    connect(m_scene, &KMinesScene::minesCountChanged, this, &KMinesMainWindow::onMinesCountChanged);
    connect(m_scene, &KMinesScene::gameOver, this, &KMinesMainWindow::onGameOver);
    connect(m_scene, &KMinesScene::firstClickDone, this, &KMinesMainWindow::onFirstClick);

    m_view = new KMinesView( m_scene, this );
    m_view->setCacheMode( QGraphicsView::CacheBackground );
    m_view->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_view->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_view->setFrameStyle(QFrame::NoFrame);

    m_view->setOptimizationFlags( 
                                QGraphicsView::DontSavePainterState |
                                QGraphicsView::DontAdjustForAntialiasing );


    m_gameClock = new KGameClock(this, KGameClock::MinSecOnly);
    connect(m_gameClock, &KGameClock::timeChanged, this, &KMinesMainWindow::advanceTime);

    mineLabel->setText(i18n("Mines: 0/0"));
    timeLabel->setText(i18n("Time: 00:00"));
    
    statusBar()->insertPermanentWidget( 0, mineLabel );
    statusBar()->insertPermanentWidget( 1, timeLabel );
    setCentralWidget(m_view);
    setupActions();

    newGame();
}

void KMinesMainWindow::setupActions()
{
    KStandardGameAction::gameNew(this, SLOT(newGame()), actionCollection());
    KStandardGameAction::highscores(this, SLOT(showHighscores()), actionCollection());

    KStandardGameAction::quit(this, SLOT(close()), actionCollection());
    KStandardAction::preferences( this, SLOT(configureSettings()), actionCollection() );
    m_actionPause = KStandardGameAction::pause( this, SLOT(pauseGame(bool)), actionCollection() );

    Kg::difficulty()->addStandardLevelRange(
        KgDifficultyLevel::Easy, KgDifficultyLevel::Hard
    );
    Kg::difficulty()->addLevel(new KgDifficultyLevel(1000,
        QByteArray( "Custom" ), i18n( "Custom" )
    ));
    KgDifficultyGUI::init(this);
    connect(Kg::difficulty(), &KgDifficulty::currentLevelChanged, this, &KMinesMainWindow::newGame);

#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    setupGUI(QApplication::screens().at(0)->availableGeometry().size() * 0.4);
#else
    setupGUI(screen()->availableGeometry().size() * 0.4);
#endif
}

void KMinesMainWindow::onMinesCountChanged(int count)
{
    mineLabel->setText(i18n("Mines: %1/%2", count, m_scene->totalMines()));
}

void KMinesMainWindow::newGame()
{
    qCDebug(KMINES_LOG) << "Inside game";
    m_gameClock->restart();
    m_gameClock->pause(); // start only with the 1st click

    // some things to manage pause
    if( m_actionPause->isChecked() )
    {
            m_scene->setGamePaused(false);
            m_actionPause->setChecked(false);
    }
    m_actionPause->setEnabled(false);

    Kg::difficulty()->setGameRunning(false);
    switch(Kg::difficultyLevel())
    {
        case KgDifficultyLevel::Easy:
            m_scene->startNewGame(9, 9, 10);
            break;
        case KgDifficultyLevel::Medium:
            m_scene->startNewGame(16,16,40);
            break;
        case KgDifficultyLevel::Hard:
            m_scene->startNewGame(16,30,99);
            break;
        case KgDifficultyLevel::Custom:
            m_scene->startNewGame(Settings::customHeight(),
                                  Settings::customWidth(),
                                  Settings::customMines());
        default:
            //unsupported
            break;
    }
    
    timeLabel->setText(i18n("Time: 00:00"));
}

void KMinesMainWindow::onGameOver(bool won)
{
    m_gameClock->pause();
    m_actionPause->setEnabled(false);
    Kg::difficulty()->setGameRunning(false);
    if(won && m_scene->canScore())
    {
        QPointer<KScoreDialog> scoreDialog = new KScoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
        scoreDialog->initFromDifficulty(Kg::difficulty());
        scoreDialog->hideField(KScoreDialog::Score);

        KScoreDialog::FieldInfo scoreInfo;
        // score-in-seconds will be hidden
        scoreInfo[KScoreDialog::Score].setNum(m_gameClock->seconds());
        //score-as-time will be shown
        scoreInfo[KScoreDialog::Time] = m_gameClock->timeString();

        // we keep highscores as number of seconds
        if( scoreDialog->addScore(scoreInfo, KScoreDialog::LessIsMore) != 0 )
            scoreDialog->exec();

        delete scoreDialog;
    } else if (!won)
    {
        //ask to reset
        if (Settings::allowKminesReset() && QMessageBox::question(this, i18n("Reset?"), i18n("Reset the Game?")) == QMessageBox::Yes){
            m_scene->reset();
            m_gameClock->restart();
            m_actionPause->setEnabled(true);
            m_scene->setCanScore(!Settings::disableScoreOnReset());
        }
    }
}

void KMinesMainWindow::advanceTime(const QString& timeStr)
{
    timeLabel->setText(i18n("Time: %1", timeStr));
}

void KMinesMainWindow::onFirstClick()
{
    // enable pause action
    m_actionPause->setEnabled(true);
    // start clock
    m_gameClock->resume();
    Kg::difficulty()->setGameRunning(true);
}

void KMinesMainWindow::showHighscores()
{
    QPointer<KScoreDialog> scoreDialog = new KScoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
    scoreDialog->initFromDifficulty(Kg::difficulty());
    scoreDialog->hideField(KScoreDialog::Score);
    scoreDialog->exec();
    delete scoreDialog;
}

void KMinesMainWindow::configureSettings()
{
    if ( KConfigDialog::showDialog( QStringLiteral(  "settings" ) ) )
        return;
    KConfigDialog *dialog = new KConfigDialog( this, QStringLiteral( "settings" ), Settings::self() );
    dialog->addPage( new GeneralOptsConfig( dialog ), i18n("General"), QStringLiteral( "games-config-options" ));
    dialog->addPage( new KgThemeSelector( m_scene->renderer().themeProvider() ), i18n( "Theme" ), QStringLiteral( "games-config-theme" ));
    dialog->addPage( new CustomGameConfig( dialog ), i18n("Custom Game"), QStringLiteral( "games-config-custom" ));
    connect( m_scene->renderer().themeProvider(), &KgThemeProvider::currentThemeChanged, this, &KMinesMainWindow::loadSettings );
    connect(dialog, &KConfigDialog::settingsChanged, this, &KMinesMainWindow::loadSettings);
    
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
    m_view->resetCachedContent();
    // trigger complete redraw
    m_scene->resizeScene( (int)m_scene->sceneRect().width(),
                          (int)m_scene->sceneRect().height() );
}

#include "mainwindow.moc"
#include "moc_mainwindow.cpp"
