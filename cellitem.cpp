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

#include "cellitem.h"
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include <kdebug.h>

#include "renderer.h"

// HIDDEN NOTE: current design is that every CellItem is a QObject
// and analyzes all mouse clicks by itself and emits corresponding
// signals when needed. The thoughts are coming to my head that
// this might be a bit heavy for every item be a QObject
// (although I doubt this, but I decided to write this note anyway)
// Alternative approach would be to handle mouse events in MineFieldItem
// and forward them to corresponding items, but that wouldn't be so nice
// looking IMO :). From design side.
// I don't know if this "heaviness" applies to this case - I doubt we'll
// have huge-huge boards. Anyway I'll leave this note. If someone finds
// it's a nonsense or that it doesn't apply or it's stupid, please remove
// it immediately :-)

CellItem::CellItem(QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
{
    reset();
    setShapeMode(BoundingRectShape);
    updatePixmap();
}

void CellItem::reset()
{
    m_state = KMinesState::Released;
    m_hasMine = false;
    m_exploded = false;
    m_digit = 0;
}

void CellItem::updatePixmap()
{
    // several special cases at the beginning

    if( m_state == KMinesState::Revealed && m_digit != 0)
    {
        setPixmap( KMinesRenderer::self()->pixmapForDigitElement(m_digit) );
        return;
    }

    if( m_state == KMinesState::Revealed && m_hasMine )
    {
        if( m_exploded )
            setPixmap( KMinesRenderer::self()->pixmapExplodedMine() );
        else
            setPixmap( KMinesRenderer::self()->pixmapMine() );
        return;
    }

    setPixmap( KMinesRenderer::self()->pixmapForCellState( m_state ) );
}

void CellItem::mousePressEvent( QGraphicsSceneMouseEvent * ev )
{
    if(ev->button() == Qt::LeftButton && m_state == KMinesState::Released)
    {
        m_state = KMinesState::Pressed;
        updatePixmap();
    }
}

void CellItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *ev )
{
    if(m_state == KMinesState::Pressed )
    {
        // if we hold mine, let's explode on mouse click
        m_exploded = m_hasMine;
        reveal();
        emit revealed();
    }
    else if(ev->button() == Qt::RightButton )
    {
        // this will provide cycling through
        // Released -> "?"-mark -> "RedFlag"-mark -> Released

        switch(m_state)
        {
            case KMinesState::Released:
                m_state = KMinesState::Questioned;
                break;
            case KMinesState::Questioned:
                m_state = KMinesState::Flagged;
                break;
            case KMinesState::Flagged:
                m_state = KMinesState::Released;
                break;
            default:
                // shouldn't be here
                break;
        } // end switch
        updatePixmap();
        emit flaggedStateChanged( m_state == KMinesState::Flagged );
    }
}

void CellItem::reveal()
{
    m_state = KMinesState::Revealed;
    updatePixmap();
}
