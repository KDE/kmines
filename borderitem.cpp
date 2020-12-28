/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2010 Brian Croom <brian.s.croom@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
