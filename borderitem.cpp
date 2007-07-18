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

#include "borderitem.h"
#include "renderer.h"

BorderItem::BorderItem( QGraphicsItem* parent )
    : QGraphicsPixmapItem(parent), m_element(KMinesState::BorderEast),
      m_row(-1), m_col(-1)
{
    setShapeMode(BoundingRectShape);
}

void BorderItem::updatePixmap()
{
    setPixmap(KMinesRenderer::self()->pixmapForBorderElement(m_element));
}
