/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2010 Brian Croom <brian.s.croom@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "minefielditem.h"

// own
#include "kmines_debug.h"
#include "cellitem.h"
#include "borderitem.h"
#include "settings.h"
// Qt
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QRandomGenerator>

MineFieldItem::MineFieldItem(KGameRenderer* renderer)
    : m_leftButtonPos(-1,-1), m_midButtonPos(-1,-1), m_gameOver(false),
      m_emulatingMidButton(false), m_renderer(renderer)
{
	setFlag(QGraphicsItem::ItemHasNoContents);
}

void MineFieldItem::resetMines()
{
    m_gameOver = false;
    m_numUnrevealed = m_numRows*m_numCols;

    for(CellItem* item : std::as_const(m_cells)) {
        item->unreveal();
        item->unflag();
        item->unexplode();
    }

    m_flaggedMinesCount = 0;
    Q_EMIT flaggedMinesCountChanged(m_flaggedMinesCount);
}


void MineFieldItem::initField( int numRows, int numCols, int numMines )
{
    numMines = qMin(numMines, numRows*numCols - MINIMAL_FREE );

    m_firstClick = true;
    m_gameOver = false;

    int oldSize = m_cells.size();
    int newSize = numRows*numCols;
    int oldBorderSize = m_borders.size();
    int newBorderSize = (numCols+2)*2 + (numRows+2)*2-4;

    // if field is being shrunk, delete elements at the end before resizing vector
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
    m_midButtonPos = qMakePair(-1, -1);
    m_leftButtonPos = qMakePair(-1, -1);

    for(int i=0; i<newSize; ++i)
    {
        // reset old, create new
        if(i<oldSize)
            m_cells[i]->reset();
        else
            m_cells[i] = new CellItem(m_renderer, this);
        // let it be empty by default
        // generateField() will adjust needed cells
        // to hold digits or mines
        m_cells[i]->setDigit(0);
    }

    for(int i=oldBorderSize; i<newBorderSize; ++i)
            m_borders[i] = new BorderItem(m_renderer, this);

    setupBorderItems();

    adjustItemPositions();
    m_flaggedMinesCount = 0;
    Q_EMIT flaggedMinesCountChanged(m_flaggedMinesCount);
}

void MineFieldItem::generateField(int clickedIdx)
{
    // generating mines ensuring that clickedIdx won't hold mine
    // and that it will be an empty cell so the user don't have
    // to make random guesses at the start of the game
    QList<int> cellsWithMines;
    int minesToPlace = m_minesCount;
    int randomIdx = 0;
    CellItem* item = nullptr;
    FieldPos fp = rowColFromIndex(clickedIdx);

    // this is the list of items we don't want to put the mine in
    // to ensure that clickedIdx will stay an empty cell
    // (it will be empty if none of surrounding items holds mine)
    QList<CellItem*> neighbForClicked = adjacentItemsFor(fp.first, fp.second);

    QRandomGenerator random(QRandomGenerator::global()->generate());
    while(minesToPlace != 0)
    {
        randomIdx = random.bounded( m_numRows*m_numCols );
        item = m_cells.at(randomIdx);
        if(!item->hasMine()
           && neighbForClicked.indexOf(item) == -1
           && randomIdx != clickedIdx)
        {
            // ok, let's mine this place! :-)
            item->setHasMine(true);
            cellsWithMines.append(randomIdx);
            minesToPlace--;
        }
        else
            continue;
    }

    for (int idx : std::as_const(cellsWithMines)) {
        FieldPos rc = rowColFromIndex(idx);
        const QList<CellItem*> neighbours = adjacentItemsFor(rc.first, rc.second);
        for (CellItem *item : neighbours) {
            if(!item->hasMine())
                item->setDigit( item->digit()+1 );
        }
    }
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
    // +2 - because of border on each side
    return QRectF(0, 0, m_cellSize*(m_numCols+2), m_cellSize*(m_numRows+2));
}

