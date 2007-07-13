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
#ifndef MINEFIELDITEM_H
#define MINEFIELDITEM_H

#include <QVector>
#include <QGraphicsItem>
#include <KRandomSequence>

class CellItem;

// TODO docs-docs-docs
class MineFieldItem : public QGraphicsItem
{
public:
    MineFieldItem( int numRows, int numCols, int numMines );
    void regenerateField( int numRows, int numCols, int numMines );
    void resizeToFitInRect(const QRectF& rect);
    QRectF boundingRect() const;// reimp
private:
    inline CellItem* itemAt(int row, int col) { return m_cells.at( row*m_numCols + col ); }

    void paint( QPainter * painter, const QStyleOptionGraphicsItem*, QWidget * widget = 0 );
    void adjustItemPositions();

    // note: in member functions use itemAt (see above )
    // instead of hand-computing index from row & col!
    // => not depend on how m_cells is represented
    QVector<CellItem*> m_cells;
    int m_numRows;
    int m_numCols;
    int m_minesCount;
    KRandomSequence m_randomSeq;
};

#endif
