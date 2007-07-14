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
#include <QPair>
#include <KRandomSequence>

class CellItem;
class QSignalMapper;

/**
 * Graphics item that represents MineField.
 * It is composed of many (or little) of CellItems.
 * This class is responsible of generation game field
 * with given properties (num rows, num cols, num mines) and
 * handling resizes
 */
class MineFieldItem : public QObject, public QGraphicsItem
{
    Q_OBJECT
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
    /**
     * @return num rows in field
     */
    int rowCount() const { return m_numRows; }
    /**
     * @return num columns in field
     */
    int columnCount() const { return m_numCols; }
    /**
     * @return num mines in field
     */
    int minesCount() const { return m_minesCount; }
signals:
    void flaggedMinesCountChanged(int);
private slots:
    void onItemRevealed(int idx);
    void onItemFlagStateChanged(int idx);
private:
    /**
     * Returns cell item at (row,col).
     * Always use this function instead hand-computing index in m_cells
     */
    inline CellItem* itemAt(int row, int col) { return m_cells.at( row*m_numCols + col ); }
    /**
     * Calculates (row,col) from given index in m_cells and returns them in QPair
     */
    inline QPair<int,int> rowColFromIndex(int idx)
        {
            int row = idx/m_numCols;
            return qMakePair(row, idx - row*m_numCols);
        }

    /**
     * Reimplemented from QGraphicsItem
     */
    void paint( QPainter * painter, const QStyleOptionGraphicsItem*, QWidget * widget = 0 );
    /**
     * Repositions all child cell items upon resizes
     */
    void adjustItemPositions();
    /**
     * Reveals all empty cells around cell at (row,col),
     * until it found cells with digits (which are also revealed)
     */
    void revealEmptySpace(int row, int col);

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
     * Number of flagged mines
     */
    int m_flaggedMinesCount;
    /**
     * Random sequence used to generate mine positions
     */
    KRandomSequence m_randomSeq;
    /**
     * Used to map signals from all items to single slot
     */
    QSignalMapper *m_revealSignalMapper;
    QSignalMapper *m_flagSignalMapper;
};

#endif
