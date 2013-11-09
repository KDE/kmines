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
#ifndef MINEFIELDITEM_H
#define MINEFIELDITEM_H

#include <QVector>
#include <QGraphicsObject>
#include <QPair>
#include <KRandomSequence>

class KGameRenderer;
class CellItem;
class BorderItem;

typedef QPair<int,int> FieldPos;

/**
 * Graphics item that represents MineField.
 * It is composed of many (or little) of CellItems.
 * This class is responsible of generation game field
 * with given properties (num rows, num cols, num mines) and
 * handling resizes
 */
class MineFieldItem : public QGraphicsObject
{
    Q_OBJECT
public:
    /**
     * Constructor.
     */
    explicit MineFieldItem(KGameRenderer* renderer);
    /**
     * Initializes game field: creates items, places them on positions,
     * (re)sets some variables
     *
     * @param numRows number of rows
     * @param numCols number of columns
     * @param numMines number of mines
     */
    void initField( int numRows, int numCols, int numMines );
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

    /**
     * Minimal number of free positions on a field
     */
    static const int MINIMAL_FREE = 10;

signals:
    void flaggedMinesCountChanged(int);
    void firstClickDone();
    void gameOver(bool won);
private:
    // reimplemented
    virtual void mousePressEvent( QGraphicsSceneMouseEvent * );
    // reimplemented
    virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * );
    // reimplemented
    virtual void mouseMoveEvent( QGraphicsSceneMouseEvent * );

    /**
     * Returns cell item at (row,col).
     * Always use this function instead hand-computing index in m_cells
     */
    inline CellItem* itemAt(int row, int col) { return m_cells.at( row*m_numCols + col ); }
    /**
     * Overloaded one, which takes QPair
     */
    inline CellItem* itemAt( const FieldPos& pos ) { return itemAt(pos.first,pos.second); }
    /**
     * Calculates (row,col) from given index in m_cells and returns them in QPair
     */
    inline FieldPos rowColFromIndex(int idx)
        {
            int row = idx/m_numCols;
            return qMakePair(row, idx - row*m_numCols);
        }
    /**
     * Generates game field ensuring that cell at clickedIdx
     * will be empty to allow the player quickly jump into the game.
     *
     * @param clickedIdx specifies index which should NOT have mine and be empty
     */
    void generateField(int clickedIdx);
    /**
     * Returns all adjasent items for item at row, col
     */
    QList<CellItem*> adjasentItemsFor(int row, int col);
    /**
     * Returns all valid adjasent row,col pairs for row, col
     */
    QList<FieldPos> adjasentRowColsFor(int row, int col);
    /**
     * Checks if player lost the game
     */
    void checkLost();
    /**
     * Checks if player won the game
     */
    void checkWon();
    /**
     * Reveals all unmarked items containing mines
     */
    void revealAllMines();
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
    /**
     * Sets up border items (positions and properties)
     */
    void setupBorderItems();

    void onItemRevealed(int row, int col);
    // overload
    void onItemRevealed(CellItem* item);

    // note: in member functions use itemAt (see above )
    // instead of hand-computing index from row & col!
    // => not depend on how m_cells is represented
    /**
     * Array which holds all child cell items
     */
    QVector<CellItem*> m_cells;
    /**
     * Array which holds border items
     */
    QVector<BorderItem*> m_borders;
    /**
     * The width and height of minefield cells in scene coordinates
     */
    int m_cellSize;
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
     * row and column where mouse was pressed.
     * (-1,-1) if it is already released
     */
    FieldPos m_leftButtonPos;
    FieldPos m_midButtonPos;
    bool m_firstClick;
    bool m_gameOver;
    bool m_emulatingMidButton;
    int m_numUnrevealed;

    KGameRenderer* m_renderer;
};

#endif
