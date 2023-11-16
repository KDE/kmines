/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2010 Brian Croom <brian.s.croom@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MINEFIELDITEM_H
#define MINEFIELDITEM_H

// Qt
#include <QGraphicsObject>
#include <QList>
#include <QPair>

class KGameRenderer;
class CellItem;
class BorderItem;

using FieldPos = QPair<int, int>;

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
     * Resets mines to the initial state.
     */
    void resetMines();
    /**
     * Resizes this graphics item so it fits in given rect
     */
    void resizeToFitInRect(const QRectF& rect);
    /**
     * Reimplemented from QGraphicsItem
     */
    QRectF boundingRect() const override;// reimp
    /**
     * @return num rows in field
     */
    int rowCount() const;
    /**
     * @return num columns in field
     */
    int columnCount() const;
    /**
     * @return num mines in field
     */
    int minesCount() const;

    /**
     * Minimal number of free positions on a field
     */
    static const int MINIMAL_FREE = 10;

Q_SIGNALS:
    void flaggedMinesCountChanged(int);
    void firstClickDone();
    void gameOver(bool won);
private:
    // reimplemented
    void mousePressEvent( QGraphicsSceneMouseEvent * ) override;
    // reimplemented
    void mouseReleaseEvent( QGraphicsSceneMouseEvent * ) override;
    // reimplemented
    void mouseMoveEvent( QGraphicsSceneMouseEvent * ) override;

    /**
     * Returns cell item at (row,col).
     * Always use this function instead hand-computing index in m_cells
     */
    inline CellItem* itemAt(int row, int col) { return m_cells.at( row*m_numCols + col ); }
    /**
     * Overloaded one, which takes QPair
     */
    inline CellItem* itemAt( FieldPos pos ) { return itemAt(pos.first,pos.second); }
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
     * Returns all adjacent items for item at row, col
     */
    QList<CellItem*> adjacentItemsFor(int row, int col);
    /**
     * Returns all valid adjacent row,col pairs for row, col
     */
    QList<FieldPos> adjacentRowColsFor(int row, int col);
    /**
     * Checks if player lost the game. Return `true` if lost.
     * A `true` return value and a `false` `m_gameOver` indicates that the game is restarted.
     */
    bool checkLost();
    /**
     * Checks if player won the game. Return `true` if won.
     */
    bool checkWon();
    /**
     * Reveals all unmarked items containing mines
     */
    void revealAllMines();
    /**
     * Reimplemented from QGraphicsItem
     */
    void paint( QPainter * painter, const QStyleOptionGraphicsItem*, QWidget * widget = nullptr ) override;
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
    /**
     * Changes the flag state of a clicked cell and updates mine count
     */
    void handleFlag(CellItem* itemUnderMouse);
    /**
     * Return `true` if the game is finished (and possibly restarted) after the call.
     */
    bool onItemRevealed(int row, int col);
    // overload
    bool onItemRevealed(CellItem* item);

    // note: in member functions use itemAt (see above )
    // instead of hand-computing index from row & col!
    // => not depend on how m_cells is represented
    /**
     * Array which holds all child cell items
     */
    QList<CellItem*> m_cells;
    /**
     * Array which holds border items
     */
    QList<BorderItem*> m_borders;
    /**
     * The width and height of minefield cells in scene coordinates
     */
    int m_cellSize = 1; // dummy init value for non-large boundingRect, non-null because used for divisions
    /**
     * Number of field rows
     */
    int m_numRows = 1; // dummy init value for non-large boundingRect, non-null because used for divisions
    /**
     * Number of field columns
     */
    int m_numCols = 1; // dummy init value for non-large boundingRect, non-null because used for divisions
    /**
     * Number of mines in field
     */
    int m_minesCount;
    /**
     * Number of flagged mines
     */
    int m_flaggedMinesCount;
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
