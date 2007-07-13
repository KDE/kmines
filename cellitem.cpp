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

CellItem::CellItem(QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent), m_state(KMinesState::Released), m_hasMine(false), m_digit(0)
{
    setShapeMode(BoundingRectShape);
    updatePixmap();
}

void CellItem::updatePixmap()
{
    if( m_state == KMinesState::Revealed && m_digit != 0)
        setPixmap( KMinesRenderer::self()->pixmapForDigitElement(m_digit) );
    else
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
        m_state = KMinesState::Released;
        updatePixmap();
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
    }
}

void CellItem::reveal()
{
    m_state = m_hasMine ? KMinesState::Exploded : KMinesState::Revealed;
    updatePixmap();
}