int MineFieldItem::rowCount() const
{
    return m_numRows;
}

int MineFieldItem::columnCount() const
{
    return m_numCols;
}

int MineFieldItem::minesCount() const
{
    return m_minesCount;
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

    m_cellSize = static_cast<int>(size);

    for (CellItem* item : std::as_const(m_cells)) {
        item->setRenderSize(QSize(m_cellSize, m_cellSize));
    }

    for (BorderItem *item : std::as_const(m_borders)) {
        item->setRenderSize(QSize(m_cellSize, m_cellSize));
    }

    adjustItemPositions();
}

void MineFieldItem::adjustItemPositions()
{
    Q_ASSERT( m_cells.size() == m_numRows*m_numCols );

    for(int row=0; row<m_numRows; ++row)
        for(int col=0; col<m_numCols; ++col)
        {
            itemAt(row,col)->setPos((col+1)*m_cellSize, (row+1)*m_cellSize);
        }

    for (BorderItem* item : std::as_const(m_borders)) {
        item->setPos( item->col()*m_cellSize, item->row()*m_cellSize );
    }
}

bool MineFieldItem::onItemRevealed(int row, int col)
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
    if(checkLost())
        return true;
    return checkWon();
}

void MineFieldItem::revealEmptySpace(int row, int col)
{
    // recursively reveal neighbour cells until we find cells with digit
    const QList<FieldPos> list = adjacentRowColsFor(row,col);
    CellItem *item = nullptr;

    for (const FieldPos& pos : list) {
        // first is row, second is col
        item = itemAt(pos);
        if(item->isRevealed() || item->isFlagged() || item->isQuestioned())
            continue;
        if(item->digit() == 0)
        {
            item->reveal();
            m_numUnrevealed--;
            revealEmptySpace(pos.first,pos.second);
        }
        else
        {
            item->reveal();
            m_numUnrevealed--;
        }
    }
}

void MineFieldItem::handleFlag(CellItem* itemUnderMouse)
{
    bool wasFlagged = itemUnderMouse->isFlagged();

    itemUnderMouse->mark();

    bool flagStateChanged = (itemUnderMouse->isFlagged() != wasFlagged);
    if(flagStateChanged)
    {
        if(itemUnderMouse->isFlagged())
            m_flaggedMinesCount++;
        else
            m_flaggedMinesCount--;
        Q_EMIT flaggedMinesCountChanged(m_flaggedMinesCount);
    }
}

void MineFieldItem::mousePressEvent( QGraphicsSceneMouseEvent *ev )
{
    if(m_gameOver)
        return;

    int row = static_cast<int>(ev->pos().y()/m_cellSize)-1;
    int col = static_cast<int>(ev->pos().x()/m_cellSize)-1;
    if( row <0 || row >= m_numRows || col < 0 || col >= m_numCols )
        return;

    CellItem* itemUnderMouse = itemAt(row,col);
    if(!itemUnderMouse)
    {
        qCDebug(KMINES_LOG) << "unexpected - no item under mouse";
        return;
    }

    bool useFastExplore = Settings::exploreWithLeftClickOnNumberCells();
    bool placeFlagWhenPressed = Settings::placeFlagOn() == Settings::EnumPlaceFlagOn::MousePress;
    m_emulatingMidButton = ( useFastExplore ? ( (ev->buttons() & Qt::LeftButton) && ( itemUnderMouse->isRevealed() ) ) : ( (ev->buttons() & Qt::LeftButton) && (ev->buttons() & Qt::RightButton) ) );
    bool midButtonPressed = (ev->button() == Qt::MiddleButton || m_emulatingMidButton );

    if(midButtonPressed)
    {
        // in case we just started mid-button emulation (first LeftClick then added a RightClick)
        // undo press that was made by LeftClick. in other cases it won't hurt :)
        itemUnderMouse->undoPress();

        const QList<CellItem*> neighbours = adjacentItemsFor(row,col);
        for (CellItem* item : neighbours) {
            if(!item->isFlagged() && !item->isQuestioned() && !item->isRevealed())
                item->press();
            m_midButtonPos = qMakePair(row,col);

            m_leftButtonPos = qMakePair(-1,-1); // reset it
        }
    }
    else if(ev->button() == Qt::LeftButton)
    {
        itemUnderMouse->press();
        m_leftButtonPos = qMakePair(row,col);
    }
    else if(placeFlagWhenPressed && ev->button() == Qt::RightButton && (ev->buttons() & Qt::LeftButton) == false)
    {
        handleFlag(itemUnderMouse);
    }
}

