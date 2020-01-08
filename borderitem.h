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
#ifndef BORDERITEM_H
#define BORDERITEM_H
#include <KGameRenderedItem>

#include "commondefs.h"

class KGameRenderer;

/**
 * Graphics item representing border cell
 */
class BorderItem : public KGameRenderedItem
{
public:
    BorderItem( KGameRenderer* renderer, QGraphicsItem* parent );
    void setBorderType( KMinesState::BorderElement e );
    void setRowCol( int row, int col );
    Q_REQUIRED_RESULT int row() const;
    Q_REQUIRED_RESULT int col() const;
    void updatePixmap();

    // enable use of qgraphicsitem_cast
    enum { Type = UserType + 1 };
    Q_REQUIRED_RESULT int type() const override;
private:
    static QHash<KMinesState::BorderElement, QString> s_elementNames;
    static void fillNameHash();

    KMinesState::BorderElement m_element;
    int m_row = -1;
    int m_col = -1;
};

#endif
