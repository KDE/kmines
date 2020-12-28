/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2010 Brian Croom <brian.s.croom@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef CELLITEM_H
#define CELLITEM_H

#include <KGameRenderedItem>

#include "commondefs.h"

class KGameRenderer;

/**
 * Graphics item representing single cell on
 * the game field.
 * Handles clicks, emits signals when something important happens :)
 */
class CellItem : public KGameRenderedItem
{
public:
    CellItem(KGameRenderer* renderer, QGraphicsItem* parent);
    /**
     * Updates item pixmap according to its current
     * state and properties
     */
    void updatePixmap();
    /**
     * Reimplemented to pass the call on to any child items as well
     */
    void setRenderSize(const QSize &renderSize);
    // FIXME: will it EVER be needed to setHasMine(false)???
    /**
     * Sets whether this item holds mine or not
     */
    void setHasMine(bool hasMine);
    /**
     * @return whether this item holds mine
     */
    bool hasMine() const;
    /**
     * Sets this item so it holds a digit
     *
     * @param digit digit number (1 to 8)
     */
    void setDigit(int digit);
    /**
     * @return digit this item holds or 0 if none
     */
    int digit() const;
    /**
     * Shows what this item hides :)
     * Can be a bomb, a digit, an empty square
     */
    void reveal();
    /**
     * Hides what this item shows ;).
     * I.e. resets revealed state
     */
    void unreveal();
    /**
     * Removes the flag
     */
    void unflag();
    /**
     * Stops the mine from being exploded
     */
    void unexplode();
    /**
     * @return whether this cell is revealed
     */
    bool isRevealed() const;
    /**
     * @return whether this cell is marked with flag
     */
    bool isFlagged() const;
    /**
     * @return whether this cell is marked with question
     */
    bool isQuestioned() const;
    /**
     * @return whether this cell is exploded
     */
    bool isExploded() const;
    /**
     * Resets all properties & state of an item to default ones
     */
    void reset();
    // TODO docs
    void press();
    void release(bool force=false);
    void undoPress();
    void mark();
    // enable use of qgraphicsitem_cast
    enum { Type = UserType + 1 };
    int type() const override;
Q_SIGNALS:
    /**
     * Emitted when this item is revealed with mouse click
     */
    void revealed();
    /**
     * Emitted when flag (not question mark) is set or unset on this item
     * New flagged state can be retrieved via isFlagged()
     */
    void flaggedStateChanged();
private:
    static QHash<int, QString> s_digitNames;
    static QHash<KMinesState::CellState, QList<QString> > s_stateNames;
    static void fillNameHashes();
    /**
     * Current state of this item
     */
    KMinesState::CellState m_state;
    /**
     * True if this item holds mine
     */
    bool m_hasMine;
    /**
     * True if mine is exploded
     */
    bool m_exploded;
    /**
     * Specifies a digit this item holds. 0 if none
     */
    int m_digit;
    /**
     * Add a child object to display an overlayed pixmap
     */
    void addOverlay(const QString& spriteKey);
};

#endif
