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

#include <QPainter>
#include <QBitmap>
#include <QStyleOptionFocusRect>
#include <QPalette>

#include "settings.h"

#include "kstandarddirs.h"


FieldFrame::FieldFrame(QWidget *parent)
    : QFrame(parent), _button(0)
{
    setFrameStyle( QFrame::Box | QFrame::Raised );
    setLineWidth(2);
    setMidLineWidth(2);

    QString themePath = KStandardDirs::locate("appdata", QString("themes/kmines_classic.svgz"));
    if (themePath.isNull()) {
        qDebug () << "theme svg not found!!!";
    };
    svg.load(themePath);
}

void FieldFrame::adjustSize()
{
    setFixedSize(sizeHint());
    _button.resize(Settings::caseSize(), Settings::caseSize());

    QPixmap mask;
    for (uint i=0; i<Nb_Pixmap_Types; i++) {
        drawPixmap(_pixmaps[i], (PixmapType)i, true);
    }
    for (uint i=0; i<Nb_Advised; i++) {
        drawAdvised(_advised[i], i, true);;
    }

    QFont f = font();
    f.setPointSize(qMax(1, Settings::caseSize()-6));
    f.setBold(true);
    setFont(f);
}

void FieldFrame::initPixmap(QPixmap &pix, bool mask) 
{
    pix = QPixmap( Settings::caseSize(), Settings::caseSize() );
    if (mask) pix.fill(Qt::color0);
}

void FieldFrame::drawPixmap(QPixmap &pix, PixmapType type, bool mask) 
{
    initPixmap(pix, mask);

    QImage qiRend(QSize(pix.width(), pix.height()),QImage::Format_ARGB32_Premultiplied);
    qiRend.fill(0);
    QPainter p(&qiRend);

    if ( type==FlagPixmap ) {
        svg.render(&p, "flag");
        pix = QPixmap::fromImage(qiRend);
        return;
    }

    //If exploding...;
    if ( type==ExplodedPixmap ) {
        svg.render(&p, "explosion");;
    }

    //Now render mine graphic
    svg.render(&p, "mine");

    //Finally render the error marker
    if ( type==ErrorPixmap ) {
      svg.render(&p, "error");
    }
    pix = QPixmap::fromImage(qiRend);
}

void FieldFrame::drawAdvised(QPixmap &pix, uint i, bool mask) 
{
    initPixmap(pix, mask);
    QImage qiRend(QSize(pix.width(), pix.height()),QImage::Format_ARGB32_Premultiplied);
    qiRend.fill(0);
    QPainter p(&qiRend);
    /*QPainter p(&pix);
    p.setWindow(0, 0, 16, 16);
    p.setPen( QPen(mask ? Qt::color1 : Settings::mineColor(i), 2) );
    p.drawRect(3, 3, 11, 11);*/
    svg.render(&p, "hint");
    pix = QPixmap::fromImage(qiRend);
}

void FieldFrame::drawBox(QPainter &painter, const QPoint &p,
                      bool pressed, PixmapType type, const QString &text,
                      uint nbMines, int advised,
                      bool hasFocus) 
{
    //Use SVG theme instead
    /*qDrawShadePanel(&painter, p.x(), p.y(), _button.width(), _button.height(),
                    palette(),  pressed, 2,
                    &palette().brush(QPalette::Background));

    if (hasFocus) {
        painter.translate(p.x(), p.y());
        QStyleOptionFocusRect option;
        option.init(this);
        option.fontMetrics = painter.fontMetrics();
        option.state = QStyle::State_Enabled;
        QRect fbr = style()->subElementRect(QStyle::SE_PushButtonFocusRect, &option, &_button);
        option.rect = fbr;
        style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter);
        painter.resetMatrix();
    }*/

    QRect r(p, _button.size());
    if (pressed) {
        svg.render(&painter, "cell_down", r);
    } else {
        svg.render(&painter, "cell_up", r);
    }
    const QPixmap *pixmap = (type==NoPixmap ? 0 : &_pixmaps[type]);
    QColor color = (nbMines==0 ? Qt::black : Settings::mineColor(nbMines-1));
    QPalette pal;
    pal.setColor( QPalette::WindowText, color );
    style()->drawItemText(&painter, r, Qt::AlignCenter, pal, true, text, QPalette::WindowText);
    if (pixmap)
      style()->drawItemPixmap(&painter, r, Qt::AlignCenter, *pixmap);
    if ( advised!=-1 )
        style()->drawItemPixmap(&painter, r, Qt::AlignCenter, _advised[advised]);
}
