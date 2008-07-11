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

#include "settings.h"

#include <kdebug.h>

#include "renderer.h"

CellItem::CellItem(QGraphicsItem* parent)
    : QGraphicsPixmapItem(parent)
{
    setShapeMode(BoundingRectShape);
    reset();
}

void CellItem::reset()
{
    m_state = KMinesState::Released;
    m_hasMine = false;
    m_exploded = false;
    m_digit = 0;
    updatePixmap();
}

void CellItem::updatePixmap()
{
    if(KMinesRenderer::self()->cellSize() == 0)
        return;

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

void CellItem::press()
{
    if(m_state == KMinesState::Released)
    {
        m_state = KMinesState::Pressed;
        updatePixmap();
    }
}

void CellItem::release(bool force)
{
    // special case for mid-button magic
    if(force && (m_state == KMinesState::Flagged || m_state == KMinesState::Questioned))
        return;

    if(m_state == KMinesState::Pressed || force)
    {
        // if we hold mine, let's explode
        m_exploded = m_hasMine;
        reveal();
    }
}

void CellItem::mark()
{
    // this will provide cycling through
    // Released -> "?"-mark -> "RedFlag"-mark -> Released

    bool useQuestion = Settings::self()->useQuestionMarks();

    switch(m_state)
    {
        case KMinesState::Released:
            m_state = KMinesState::Flagged;
            break;
        case KMinesState::Flagged:
            m_state = useQuestion ? KMinesState::Questioned : KMinesState::Released;
            break;
        case KMinesState::Questioned:
            m_state = KMinesState::Released;
            break;
        default:
            // shouldn't be here
            break;
    } // end switch
    updatePixmap();
}

void CellItem::reveal()
{
    if(isRevealed())
        return; // already revealed

    if(m_state == KMinesState::Flagged && m_hasMine == false)
        m_state = KMinesState::Error;
    else
        m_state = KMinesState::Revealed;
    updatePixmap();
}

void CellItem::undoPress()
{
    if(m_state == KMinesState::Pressed)
    {
        m_state = KMinesState::Released;
        updatePixmap();
    }
}
