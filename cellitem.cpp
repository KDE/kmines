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

#include "renderer.h"

CellItem::CellItem(QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent), m_state(Released), m_hasMine(false)
{
    setShapeMode(BoundingRectShape);
    updatePixmap();
}

void CellItem::updatePixmap()
{
    switch(m_state)
    {
        case Released:
            setPixmap( KMinesRenderer::self()->pixmapForElement( KMinesRenderer::CellUp ) );
            break;
        case Pressed:
            setPixmap( KMinesRenderer::self()->pixmapForElement( KMinesRenderer::CellDown ) );
            break;
        case Questioned:
            setPixmap( KMinesRenderer::self()->pixmapForElement( KMinesRenderer::Question ) );
            break;
        case Flagged:
            setPixmap( KMinesRenderer::self()->pixmapForElement( KMinesRenderer::Flag ) );
            break;
    }
}

void CellItem::mousePressEvent( QGraphicsSceneMouseEvent * ev )
{
    if(ev->button() == Qt::LeftButton
       && m_state != Questioned && m_state != Flagged)
    {
        m_state = Pressed;
        updatePixmap();
    }
}

void CellItem::mouseReleaseEvent( QGraphicsSceneMouseEvent *ev )
{
    if(m_state == Pressed )
    {
        m_state = Released;
        updatePixmap();
    }
    else if(ev->button() == Qt::RightButton )
    {
        // this will provide cycling through
        // Released -> "?"-mark -> "RedFlag"-mark -> Released

        switch(m_state)
        {
            case Released:
                m_state = Questioned;
                break;
            case Questioned:
                m_state = Flagged;
                break;
            case Flagged:
                m_state = Released;
                break;
            default:
                // shouldn't be here
                break;
        } // end switch
        updatePixmap();
    }
}
