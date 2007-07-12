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
#ifndef CELLITEM_H
#define CELLITEM_H

#include <QGraphicsPixmapItem>

#include "commondefs.h"

class QGraphicsSceneMouseEvent;

// TODO docs-docs-docs
class CellItem : public QGraphicsPixmapItem
{
public:
    CellItem(QGraphicsItem* parent);

    void updatePixmap();// according to state

    void setHasMine(bool hasMine) { m_hasMine = hasMine; }
    bool hasMine() const { return m_hasMine; }

    void setDigit(int digit) { m_digit = digit; }
    int digit() const { return m_digit; }

    /**
     * Shows what this item hides :)
     * Can be a bomb, a digit, an empty square
     */
    void reveal();

    // enable use of qgraphicsitem_cast
    enum { Type = UserType + 1 };
    virtual int type() const { return Type; }
private:
    virtual void mousePressEvent( QGraphicsSceneMouseEvent * );
    virtual void mouseReleaseEvent( QGraphicsSceneMouseEvent * );

    KMinesState::CellState m_state;

    bool m_hasMine;
    int m_digit;
};

#endif