void MineFieldItem::mouseReleaseEvent( QGraphicsSceneMouseEvent * ev)
{
    if(m_gameOver)
        return;

    int row = static_cast<int>(ev->pos().y()/m_cellSize)-1;
    int col = static_cast<int>(ev->pos().x()/m_cellSize)-1;

    if( row <0 || row >= m_numRows || col < 0 || col >= m_numCols )
    {
        // there might be the case when player moved mouse outside game field
        // while holding mid button and released it outside the field
        // in this case we must unpress pressed buttons, let's do it here
        // and return
        if(m_midButtonPos.first != -1)
        {
            const QList<CellItem*> neighbours = adjacentItemsFor(m_midButtonPos.first,m_midButtonPos.second);
            for (CellItem *item : neighbours) {
                item->undoPress();
            }
            m_midButtonPos = qMakePair(-1,-1);
            m_emulatingMidButton = false;
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

    bool placeFlagWhenReleased = Settings::placeFlagOn() == Settings::EnumPlaceFlagOn::MouseRelease;
    bool midButtonReleased = (ev->button() == Qt::MiddleButton || m_emulatingMidButton);

    if( midButtonReleased )
    {
        m_midButtonPos = qMakePair(-1,-1);

        const QList<CellItem*> neighbours = adjacentItemsFor(row,col);
        if(!itemUnderMouse->isRevealed())
        {
            for (CellItem *item : neighbours) {
                item->undoPress();
            }
            return;
        }

        int numFlags = 0;
        int numMines = 0;
        for (CellItem *item : neighbours) {
            if(item->isFlagged())
                numFlags++;
            if(item->hasMine())
                numMines++;
        }
        if(numFlags == numMines && numFlags != 0)
        {
            for (CellItem *item : neighbours) {
                if(!item->isRevealed()) // revealing only unrevealed ones
                {
                    // force=true to omit Pressed check
                    item->release(true);
                    // If revealing the item ends the game, stop the loop,
                    // since everything that needs to be done for the current game is finished.
                    // Otherwise, if the user has restarted the game, we'll be revealing
                    // items for the new game.
                    if(item->isRevealed() && onItemRevealed(item))
                        break;
                }
            }
        }
        else
        {
            for (CellItem *item : neighbours) {
                item->undoPress();
            }
        }
    }
    else if(ev->button() == Qt::LeftButton && (ev->buttons() & Qt::RightButton) == false)
    {
        if(m_midButtonPos.first != -1) // mid-button is already pressed
        {
            itemUnderMouse->undoPress();
            return;
        }

        // this can happen like this:
        // mid-button pressed, left-button pressed, mid-button released, left-button released
        // m_leftButtonPos never gets set in this scenario, so we must protect ourselves :)
        if(m_leftButtonPos.first == -1)
            return;

        if(!itemUnderMouse->isRevealed()) // revealing only unrevealed ones
        {
            if(m_firstClick)
            {
                m_firstClick = false;
                generateField( row*m_numCols + col );
                Q_EMIT firstClickDone();
            }

            itemUnderMouse->release();
            if(itemUnderMouse->isRevealed())
                onItemRevealed(row,col);
        }
        m_leftButtonPos = qMakePair(-1,-1);//reset
    }
    else if(placeFlagWhenReleased && ev->button() == Qt::RightButton && (ev->buttons() & Qt::LeftButton) == false)
    {
        handleFlag(itemUnderMouse);
    }
}

void MineFieldItem::mouseMoveEvent( QGraphicsSceneMouseEvent *ev )
{
    if(m_gameOver)
        return;

    int row = static_cast<int>(ev->pos().y()/m_cellSize)-1;
    int col = static_cast<int>(ev->pos().x()/m_cellSize)-1;

    if( row < 0 || row >= m_numRows || col < 0 || col >= m_numCols )
        return;

    bool midButtonPressed = ((ev->buttons() & Qt::MiddleButton) ||
                            ( (ev->buttons() & Qt::LeftButton) && (ev->buttons() & Qt::RightButton) ) );

    if(midButtonPressed)
    {
        if((m_midButtonPos.first != -1 && m_midButtonPos.second != -1) &&
           (m_midButtonPos.first != row || m_midButtonPos.second != col))
        {
            // un-press previously pressed cells
            const QList<CellItem*> prevNeighbours = adjacentItemsFor(m_midButtonPos.first,
                                                                     m_midButtonPos.second);
            for (CellItem *item : prevNeighbours) {
                   item->undoPress();
            }

            // and press current neighbours
            const QList<CellItem*> neighbours = adjacentItemsFor(row,col);
            for (CellItem *item : neighbours) {
                item->press();
            }

            m_midButtonPos = qMakePair(row,col);
        }
    }
    else if(ev->buttons() & Qt::LeftButton)
    {
        if((m_leftButtonPos.first != -1 && m_leftButtonPos.second != -1) &&
           (m_leftButtonPos.first != row || m_leftButtonPos.second != col))
        {
            itemAt(m_leftButtonPos)->undoPress();
            itemAt(row,col)->press();
            m_leftButtonPos = qMakePair(row,col);
        }
    }
}

void MineFieldItem::revealAllMines()
{
    for (CellItem* item : std::as_const(m_cells)) {
        if( (item->isFlagged() && !item->hasMine()) || (!item->isFlagged() && item->hasMine()) )
        {
            item->reveal();
            m_numUnrevealed--;
        }
    }
}

bool MineFieldItem::onItemRevealed(CellItem* item)
{
    int idx = m_cells.indexOf(item);
    if(idx == -1)
    {
        qCDebug(KMINES_LOG) << "really strange - item not found";
        return false;
    }

    int row = idx / m_numCols;
    int col = idx - row*m_numCols;
    return onItemRevealed(row,col);
}

bool MineFieldItem::checkLost()
{
    // for loss...
    for (CellItem* item : std::as_const(m_cells)) {
        if(item->isExploded())
        {
            m_gameOver = true;
            Q_EMIT gameOver(false);
            return true;
        }
    }
    return false;
}

bool MineFieldItem::checkWon()
{
    // this also takes into account the trivial case when
    // only some cells left unflagged and they
    // all contain bombs. this counts as win
    if(m_numUnrevealed == m_minesCount)
    {
        // mark not flagged cells (if any) with flags
        for (CellItem* item : std::as_const(m_cells)) {
            if( item->isQuestioned() )
                item->mark();
            if( !item->isRevealed() && !item->isFlagged() )
                item->mark();
        }
        m_gameOver = true;
        // now all mines should be flagged, notify about this
        Q_EMIT flaggedMinesCountChanged(m_minesCount);
        Q_EMIT gameOver(true);
        return true;
    }
    return false;
}

QList<FieldPos> MineFieldItem::adjacentRowColsFor(int row, int col)
{
    QList<FieldPos> resultingList;
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

QList<CellItem*> MineFieldItem::adjacentItemsFor(int row, int col)
{
    const QList<FieldPos > rowcolList = adjacentRowColsFor(row,col);
    QList<CellItem*> resultingList;
    for (const FieldPos& pos : rowcolList) {
        resultingList.append( itemAt(pos) );
    }
    return resultingList;
}

#include "moc_minefielditem.cpp"
