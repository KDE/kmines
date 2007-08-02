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
#include "minefielditem.h"

#include <kdebug.h>
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>

#include "cellitem.h"
#include "borderitem.h"
#include "renderer.h"

MineFieldItem::MineFieldItem()
    : m_leftButtonPos(-1,-1), m_midButtonPos(-1,-1), m_gameOver(false)
{
//    regenerateField(numRows, numCols, numMines);
}

void MineFieldItem::regenerateField( int numRows, int numCols, int numMines )
{
    Q_ASSERT( numMines < numRows*numCols );

    kDebug() << "regenerate field";
    m_firstClick = true;
    m_gameOver = false;

    int oldSize = m_cells.size();
    int newSize = numRows*numCols;
    int oldBorderSize = m_borders.size();
    int newBorderSize = (numCols+2)*2 + (numRows+2)*2-4;

    // if field is being shrinked, delete elements at the end before resizing vector
    if(oldSize > newSize)
    {
        for( int i=newSize; i<oldSize; ++i )
        {
            // is this the best way to remove an item?
            scene()->removeItem(m_cells[i]);
            delete m_cells[i];
        }

        // adjust border item array too
        for( int i=newBorderSize; i<oldBorderSize; ++i)
        {
            scene()->removeItem(m_borders[i]);
            delete m_borders[i];
        }
    }

    m_cells.resize(newSize);
    m_borders.resize(newBorderSize);

    m_numRows = numRows;
    m_numCols = numCols;
    m_minesCount = numMines;
    m_numUnrevealed = m_numRows*m_numCols;

    for(int i=0; i<newSize; ++i)
    {
        // reset old, create new
        if(i<oldSize)
            m_cells[i]->reset();
        else
            m_cells[i] = new CellItem(this);
    }

    for(int i=oldBorderSize; i<newBorderSize; ++i)
            m_borders[i] = new BorderItem(this);

    setupBorderItems();

    // generating mines
    int minesToPlace = m_minesCount;
    int randomIdx = 0;
    while(minesToPlace != 0)
    {
        randomIdx = m_randomSeq.getLong( m_numRows*m_numCols );
        if(!m_cells.at(randomIdx)->hasMine())
        {
            m_cells.at(randomIdx)->setHasMine(true);
            minesToPlace--;
        }
        else
            continue;
    }

    // calculating digits for cells around mines
    for(int row=0; row < m_numRows; ++row)
        for(int col=0; col < m_numCols; ++col)
        {
            if(itemAt(row,col)->hasMine())
                continue;
            // simply looking at all 8 neighbour cells and adding +1 for each
            // mine we found
            int resultingDigit = 0;
            QList<CellItem*> neighbours = adjasentItemsFor(row,col);
            foreach(CellItem* item, neighbours)
            {
                if(item->hasMine())
                    resultingDigit++;
            }

            // having 0 is ok here - it'll be empty
            itemAt(row,col)->setDigit(resultingDigit);
        }

    adjustItemPositions();
    m_flaggedMinesCount = 0;
    emit flaggedMinesCountChanged(m_flaggedMinesCount);
}

void MineFieldItem::setupBorderItems()
{
    int i = 0;
    for(int row=0; row<m_numRows+2; ++row)
        for(int col=0; col<m_numCols+2; ++col)
        {
            if( row == 0 && col == 0)
            {
                m_borders.at(i)->setRowCol(0,0);
                m_borders.at(i)->setBorderType(KMinesState::BorderCornerNW);
                i++;
            }
            else if( row == 0 && col == m_numCols+1)
            {
                m_borders.at(i)->setRowCol(row,col);
                m_borders.at(i)->setBorderType(KMinesState::BorderCornerNE);
                i++;
            }
            else if( row == m_numRows+1 && col == 0 )
            {
                m_borders.at(i)->setRowCol(row,col);
                m_borders.at(i)->setBorderType(KMinesState::BorderCornerSW);
                i++;
            }
            else if( row == m_numRows+1 && col == m_numCols+1 )
            {
                m_borders.at(i)->setRowCol(row,col);
                m_borders.at(i)->setBorderType(KMinesState::BorderCornerSE);
                i++;
            }
            else if( row == 0 )
            {
                m_borders.at(i)->setRowCol(row,col);
                m_borders.at(i)->setBorderType(KMinesState::BorderNorth);
                i++;
            }
            else if( row == m_numRows+1 )
            {
                m_borders.at(i)->setRowCol(row,col);
                m_borders.at(i)->setBorderType(KMinesState::BorderSouth);
                i++;
            }
            else if( col == 0 )
            {
                m_borders.at(i)->setRowCol(row,col);
                m_borders.at(i)->setBorderType(KMinesState::BorderWest);
                i++;
            }
            else if( col == m_numCols+1 )
            {
                m_borders.at(i)->setRowCol(row,col);
                m_borders.at(i)->setBorderType(KMinesState::BorderEast);
                i++;
            }
        }
}
QRectF MineFieldItem::boundingRect() const
{
    qreal cellSize = KMinesRenderer::self()->cellSize();
    // +2 - because of border on each side
    return QRectF(0, 0, cellSize*(m_numCols+2), cellSize*(m_numRows+2));
}

