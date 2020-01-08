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

#include "borderitem.h"

QHash<KMinesState::BorderElement, QString> BorderItem::s_elementNames;

BorderItem::BorderItem( KGameRenderer* renderer, QGraphicsItem* parent )
    : KGameRenderedItem(renderer, QString(), parent), m_element(KMinesState::BorderEast),
      m_row(-1), m_col(-1)
{
    if(s_elementNames.isEmpty())
        fillNameHash();
    setShapeMode(BoundingRectShape);
}

void BorderItem::setBorderType(KMinesState::BorderElement e)
{
    m_element = e;
    updatePixmap();
}

void BorderItem::setRowCol(int row, int col)
{
    m_row = row;
    m_col = col;
}

int BorderItem::row() const
{
    return m_row;
}

int BorderItem::col() const
{
    return m_col;
}

void BorderItem::updatePixmap()
{
    setSpriteKey(s_elementNames[m_element]);
}

int BorderItem::type() const
{
    return Type;
}

void BorderItem::fillNameHash()
{
    s_elementNames[KMinesState::BorderNorth] = QStringLiteral( "border.edge.north" );
    s_elementNames[KMinesState::BorderSouth] = QStringLiteral( "border.edge.south" );
    s_elementNames[KMinesState::BorderEast] = QStringLiteral( "border.edge.east" );
    s_elementNames[KMinesState::BorderWest] = QStringLiteral( "border.edge.west" );
    s_elementNames[KMinesState::BorderCornerNE] = QStringLiteral( "border.outsideCorner.ne" );
    s_elementNames[KMinesState::BorderCornerNW] = QStringLiteral( "border.outsideCorner.nw" );
    s_elementNames[KMinesState::BorderCornerSW] = QStringLiteral( "border.outsideCorner.sw" );
    s_elementNames[KMinesState::BorderCornerSE] = QStringLiteral( "border.outsideCorner.se" );
}
