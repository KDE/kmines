/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2010 Brian Croom <brian.s.croom@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mainwindow.h"

// own
#include "minefielditem.h"
#include "scene.h"
#include "settings.h"
#include "kmines_debug.h"
#include "ui_customgame.h"
#include "ui_generalopts.h"
// KDEGames
#include <KGameClock>
#include <KGameDifficulty>
#include <KStandardGameAction>
#include <KGameThemeSelector>
#include <KScoreDialog>
// KF
#include <KActionCollection>
#include <KConfigDialog>
#include <KLocalizedString>
// Qt
#include <QStatusBar>
#include <QMessageBox>
#include <QScreen>
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
        connect(ui.kcfg_CustomWidth, &QSpinBox::valueChanged,
                this, &CustomGameConfig::updateMaxMines);
        connect(ui.kcfg_CustomHeight, &QSpinBox::valueChanged,
                this, &CustomGameConfig::updateMaxMines);
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


    m_gameClock = new KGameClock(this, KGameClock::FlexibleHourMinSec);
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
    KStandardGameAction::gameNew(this, &KMinesMainWindow::newGame, actionCollection());
    KStandardGameAction::highscores(this, &KMinesMainWindow::showHighscores, actionCollection());

    KStandardGameAction::quit(this, &KMinesMainWindow::close, actionCollection());
    KStandardAction::preferences(this, &KMinesMainWindow::configureSettings, actionCollection());
    m_actionPause = KStandardGameAction::pause(this, &KMinesMainWindow::pauseGame, actionCollection());

    KGameDifficulty::global()->addStandardLevelRange(
        KGameDifficultyLevel::Easy, KGameDifficultyLevel::Hard
    );
    KGameDifficulty::global()->addLevel(new KGameDifficultyLevel(1000,
        QByteArray( "Custom" ), i18n( "Custom" )
    ));
    KGameDifficultyGUI::init(this);
    connect(KGameDifficulty::global(), &KGameDifficulty::currentLevelChanged, this, &KMinesMainWindow::newGame);

    setupGUI(screen()->availableGeometry().size() * 0.4);
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

    KGameDifficulty::global()->setGameRunning(false);
    switch(KGameDifficulty::globalLevel())
    {
        case KGameDifficultyLevel::Easy:
            m_scene->startNewGame(9, 9, 10);
            break;
        case KGameDifficultyLevel::Medium:
            m_scene->startNewGame(16,16,40);
            break;
        case KGameDifficultyLevel::Hard:
            m_scene->startNewGame(16,30,99);
            break;
        case KGameDifficultyLevel::Custom:
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
    KGameDifficulty::global()->setGameRunning(false);
    if(won && m_scene->canScore())
    {
        QPointer<KScoreDialog> scoreDialog = new KScoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
        scoreDialog->initFromDifficulty(KGameDifficulty::global());
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
    KGameDifficulty::global()->setGameRunning(true);
}

void KMinesMainWindow::showHighscores()
{
    QPointer<KScoreDialog> scoreDialog = new KScoreDialog(KScoreDialog::Name | KScoreDialog::Time, this);
    scoreDialog->initFromDifficulty(KGameDifficulty::global());
    scoreDialog->hideField(KScoreDialog::Score);
    scoreDialog->exec();
    delete scoreDialog;
}

void KMinesMainWindow::configureSettings()
{
    if ( KConfigDialog::showDialog( QStringLiteral(  "settings" ) ) )
        return;
    auto *dialog = new KConfigDialog( this, QStringLiteral( "settings" ), Settings::self() );
    dialog->addPage( new GeneralOptsConfig( dialog ), i18n("General"), QStringLiteral( "games-config-options" ));
    dialog->addPage( new KGameThemeSelector( m_scene->renderer().themeProvider() ), i18n( "Theme" ), QStringLiteral( "games-config-theme" ));
    dialog->addPage( new CustomGameConfig( dialog ), i18n("Custom Game"), QStringLiteral( "games-config-custom" ));
    connect( m_scene->renderer().themeProvider(), &KGameThemeProvider::currentThemeChanged, this, &KMinesMainWindow::loadSettings );
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