void MineFieldItem::paint( QPainter * painter, const QStyleOptionGraphicsItem* opt, QWidget* w)
{
    Q_UNUSED(painter);
    Q_UNUSED(opt);
    Q_UNUSED(w);
}

void MineFieldItem::resizeToFitInRect(const QRectF& rect)
{
    prepareGeometryChange();

    // +2 in some places - because of border on each side

    // here follows "cooomplex" algorithm to choose which side to
    // take when calculating cell size by dividing this side by
    // numRows or numCols correspondingly
    // it's cooomplex, because I have to paint some figures on paper
    // to understand that criteria for choosing one side or another (for
    // determining cell size from it) is comparing
    // cols/r.width() and rows/r.height():
    bool chooseHorizontalSide = (m_numCols+2) / rect.width() > (m_numRows+2) / rect.height();

    qreal size = 0;
    if( chooseHorizontalSide )
        size = rect.width() / (m_numCols+2);
    else
        size = rect.height() / (m_numRows+2);

    KMinesRenderer::self()->setCellSize( static_cast<int>(size) );

    foreach( CellItem* item, m_cells )
        item->updatePixmap();

    foreach( BorderItem *item, m_borders)
        item->updatePixmap();

    adjustItemPositions();
}

void MineFieldItem::adjustItemPositions()
{
    Q_ASSERT( m_cells.size() == m_numRows*m_numCols );

    int itemSize = KMinesRenderer::self()->cellSize();

    for(int row=0; row<m_numRows; ++row)
        for(int col=0; col<m_numCols; ++col)
        {
            itemAt(row,col)->setPos((col+1)*itemSize, (row+1)*itemSize);
        }

    foreach( BorderItem* item, m_borders )
    {
        item->setPos( item->col()*itemSize, item->row()*itemSize );
    }
}

void MineFieldItem::onItemRevealed(int row, int col)
{
    m_numUnrevealed--;
    if(itemAt(row,col)->hasMine())
    {
        revealAllMines();
    }
    else if(itemAt(row,col)->digit() == 0) // empty cell
    {
        revealEmptySpace(row,col);
    }
    // now let's check for possible win/loss
    checkLost();
    if(!m_gameOver) // checkLost might set it
        checkWon();
}

void MineFieldItem::revealEmptySpace(int row, int col)
{
    // recursively reveal neighbour cells until we find cells with digit
    typedef QPair<int,int> RowColPair;
    QList<RowColPair> list = adjasentRowColsFor(row,col);
    CellItem *item = 0;

    foreach( const RowColPair& pair, list )
    {
        // first is row, second is col
        item = itemAt(pair);
        if(item->isRevealed() || item->isFlagged())
            continue;
        if(item->digit() == 0)
        {
            item->reveal();
            m_numUnrevealed--;
            revealEmptySpace(pair.first,pair.second);
        }
        else if(!item->isFlagged())
        {
            item->reveal();
            m_numUnrevealed--;
        }
    }
}

