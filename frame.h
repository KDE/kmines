/*
 * Copyright (c) 2002 Nicolas HADACEK (hadacek@kde.org)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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

    virtual void readSettings();

    enum PixmapType { FlagPixmap = 0, MinePixmap, ExplodedPixmap,
                      ErrorPixmap, Nb_Pixmap_Types,
                      NoPixmap = Nb_Pixmap_Types };
    enum Advised { Nb_Advised = 5 };

    void drawBox(QPainter &, const QPoint &, bool pressed,
                 PixmapType = NoPixmap, const QString &text = QString::null,
                 uint nbMines = 0, int advised = -1,
                 bool hasFocus = false) const;

    uint caseSize() const { return _cp.size; }

 private:
    CaseProperties _cp;
    QPushButton    _button;
    QPixmap        _pixmaps[Nb_Pixmap_Types];
    QPixmap        _advised[Nb_Advised];

    void drawPixmap(QPixmap &, PixmapType, bool mask) const;
    void drawAdvised(QPixmap &, uint i, bool mask) const;
    void initPixmap(QPixmap &, bool mask) const;
};

#endif
