/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
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

#include "defines.h"

#include <klocale.h>

const uint Level::MAX_CUSTOM_SIZE = 50;
const uint Level::MIN_CUSTOM_SIZE = 5;

const Level::Data Level::DATA[Level::NbLevels+1] = {
	{ 8,  8, 10, "easy",   "8x8x10",   I18N_NOOP("Easy")   },
	{16, 16, 40, "normal", "16x16x40", I18N_NOOP("Normal") },
	{30, 16, 99, "expert", "30x16x99", I18N_NOOP("Expert") },
    {10, 10, 20, "custom", "",         I18N_NOOP("Custom") }
};

Level::Level(Type type)
{
    Q_ASSERT( type<=NbLevels );
    _width   = DATA[type].width;
    _height  = DATA[type].height;
    _nbMines = DATA[type].nbMines;
}

Level::Level(uint width, uint height, uint nbMines)
    : _width(width), _height(height), _nbMines(nbMines)
{
    if ( type()==Custom ) {
        _width   = QMIN(QMAX(_width, MIN_CUSTOM_SIZE), MAX_CUSTOM_SIZE);
        _height  = QMIN(QMAX(_height, MIN_CUSTOM_SIZE), MAX_CUSTOM_SIZE);
        _nbMines = QMIN(QMAX(_nbMines, 1), maxNbMines());
    } else {
        Q_ASSERT( width>=2 && height>=2 );
        Q_ASSERT( nbMines>0 && nbMines<=maxNbMines() );
    }
}

Level::Type Level::type() const
{
    for (uint i=0; i<NbLevels; i++)
        if ( _width==DATA[i].width && _height==DATA[i].height
             && _nbMines==DATA[i].nbMines ) return (Type)i;
    return Custom;
}

const char *KMines::STATES[NB_STATES] =
    { "playing", "paused", "gameover", "stopped" };
const char *KMines::OP_GROUP = "Options";