void MineFieldItem::mousePressEvent( QGraphicsSceneMouseEvent *ev )
{
    if(m_gameOver)
        return;

    int itemSize = KMinesRenderer::self()->cellSize();
    int row = static_cast<int>(ev->pos().y()/itemSize)-1;
    int col = static_cast<int>(ev->pos().x()/itemSize)-1;
    if( row <0 || row >= m_numRows || col < 0 || col >= m_numCols )
        return;

    if(m_firstClick)
    {
        m_firstClick = false;
        emit firstClickDone();
    }

    if(ev->button() == Qt::LeftButton)
    {
        if(m_midButtonPos.first != -1) // mid-button is already pressed
            return;

        itemAt(row,col)->press();
        m_leftButtonPos = qMakePair(row,col);
    }
    else if(ev->button() == Qt::RightButton)
    {
        CellItem* itemUnderMouse = itemAt(row,col);
        bool wasFlagged = itemUnderMouse->isFlagged();

        itemUnderMouse->mark();

        bool flagStateChanged = (itemUnderMouse->isFlagged() != wasFlagged);
        if(flagStateChanged)
        {
            if(itemUnderMouse->isFlagged())
                m_flaggedMinesCount++;
            else
                m_flaggedMinesCount--;
            emit flaggedMinesCountChanged(m_flaggedMinesCount);
        }
    }
    else if(ev->button() == Qt::MidButton)
    {
        QList<CellItem*> neighbours = adjasentItemsFor(row,col);
        foreach(CellItem* item, neighbours)
        {
            if(!item->isFlagged() && !item->isRevealed())
                item->press();
            m_midButtonPos = qMakePair(row,col);
        }
    }
}

void MineFieldItem::mouseReleaseEvent( QGraphicsSceneMouseEvent * ev)
{
    if(m_gameOver)
        return;

    int itemSize = KMinesRenderer::self()->cellSize();

    int row = static_cast<int>(ev->pos().y()/itemSize)-1;
    int col = static_cast<int>(ev->pos().x()/itemSize)-1;

    if( row <0 || row >= m_numRows || col < 0 || col >= m_numCols )
    {
        // there might be the case when player moved mouse outside game field
        // while holding mid button and released it outside the field
        // in this case we must unpress pressed buttons, let's do it here
        // and return
        if(m_midButtonPos.first != -1)
        {
            QList<CellItem*> neighbours = adjasentItemsFor(m_midButtonPos.first,m_midButtonPos.second);
            foreach(CellItem *item, neighbours)
                item->undoPress();
            m_midButtonPos = qMakePair(-1,-1);
        }
        // same with left button
        if(m_leftButtonPos.first != -1)
        {
            itemAt(m_leftButtonPos)->undoPress();
            m_leftButtonPos = qMakePair(-1,-1);
        }
        return;
    }

    CellItem* itemUnderMouse = itemAt(row,col);

    if(ev->button() == Qt::LeftButton)
    {
        if(m_midButtonPos.first != -1) // mid-button is already pressed
            return;

        // this can happen like this:
        // mid-button pressed, left-button pressed, mid-button released, left-button released
        // m_leftButtonPos never gets set in this scenario, so we must protect ourselves :)
        if(m_leftButtonPos.first == -1)
            return;

        if(!itemUnderMouse->isRevealed()) // revealing only unrevealed ones
        {
            itemUnderMouse->release();
            if(itemUnderMouse->isRevealed())
                onItemRevealed(row,col);
        }
        m_leftButtonPos = qMakePair(-1,-1);//reset
    }
    else if( ev->button() == Qt::MidButton )
    {
        m_midButtonPos = qMakePair(-1,-1);

        QList<CellItem*> neighbours = adjasentItemsFor(row,col);
        if(!itemUnderMouse->isRevealed())
        {
            foreach(CellItem *item, neighbours)
                item->undoPress();
            return;
        }

        int numFlags = 0;
        int numMines = 0;
        foreach(CellItem *item, neighbours)
        {
            if(item->isFlagged())
                numFlags++;
            if(item->hasMine())
                numMines++;
        }
        if(numFlags == numMines && numFlags != 0)
        {
            foreach(CellItem *item, neighbours)
            {
                if(!item->isRevealed()) // revealing only unrevealed ones
                {
                    // force=true to omit Pressed check
                    item->release(true);
                    if(item->isRevealed())
                        onItemRevealed(item);
                }
            }
        }
        else
        {
            foreach(CellItem *item, neighbours)
                item->undoPress();
        }
    }
}

