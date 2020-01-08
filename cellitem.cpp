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

#include "cellitem.h"

#include "settings.h"

QHash<int, QString> CellItem::s_digitNames;
QHash<KMinesState::CellState, QList<QString> > CellItem::s_stateNames;

CellItem::CellItem(KGameRenderer* renderer, QGraphicsItem* parent)
    : KGameRenderedItem(renderer, QString(), parent)
{
    if(s_digitNames.isEmpty())
        fillNameHashes();
    setShapeMode(BoundingRectShape);
    reset();
}

void CellItem::unflag()
{
    m_state = KMinesState::Released;
    updatePixmap();
}

void CellItem::unexplode()
{
    m_exploded = false;
}

bool CellItem::isRevealed() const
{
    return ( m_state == KMinesState::Revealed || m_state == KMinesState::Error);
}

bool CellItem::isFlagged() const
{
    return m_state == KMinesState::Flagged;
}

bool CellItem::isQuestioned() const
{
    return m_state == KMinesState::Questioned;
}

bool CellItem::isExploded() const
{
    return m_exploded;
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
    QList<QGraphicsItem*> children = childItems();
    qDeleteAll(children);

    QList<QString> spriteKeys = s_stateNames[m_state];
    setSpriteKey(spriteKeys[0]);
    for(int i=1; i<spriteKeys.count(); i++)
        addOverlay(spriteKeys[i]);
    if(m_state == KMinesState::Revealed)
    {
        if(m_digit != 0)
            addOverlay(s_digitNames[m_digit]);
        else if(m_hasMine)
        {
            if(m_exploded)
                addOverlay(QStringLiteral( "explosion" ));
            addOverlay(QStringLiteral( "mine" ));
        }
    }
}

void CellItem::setRenderSize(const QSize &renderSize)
{
    KGameRenderedItem::setRenderSize(renderSize);
    QList<QGraphicsItem*> children = childItems();
    foreach( QGraphicsItem* item, children)
    {
        ((KGameRenderedItem*)item)->setRenderSize(renderSize);
    }
}

void CellItem::setHasMine(bool hasMine)
{
    m_hasMine = hasMine;
}

bool CellItem::hasMine() const
{
    return m_hasMine;
}

void CellItem::setDigit(int digit)
{
    m_digit = digit;
    updatePixmap();
}

int CellItem::digit() const
{
    return m_digit;
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

    bool useQuestion = Settings::useQuestionMarks();

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

int CellItem::type() const
{
    return Type;
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

void CellItem::unreveal()
{
    m_state = KMinesState::Released;
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

void CellItem::fillNameHashes()
{
    s_digitNames[1] = QStringLiteral( "arabicOne" );
    s_digitNames[2] = QStringLiteral( "arabicTwo" );
    s_digitNames[3] = QStringLiteral( "arabicThree" );
    s_digitNames[4] = QStringLiteral( "arabicFour" );
    s_digitNames[5] = QStringLiteral( "arabicFive" );
    s_digitNames[6] = QStringLiteral( "arabicSix" );
    s_digitNames[7] = QStringLiteral( "arabicSeven" );
    s_digitNames[8] = QStringLiteral( "arabicEight" );

    s_stateNames[KMinesState::Released].append(QStringLiteral( "cell_up" ));
    s_stateNames[KMinesState::Pressed].append(QStringLiteral( "cell_down" ));
    s_stateNames[KMinesState::Revealed].append(QStringLiteral( "cell_down" ));
    s_stateNames[KMinesState::Questioned].append(QStringLiteral( "cell_up" ));
    s_stateNames[KMinesState::Questioned].append(QStringLiteral( "question" ));
    s_stateNames[KMinesState::Flagged].append(QStringLiteral( "cell_up" ));
    s_stateNames[KMinesState::Flagged].append(QStringLiteral( "flag" ));
    s_stateNames[KMinesState::Error].append(QStringLiteral( "cell_down" ));
    s_stateNames[KMinesState::Error].append(QStringLiteral( "mine" ));
    s_stateNames[KMinesState::Error].append(QStringLiteral( "error" ));
    s_stateNames[KMinesState::Hint].append(QStringLiteral( "cell_up" ));
    s_stateNames[KMinesState::Hint].append(QStringLiteral( "hint" ));
}

void CellItem::addOverlay(const QString& spriteKey)
{
    KGameRenderedItem* overlay = new KGameRenderedItem(renderer(), spriteKey, this);
    overlay->setRenderSize(renderSize());
}
