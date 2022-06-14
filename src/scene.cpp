/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2010 Brian Croom <brian.s.croom@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "scene.h"

// own
#include "settings.h"
#include "minefielditem.h"
// KDEGames
#include <kdegames_version.h>
#include <KGamePopupItem>
#include <KgThemeProvider>
// KF
#include <KLocalizedString>
// Qt
#include <QResizeEvent>

// --------------- KMinesView ---------------

KMinesView::KMinesView( KMinesScene* scene, QWidget *parent )
    : QGraphicsView(scene, parent), m_scene(scene)
{
}

void KMinesView::resizeEvent( QResizeEvent *ev )
{
    m_scene->resizeScene( ev->size().width(), ev->size().height() );
}

// -------------- KMinesScene --------------------

static KgThemeProvider* provider()
{
    auto* prov = new KgThemeProvider;
#if KDEGAMES_VERSION >= QT_VERSION_CHECK(7, 4, 0)
    prov->discoverThemes(QStringLiteral("themes"));
#else
    prov->discoverThemes("appdata", QStringLiteral("themes"));
#endif

    return prov;
}

KMinesScene::KMinesScene( QObject* parent )
    : QGraphicsScene(parent), m_renderer(provider())
{
    setItemIndexMethod( NoIndex );
    m_fieldItem = new MineFieldItem(&m_renderer);
    connect(m_fieldItem, &MineFieldItem::flaggedMinesCountChanged, this, &KMinesScene::minesCountChanged);
    connect(m_fieldItem, &MineFieldItem::firstClickDone, this, &KMinesScene::firstClickDone);
    connect(m_fieldItem, &MineFieldItem::gameOver, this, &KMinesScene::onGameOver);
    // and re-emit it for others
    connect(m_fieldItem, &MineFieldItem::gameOver, this, &KMinesScene::gameOver);
    addItem(m_fieldItem);

    m_messageItem = new KGamePopupItem;
    m_messageItem->setMessageOpacity(0.9);
    m_messageItem->setMessageTimeout(4000);
    addItem(m_messageItem);
    
    m_gamePausedMessageItem = new KGamePopupItem;
    m_gamePausedMessageItem->setMessageOpacity(0.9);
    m_gamePausedMessageItem->setMessageTimeout(0);
    m_gamePausedMessageItem->setHideOnMouseClick(false);
    addItem(m_gamePausedMessageItem);
    
    setBackgroundBrush(m_renderer.spritePixmap(QStringLiteral( "mainWidget" ), sceneRect().size().toSize()));
}

void KMinesScene::reset()
{
    m_fieldItem->resetMines();
    m_messageItem->forceHide();
}

bool KMinesScene::canScore() const
{
    return m_canScore;
}

void KMinesScene::setCanScore(bool value)
{
    m_canScore = value;
}

void KMinesScene::resizeScene(int width, int height)
{
    setSceneRect(0, 0, width, height);
    setBackgroundBrush(m_renderer.spritePixmap(QStringLiteral( "mainWidget" ), sceneRect().size().toSize()));
    m_fieldItem->resizeToFitInRect( sceneRect() );
    m_fieldItem->setPos( sceneRect().width()/2 - m_fieldItem->boundingRect().width()/2,
                         sceneRect().height()/2 - m_fieldItem->boundingRect().height()/2 );
    m_gamePausedMessageItem->setPos( sceneRect().width()/2 - m_gamePausedMessageItem->boundingRect().width()/2,
                          sceneRect().height()/2 - m_gamePausedMessageItem->boundingRect().height()/2 );
    m_messageItem->setPos( sceneRect().width()/2 - m_messageItem->boundingRect().width()/2,
                          sceneRect().height()/2 - m_messageItem->boundingRect().height()/2 );
}

void KMinesScene::startNewGame(int rows, int cols, int numMines)
{
    // hide message if any
    m_messageItem->forceHide();

    m_fieldItem->initField(rows, cols, numMines);
    // reposition items
    resizeScene((int)sceneRect().width(), (int)sceneRect().height());
}

int KMinesScene::totalMines() const
{
    return m_fieldItem->minesCount();
}

void KMinesScene::setGamePaused(bool paused)
{
    m_fieldItem->setVisible(!paused);
    if(paused)
        m_gamePausedMessageItem->showMessage(i18n("Game is paused."), KGamePopupItem::Center);
    else
        m_gamePausedMessageItem->forceHide();
}

void KMinesScene::onGameOver(bool won)
{
    if(won)
        m_messageItem->showMessage(i18n("Congratulations! You have won!"), KGamePopupItem::Center);
    else
        m_messageItem->showMessage(i18n("You have lost."), KGamePopupItem::Center);
}