void MineFieldItem::mouseMoveEvent( QGraphicsSceneMouseEvent *ev )
{
    if(m_gameOver)
        return;

    int itemSize = KMinesRenderer::self()->cellSize();

    int row = static_cast<int>(ev->pos().y()/itemSize)-1;
    int col = static_cast<int>(ev->pos().x()/itemSize)-1;

    if( row < 0 || row >= m_numRows || col < 0 || col >= m_numCols )
        return;

    if(ev->buttons() & Qt::LeftButton)
    {
        if((m_leftButtonPos.first != -1 && m_leftButtonPos.second != -1) &&
           (m_leftButtonPos.first != row || m_leftButtonPos.second != col))
        {
            itemAt(m_leftButtonPos)->undoPress();
            itemAt(row,col)->press();
            m_leftButtonPos = qMakePair(row,col);
        }
    }
    if(ev->buttons() & Qt::MidButton)
    {
        if((m_midButtonPos.first != -1 && m_midButtonPos.second != -1) &&
           (m_midButtonPos.first != row || m_midButtonPos.second != col))
        {
            // un-press previously pressed cells
            QList<CellItem*> prevNeighbours = adjasentItemsFor(m_midButtonPos.first,
                                                              m_midButtonPos.second);
            foreach(CellItem *item, prevNeighbours)
                   item->undoPress();

            // and press current neighbours
            QList<CellItem*> neighbours = adjasentItemsFor(row,col);
            foreach(CellItem *item, neighbours)
                item->press();

            m_midButtonPos = qMakePair(row,col);
        }
    }
}

void MineFieldItem::revealAllMines()
{
    foreach( CellItem* item, m_cells )
    {
        if( (item->isFlagged() && !item->hasMine()) || (!item->isFlagged() && item->hasMine()) )
        {
            item->reveal();
            m_numUnrevealed--;
        }
    }
}

void MineFieldItem::onItemRevealed(CellItem* item)
{
    int idx = m_cells.indexOf(item);
    if(idx == -1)
    {
        kDebug() << "really strange - item not found";
        return;
    }

    int row = idx / m_numCols;
    int col = idx - row*m_numCols;
    onItemRevealed(row,col);
}

void MineFieldItem::checkLost()
{
    // for loss...
    foreach( CellItem* item, m_cells )
    {
        if(item->isExploded())
        {
            m_gameOver = true;
            emit gameOver(false);
            break;
        }
    }
}

void MineFieldItem::checkWon()
{
    // let's check the trivial case when
    // only some cells left unflagged and they
    // all contain bombs. this counts as win
    if(m_numUnrevealed == m_minesCount)
    {
        m_gameOver = true;
        emit gameOver(true);
    }
}

QList<QPair<int,int> > MineFieldItem::adjasentRowColsFor(int row, int col)
{
    QList<QPair<int,int> > resultingList;
    if(row != 0 && col != 0) // upper-left diagonal
        resultingList.append( qMakePair(row-1,col-1) );
    if(row != 0) // upper
        resultingList.append(qMakePair(row-1, col));
    if(row != 0 && col != m_numCols-1) // upper-right diagonal
        resultingList.append(qMakePair(row-1, col+1));
    if(col != 0) // on the left
        resultingList.append(qMakePair(row,col-1));
    if(col != m_numCols-1) // on the right
        resultingList.append(qMakePair(row, col+1));
    if(row != m_numRows-1 && col != 0) // bottom-left diagonal
        resultingList.append(qMakePair(row+1, col-1));
    if(row != m_numRows-1) // bottom
        resultingList.append(qMakePair(row+1, col));
    if(row != m_numRows-1 && col != m_numCols-1) // bottom-right diagonal
        resultingList.append(qMakePair(row+1, col+1));
    return resultingList;
}

QList<CellItem*> MineFieldItem::adjasentItemsFor(int row, int col)
{
    QList<QPair<int,int> > rowcolList = adjasentRowColsFor(row,col);
    QList<CellItem*> resultingList;
    typedef QPair<int,int> RowColPair;
    foreach( const RowColPair& pair, rowcolList )
        resultingList.append( itemAt(pair) );
    return resultingList;
}

