/*
 * Copyright (c) 2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "frame.h"

#include <qpainter.h>
#include <qbitmap.h>
#include <qstyle.h>
#include <qdrawutil.h>

#include "settings.h"


FieldFrame::FieldFrame(QWidget *parent)
    : QFrame(parent, "field"), _button(0)
{
    setFrameStyle( QFrame::Box | QFrame::Raised );
	setLineWidth(2);
	setMidLineWidth(2);
}

void FieldFrame::adjustSize()
{
    setFixedSize(sizeHint());
    _button.resize(Settings::caseSize(), Settings::caseSize());

    QBitmap mask;
    for (uint i=0; i<Nb_Pixmap_Types; i++) {
        drawPixmap(mask, (PixmapType)i, true);
        drawPixmap(_pixmaps[i], (PixmapType)i, false);
        _pixmaps[i].setMask(mask);
    }
    for (uint i=0; i<Nb_Advised; i++) {
        drawAdvised(mask, i, true);
        drawAdvised(_advised[i], i, false);
        _advised[i].setMask(mask);
    }

    QFont f = font();
    f.setPointSize(QMAX(1, Settings::caseSize()-6));
    f.setBold(true);
    setFont(f);
}

void FieldFrame::initPixmap(QPixmap &pix, bool mask) const
{
    pix.resize(Settings::caseSize(), Settings::caseSize());
    if (mask) pix.fill(color0);
}

void FieldFrame::drawPixmap(QPixmap &pix, PixmapType type, bool mask) const
{
    initPixmap(pix, mask);
    QPainter p(&pix);

    if ( type==FlagPixmap ) {
        p.setWindow(0, 0, 16, 16);
        p.setPen( (mask ? color1 : black) );
        p.drawLine(6, 13, 14, 13);
        p.drawLine(8, 12, 12, 12);
        p.drawLine(9, 11, 11, 11);
        p.drawLine(10, 2, 10, 10);
        if (!mask) p.setPen(black);
        p.setBrush( (mask ? color1 : Settings::color(Settings::EnumType::flag)) );
        p.drawRect(4, 3, 6, 5);
        return;
    }

    p.setWindow(0, 0, 20, 20);
	if ( type==ExplodedPixmap )
		p.fillRect(2, 2, 16, 16, (mask ? color1 : Settings::color(Settings::EnumType::explosion)));
	QPen pen(mask ? color1 : black, 1);
	p.setPen(pen);
	p.setBrush(mask ? color1 : black);
	p.drawLine(10,3,10,18);
	p.drawLine(3,10,18,10);
	p.drawLine(5, 5, 16, 16);
	p.drawLine(5, 15, 15, 5);
	p.drawEllipse(5, 5, 11, 11);
	p.fillRect(8, 8, 2, 2, (mask ? color1 : white));
	if ( type==ErrorPixmap ) {
		if (!mask) {
			pen.setColor(Settings::color(Settings::EnumType::error));
			p.setPen(pen);
		}
		p.drawLine(3, 3, 17, 17);
		p.drawLine(4, 3, 17, 16);
		p.drawLine(3, 4, 16, 17);
		p.drawLine(3, 17, 17, 3);
		p.drawLine(3, 16, 16, 3);
		p.drawLine(4, 17, 17, 4);
	}
}

void FieldFrame::drawAdvised(QPixmap &pix, uint i, bool mask) const
{
    initPixmap(pix, mask);
    QPainter p(&pix);
    p.setWindow(0, 0, 16, 16);
    p.setPen( QPen(mask ? color1 : Settings::mineColor(i), 2) );
    p.drawRect(3, 3, 11, 11);
}

void FieldFrame::drawBox(QPainter &painter, const QPoint &p,
                      bool pressed, PixmapType type, const QString &text,
                      uint nbMines, int advised,
                      bool hasFocus) const
{
    qDrawShadePanel(&painter, p.x(), p.y(), _button.width(), _button.height(),
                    colorGroup(),  pressed, 2,
                    &colorGroup().brush(QColorGroup::Background));
    if (hasFocus) {
        painter.translate(p.x(), p.y());
        QRect fbr = style().subRect(QStyle::SR_PushButtonFocusRect, &_button);
        style().drawPrimitive(QStyle::PE_FocusRect, &painter, fbr,
                              colorGroup(), QStyle::Style_Enabled);
        painter.resetXForm();
    }

	QRect r(p, _button.size());
    const QPixmap *pixmap = (type==NoPixmap ? 0 : &_pixmaps[type]);
    QColor color = (nbMines==0 ? black : Settings::mineColor(nbMines-1));
    style().drawItem(&painter, r, AlignCenter, colorGroup(), true, pixmap,
                     text, -1, &color);
    if ( advised!=-1 )
        style().drawItem(&painter, r, AlignCenter, colorGroup(), true,
                         &_advised[advised], QString::null);
}
