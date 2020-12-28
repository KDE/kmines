/*
    SPDX-FileCopyrightText: 2007 Dmitry Suzdalev <dimsuz@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef COMMONDEFS_H
#define COMMONDEFS_H

namespace KMinesState
{
    enum CellState { Released, Pressed, Revealed, Questioned, Flagged, Error, Hint };
    enum BorderElement { BorderNorth, BorderSouth, BorderEast, BorderWest,
                         BorderCornerNW, BorderCornerSW, BorderCornerNE, BorderCornerSE };
}

#endif
