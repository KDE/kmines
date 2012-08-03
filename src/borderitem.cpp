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
    : KGameRenderedItem(renderer, QLatin1String( "" ), parent), m_element(KMinesState::BorderEast),
      m_row(-1), m_col(-1)
{
    if(s_elementNames.isEmpty())
        fillNameHash();
    setShapeMode(BoundingRectShape);
}

void BorderItem::updatePixmap()
{
    setSpriteKey(s_elementNames[m_element]);
}

void BorderItem::fillNameHash()
{
    s_elementNames[KMinesState::BorderNorth] = QLatin1String( "border.edge.north" );
    s_elementNames[KMinesState::BorderSouth] = QLatin1String( "border.edge.south" );
    s_elementNames[KMinesState::BorderEast] = QLatin1String( "border.edge.east" );
    s_elementNames[KMinesState::BorderWest] = QLatin1String( "border.edge.west" );
    s_elementNames[KMinesState::BorderCornerNE] = QLatin1String( "border.outsideCorner.ne" );
    s_elementNames[KMinesState::BorderCornerNW] = QLatin1String( "border.outsideCorner.nw" );
    s_elementNames[KMinesState::BorderCornerSW] = QLatin1String( "border.outsideCorner.sw" );
    s_elementNames[KMinesState::BorderCornerSE] = QLatin1String( "border.outsideCorner.se" );
}
