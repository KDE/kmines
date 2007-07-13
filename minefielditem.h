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

/**
 * Graphics item that represents MineField.
 * It is composed of many (or little) of CellItems.
 * This class is responsible of generation game field
 * with given properties (num rows, num cols, num mines) and
 * handling resizes
 */
class MineFieldItem : public QGraphicsItem
{
public:
    /**
     * Constructor.
     * Generates field with given properties
     *
     * @param numRows number of rows
     * @param numCols number of columns
     * @param numMines number of mines
     */
    MineFieldItem( int numRows, int numCols, int numMines );
    /**
     * (re)Generates field with given properties.
     * Old properties & item states (if any) are reset
     *
     * @param numRows number of rows
     * @param numCols number of columns
     * @param numMines number of mines
     */
    void regenerateField( int numRows, int numCols, int numMines );
    /**
     * Resizes this graphics item so it fits in given rect
     */
    void resizeToFitInRect(const QRectF& rect);
    /**
     * Reimplemented from QGraphicsItem
     */
    QRectF boundingRect() const;// reimp
private:
    /**
     * Returns cell item at (row,col).
     * Always use this function instead hand-computing index in m_cells
     */
    inline CellItem* itemAt(int row, int col) { return m_cells.at( row*m_numCols + col ); }
    /**
     * Reimplemented from QGraphicsItem
     */
    void paint( QPainter * painter, const QStyleOptionGraphicsItem*, QWidget * widget = 0 );
    /**
     * Repositions all child cell items upon resizes
     */
    void adjustItemPositions();

    // note: in member functions use itemAt (see above )
    // instead of hand-computing index from row & col!
    // => not depend on how m_cells is represented
    /**
     * Array which holds all child cell items
     */
    QVector<CellItem*> m_cells;
    /**
     * Number of field rows
     */
    int m_numRows;
    /**
     * Number of field columns
     */
    int m_numCols;
    /**
     * Number of mines in field
     */
    int m_minesCount;
    /**
     * Random sequence used to generate mine positions
     */
    KRandomSequence m_randomSeq;
};

#endif
