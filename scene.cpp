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

#include "scene.h"

#include <QResizeEvent>
#include <KGamePopupItem>
#include <KLocale>

#include "minefielditem.h"
#include "renderer.h"
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

KMinesScene::KMinesScene( QObject* parent )
    : QGraphicsScene(parent)
{
    setItemIndexMethod( NoIndex );
    m_fieldItem = new MineFieldItem();
    connect(m_fieldItem, SIGNAL(flaggedMinesCountChanged(int)), SIGNAL(minesCountChanged(int)));
    connect(m_fieldItem, SIGNAL(firstClickDone()), SIGNAL(firstClickDone()));
    connect(m_fieldItem, SIGNAL(gameOver(bool)), SLOT(onGameOver(bool)));
    // and re-emit it for others
    connect(m_fieldItem, SIGNAL(gameOver(bool)), SIGNAL(gameOver(bool)));
    addItem(m_fieldItem);

    m_messageItem = new KGamePopupItem;
    m_messageItem->setMessageOpacity(0.9);
    m_messageItem->setMessageTimeout(4000);
    addItem(m_messageItem);
}

void KMinesScene::resizeScene(int width, int height)
{
    setSceneRect(0, 0, width, height);
    m_fieldItem->resizeToFitInRect( sceneRect() );
    m_fieldItem->setPos( sceneRect().width()/2 - m_fieldItem->boundingRect().width()/2,
                         sceneRect().height()/2 - m_fieldItem->boundingRect().height()/2 );
}

void KMinesScene::drawBackground( QPainter* p, const QRectF& )
{
    p->drawPixmap( 0, 0, KMinesRenderer::self()->backgroundPixmap(sceneRect().size().toSize()) );
}

void KMinesScene::startNewGame(int rows, int cols, int numMines)
{
    m_fieldItem->initField(rows, cols, numMines);
    // reposition items
    resizeScene((int)sceneRect().width(), (int)sceneRect().height());
}

int KMinesScene::totalMines() const
{
    return m_fieldItem->minesCount();
}

void KMinesScene::onGameOver(bool won)
{
    if(won)
        m_messageItem->showMessage(i18n("Congratulatons! You have won!"), KGamePopupItem::Center);
    else
        m_messageItem->showMessage(i18n("You have lost."), KGamePopupItem::Center);
}
