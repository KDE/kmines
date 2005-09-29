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

#ifndef FRAME_H
#define FRAME_H

#include <qframe.h>
#include <qpixmap.h>
#include <qpushbutton.h>

#include "defines.h"

class QPainter;

class FieldFrame : public QFrame, public KMines
{
 public:
    FieldFrame(QWidget *parent);

 protected:
    enum PixmapType { FlagPixmap = 0, MinePixmap, ExplodedPixmap,
                      ErrorPixmap, Nb_Pixmap_Types,
                      NoPixmap = Nb_Pixmap_Types };
    enum { Nb_Advised = 5 };

    void drawBox(QPainter &, const QPoint &, bool pressed,
                 PixmapType, const QString &text,
                 uint nbMines, int advised, bool hasFocus) const;
    virtual void adjustSize();

 private:
    QPushButton    _button;
    QPixmap        _pixmaps[Nb_Pixmap_Types];
    QPixmap        _advised[Nb_Advised];

    void drawPixmap(QPixmap &, PixmapType, bool mask) const;
    void drawAdvised(QPixmap &, uint i, bool mask) const;
    void initPixmap(QPixmap &, bool mask) const;
};

#endif
