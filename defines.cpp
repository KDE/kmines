/*
 * Copyright (c) 1996-2002 Nicolas HADACEK (hadacek@kde.org)
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

#include "defines.h"

#include <klocale.h>

const char *Level::LABELS[NB_TYPES+1] = {
  I18N_NOOP("Easy"), I18N_NOOP("Normal"), I18N_NOOP("Expert"),
  I18N_NOOP("Custom")
};

const Level::Data Level::DATA[NB_TYPES] = {
    { 8,  8, 10, "easy",   "8x8x10",   },
    {16, 16, 40, "normal", "16x16x40", },
    {30, 16, 99, "expert", "30x16x99", }
};

Level::Level(Type type)
{
    Q_ASSERT( type!=Custom );
    _width   = DATA[type].width;
    _height  = DATA[type].height;
    _nbMines = DATA[type].nbMines;
}

Level::Level(uint width, uint height, uint nbMines)
    : _width(width), _height(height), _nbMines(nbMines)
{
    Q_ASSERT( width>=2 && height>=2 );
    if (_nbMines > maxNbMines(width, height) )
      _nbMines = maxNbMines(width, height);
}

Level::Type Level::type() const
{
    for (uint i=0; i<NB_TYPES; i++)
        if ( _width==DATA[i].width && _height==DATA[i].height
             && _nbMines==DATA[i].nbMines ) return (Type)i;
    return Custom;
}

const char *KMines::STATES[NB_STATES] = {
    "playing", "paused", "gameover", "stopped", "replaying", "init"
};
