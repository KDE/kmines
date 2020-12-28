/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>
    SPDX-FileCopyrightText: 2010 Brian Croom <brian.s.croom@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
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
