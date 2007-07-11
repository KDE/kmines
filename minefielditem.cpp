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

#include "cellitem.h"

MineFieldItem::MineFieldItem( int numRows, int numCols, int numMines )
{
    changeField(numRows, numCols, numMines);
}

void MineFieldItem::changeField( int numRows, int numCols, int numMines )
{
    int oldSize = m_cells.size();

    m_numRows = numRows;
    m_numCols = numCols;
    m_minesCount = numMines;

    m_cells.resize(m_numRows*m_numCols);

    for(int i=oldSize; i<m_numRows*m_numCols; ++i)
    {
        m_cells[i] = new CellItem(this);
    }

    adjustItemPositions();
}

QRectF MineFieldItem::boundingRect() const
{
    return childrenBoundingRect();
}

void MineFieldItem::paint( QPainter * painter, const QStyleOptionGraphicsItem* opt, QWidget* w)
{

}

void MineFieldItem::setCellSize(int size)
{
    prepareGeometryChange();

    foreach( CellItem* item, m_cells )
    {
        item->setSize(size);
    }

    adjustItemPositions();
}

void MineFieldItem::adjustItemPositions()
{
    Q_ASSERT( m_cells.size() == m_numRows*m_numCols );

    int itemSize = m_cells.at(0)->size();

    for(int row=0; row<m_numRows; ++row)
        for(int col=0; col<m_numCols; ++col)
        {
            itemAt(row,col)->setPos(col*itemSize, row*itemSize);
        }

}
