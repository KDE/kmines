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

#ifndef DEFINES_H
#define DEFINES_H

#include <qcolor.h>


class Level
{
 public:
    enum Type { Easy = 0, Normal, Expert, NB_TYPES, Custom = NB_TYPES };
    static const char *LABELS[NB_TYPES+1];
    struct Data {
        uint width, height, nbMines;
        const char *label, *wwLabel;
    };
    static const Data DATA[NB_TYPES];

    Level(Type);
    Level(uint width, uint height, uint nbMines);

    uint width() const   { return _width; }
    uint height() const  { return _height; }
    uint nbMines() const { return _nbMines; }
    Type type() const;
    static uint maxNbMines(uint width, uint height) { return width*height - 2;}

    bool operator ==(const Level &level) const {
        return ( _width==level._width && _height==level._height &&
                 _nbMines==level._nbMines );
    }

 private:
    uint _width, _height, _nbMines;
};

class KMines
{
 public:
    enum GameState   { Playing = 0, Paused, GameOver, Stopped, Replaying,
                       Init, NB_STATES };
	static const char *STATES[NB_STATES];
	enum SolvingState { Regular, Advised, Solved };

    enum CaseState { Covered, Uncovered, Uncertain, Marked, Exploded, Error };
    struct Case {
        bool      mine;
        CaseState state;
    };

    enum NumberColor { NB_N_COLORS = 8 };
    enum Mood { Normal = 0, Stressed, Happy, Sad, Sleeping, NbMoods };
};

#endif
